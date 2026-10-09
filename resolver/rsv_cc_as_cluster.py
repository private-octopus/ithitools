# Automatic classification of CC+AS into a small number of client-profile
# categories, based on the per-CC+AS device/OS/currency summary produced by
# rsv_cc_as_ua.py.
#
# Input: the JSON (or bz2-compressed JSON) file produced by rsv_cc_as_ua.py,
# i.e. a { "global": {...}, "cc_as": [ {"cc", "AS", "n", "device_class",
# "os_family", "browser_family", "os_update"}, ... ] } object.
#
# We only cluster CC+AS with enough monthly traffic to give stable statistics
# (default: at least 10,000 entries, matching the threshold used elsewhere
# for AS-level reporting -- see extract_AS_from_summary() in rsv_as_focus.py).
#
# For each retained CC+AS we build a feature vector:
#   - the share of its traffic in each device_class (Windows, Mac, iPad,
#     iPhone, Android Phone, Android Tablet, Other PC, Other, Bot)
#   - a single "currency" score: the traffic-weighted average, across the
#     OS families this CC+AS actually uses, of pct_on_latest_or_previous
#     (how much of its traffic is running the latest or second-latest major
#     OS version observed globally -- see rsv_cc_as_ua.py's os_update).
#
# Note on reading "currency": Windows and Mac OS X currently show as ~97-100%
# "on latest" almost everywhere, because most modern browsers freeze the
# Windows/Mac OS version token in the UA string for privacy reasons (nearly
# all Windows report major "10", nearly all Mac report major "10"). That
# makes currency close to constant -- and uninformative -- for desktop-heavy
# CC+AS. It varies meaningfully for Android and iOS, where the OS major
# version is not frozen, so it mostly differentiates mobile-heavy clusters.
#
# We standardize the feature vectors and run KMeans for a range of cluster
# counts, picking the count with the best silhouette score, then label each
# resulting cluster from its centroid's dominant device class(es).
#
# Usage:
#   python rsv_cc_as_cluster.py <output.json> <cc_as_ua-file> [--min-n N] [--k-min N] [--k-max N]

import argparse
import bz2
import json
import time

import numpy as np
import pandas as pd
from sklearn.cluster import KMeans
from sklearn.metrics import silhouette_score
from sklearn.preprocessing import StandardScaler

DEVICE_CLASSES = [
    "Windows", "Mac", "iPad", "iPhone",
    "Android Phone", "Android Tablet", "Other PC", "Other", "Bot",
]


def load_json_file(path):
    try:
        with bz2.open(path, "rt", encoding="utf-8") as F:
            return json.load(F)
    except OSError:
        with open(path, "rt", encoding="utf-8") as F:
            return json.load(F)


def currency_score(record):
    """Traffic-weighted average of pct_on_latest_or_previous across the OS
    families used by this CC+AS, weighted by each family's share of its
    (version-classified) traffic."""
    os_update = record.get("os_update", {})
    total_weight = 0
    weighted_sum = 0.0
    for os_family, info in os_update.items():
        weight = sum(info.get("versions", {}).values())
        if weight <= 0:
            continue
        weighted_sum += weight * info.get("pct_on_latest_or_previous", 0.0)
        total_weight += weight
    if total_weight == 0:
        return np.nan
    return weighted_sum / total_weight


def build_feature_frame(cc_as_records, min_n):
    rows = []
    for r in cc_as_records:
        if r["n"] < min_n:
            continue
        row = {"cc": r["cc"], "AS": r["AS"], "n": r["n"]}
        device_class = r.get("device_class", {})
        for dc in DEVICE_CLASSES:
            row[dc] = device_class.get(dc, 0) / r["n"]
        row["currency"] = currency_score(r)
        rows.append(row)
    df = pd.DataFrame(rows)
    if df["currency"].isna().any():
        fallback = df["currency"].median()
        df["currency"] = df["currency"].fillna(fallback)
    return df


def choose_k(X, k_min, k_max, random_state=42):
    scores = []
    for k in range(k_min, k_max + 1):
        if k >= len(X):
            break
        km = KMeans(n_clusters=k, n_init=10, random_state=random_state)
        labels = km.fit_predict(X)
        score = silhouette_score(X, labels)
        scores.append((k, score, km.inertia_))
        print("  k=%d  silhouette=%.4f  inertia=%.1f" % (k, score, km.inertia_))
    best_k = max(scores, key=lambda t: t[1])[0]
    return best_k, scores


def label_cluster(mean_shares):
    ranked = sorted(mean_shares.items(), key=lambda kv: -kv[1])
    top1, top1_v = ranked[0]
    top2, top2_v = ranked[1]
    if top1_v >= 0.6:
        return "%s-dominant" % top1
    if top1_v + top2_v >= 0.6:
        return "%s / %s mixed" % (top1, top2)
    return "Diverse mix"


def build_report(source_path, df, feature_cols, k_min, k_max):
    X = StandardScaler().fit_transform(df[feature_cols].values)

    print("Selecting cluster count in [%d, %d] by silhouette score:" % (k_min, k_max))
    best_k, scores = choose_k(X, k_min, k_max)
    print("Chosen k = %d" % best_k)

    km = KMeans(n_clusters=best_k, n_init=10, random_state=42)
    df = df.copy()
    df["cluster"] = km.fit_predict(X)

    clusters = []
    for cluster_id, group in df.groupby("cluster"):
        mean_shares = {dc: float(group[dc].mean()) for dc in DEVICE_CLASSES}
        label = label_cluster(mean_shares)
        clusters.append({
            "cluster": int(cluster_id),
            "label": label,
            "num_cc_as": int(len(group)),
            "total_n": int(group["n"].sum()),
            "mean_device_share": {k: round(v, 4) for k, v in mean_shares.items()},
            "mean_currency": round(float(group["currency"].mean()), 4),
        })
    clusters.sort(key=lambda c: c["total_n"], reverse=True)
    label_by_cluster = {c["cluster"]: c["label"] for c in clusters}

    members = []
    for _, row in df.sort_values("n", ascending=False).iterrows():
        members.append({
            "cc": row["cc"],
            "AS": row["AS"],
            "n": int(row["n"]),
            "cluster": int(row["cluster"]),
            "label": label_by_cluster[row["cluster"]],
            "currency": round(float(row["currency"]), 4),
        })

    report = {
        "generated": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "source": source_path,
        "min_n": int(df["n"].min()) if len(df) else None,
        "num_cc_as_clustered": len(df),
        "k_search_range": [k_min, k_max],
        "k_chosen": best_k,
        "silhouette_by_k": [{"k": k, "silhouette": round(s, 4), "inertia": round(i, 1)} for k, s, i in scores],
        "clusters": clusters,
        "cc_as": members,
    }
    return report


def write_report(report, output_path):
    text = json.dumps(report, indent=2)
    if output_path.endswith(".bz2"):
        with bz2.open(output_path, "wt", encoding="utf-8") as F:
            F.write(text)
    else:
        with open(output_path, "wt", encoding="utf-8") as F:
            F.write(text)


def parse_args():
    p = argparse.ArgumentParser(description="Cluster CC+AS by client device/OS/currency profile.")
    p.add_argument("output", help="path of the JSON report to create")
    p.add_argument("input", help="cc_as_ua summary file produced by rsv_cc_as_ua.py")
    p.add_argument("--min-n", type=int, default=10000,
                    help="minimum monthly entries for a CC+AS to be clustered (default: 10000)")
    p.add_argument("--k-min", type=int, default=3, help="minimum number of clusters to try (default: 3)")
    p.add_argument("--k-max", type=int, default=10, help="maximum number of clusters to try (default: 10)")
    return p.parse_args()


# main

if __name__ == "__main__":
    args = parse_args()
    time_start = time.time()

    print("Loading " + args.input)
    obj = load_json_file(args.input)

    df = build_feature_frame(obj["cc_as"], args.min_n)
    print("%d CC+AS have at least %d entries and will be clustered." % (len(df), args.min_n))

    feature_cols = DEVICE_CLASSES + ["currency"]
    report = build_report(args.input, df, feature_cols, args.k_min, args.k_max)

    write_report(report, args.output)
    print("Wrote " + args.output + " in %.1f seconds." % (time.time() - time_start))
