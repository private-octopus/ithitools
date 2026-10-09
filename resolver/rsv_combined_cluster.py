# Cluster CC/AS entries using both query-statistics features (from rsv_pdns_stats.py)
# and UA-string features (from rsv_cc_as_cluster.py output JSON).
# Only CC/AS pairs present in both sources are clustered.
#
# Usage:
#   python rsv_combined_cluster.py <stats.csv> <ua_clusters.json> <output.csv>
#                                  [--clusters N] [--sweep] [--plot FILE]

import sys
import os
import json
import argparse
import pandas as pd
import numpy as np
from sklearn.preprocessing import StandardScaler
from sklearn.cluster import KMeans
from sklearn.metrics import silhouette_score
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

# ── feature definitions ──────────────────────────────────────────────────────

TRACKED_PDNS = ["googlepdns", "cloudflare", "quad9", "opendns", "level3",
                "neustar", "he", "same_CC", "others"]

DEVICE_CLASSES = ["Windows", "Mac", "iPad", "iPhone",
                  "Android Phone", "Android Tablet",
                  "Other PC", "Other", "Bot"]

def stats_features(df):
    """Build the query-statistics feature block from the stats CSV."""
    dup = df["duplication"].clip(lower=1e-9)
    feat = pd.DataFrame(index=df.index)
    feat["frac_AAAA"]  = df["frac_AAAA"].clip(0, 1)
    feat["frac_HTTPS"] = df["frac_HTTPS"].clip(0, 1)
    feat["share_ISP"]  = (df["frac_ISP4"] + df["frac_ISP6"]) / dup
    for p in TRACKED_PDNS:
        col = "frac_" + p
        if col in df.columns:
            feat["share_" + p] = df[col] / dup
    feat["duplication"]   = dup
    feat["rep_short_ISP"] = df["rep_short_ISP"].clip(lower=0)
    feat["rep_long_ISP"]  = df["rep_long_ISP"].clip(lower=0)
    return feat.fillna(0.0)

def load_ua_features(json_path):
    """Load the UA-cluster JSON and return a DataFrame keyed by (cc, as).

    Device-class shares live in clusters[].mean_device_share (one entry per
    cluster); individual cc_as[] entries reference their cluster by id.  We
    join the two to give each CC/AS its cluster's device-share profile plus its
    own currency value.
    """
    with open(json_path, "r") as f:
        data = json.load(f)

    # Build cluster-id → device shares lookup
    cluster_shares = {}
    for cl in data.get("clusters", []):
        cid = cl["cluster"]
        cluster_shares[cid] = cl.get("mean_device_share", {})

    rows = []
    for entry in data.get("cc_as", []):
        cid = entry.get("cluster")
        shares = cluster_shares.get(cid, {})
        row = {
            "cc":       entry["cc"],
            "as":       entry["AS"],    # JSON uses "AS", normalise to "as"
            "n":        entry.get("n", 0),
            "currency": entry.get("currency", 0.0),
        }
        for d in DEVICE_CLASSES:
            row["ua_" + d.replace(" ", "_")] = shares.get(d, 0.0)
        rows.append(row)

    ua_df = pd.DataFrame(rows)
    # Normalise device shares so they sum to 1 (guard against rounding drift)
    share_cols = ["ua_" + d.replace(" ", "_") for d in DEVICE_CLASSES]
    total = ua_df[share_cols].sum(axis=1).clip(lower=1e-9)
    ua_df[share_cols] = ua_df[share_cols].div(total, axis=0)
    return ua_df

# ── clustering helpers ────────────────────────────────────────────────────────

def sweep_clusters(X_scaled, k_min=2, k_max=12):
    print(f"{'k':>4}  {'inertia':>14}  {'silhouette':>12}")
    print("-" * 36)
    best_k, best_sil = k_min, -1.0
    for k in range(k_min, k_max + 1):
        km = KMeans(n_clusters=k, n_init=20, random_state=42)
        labels = km.fit_predict(X_scaled)
        inertia = km.inertia_
        sil = silhouette_score(X_scaled, labels,
                               sample_size=min(5000, len(X_scaled)))
        marker = " <" if sil > best_sil else ""
        print(f"{k:>4}  {inertia:>14.1f}  {sil:>12.4f}{marker}")
        if sil > best_sil:
            best_sil, best_k = sil, k
    print(f"\nBest k by silhouette: {best_k} (score={best_sil:.4f})")
    return best_k

def cluster_label(centroid, feature_names):
    """Human-readable label from a feature centroid."""
    # Identify dominant device class
    ua_cols = {n: centroid[i] for i, n in enumerate(feature_names)
               if n.startswith("ua_")}
    if ua_cols:
        top_dev  = max(ua_cols, key=ua_cols.get).replace("ua_", "").replace("_", " ")
        top_val  = max(ua_cols.values())
        if top_val >= 0.6:
            dev_label = f"{top_dev}-dominant"
        else:
            sorted_dev = sorted(ua_cols.items(), key=lambda x: -x[1])
            top2 = sorted_dev[:2]
            if sum(v for _, v in top2) >= 0.6:
                dev_label = " / ".join(k.replace("ua_", "").replace("_", " ")
                                       for k, _ in top2) + " mixed"
            else:
                dev_label = "Diverse devices"
    else:
        dev_label = "Unknown devices"

    # Dominant provider character
    isp_share = centroid[feature_names.index("share_ISP")] \
        if "share_ISP" in feature_names else 0
    dup = centroid[feature_names.index("duplication")] \
        if "duplication" in feature_names else 1
    prov_label = "ISP-heavy" if isp_share > 0.7 else \
                 "PDNS-heavy" if isp_share < 0.3 else "Mixed routing"

    return f"{dev_label}, {prov_label}"

def describe_clusters(merged, feature_names, labels, centroids_scaled, scaler):
    merged = merged.copy()
    merged["cluster"] = labels
    centroids_orig = scaler.inverse_transform(centroids_scaled)

    print("\n=== Cluster summary ===")
    for c in sorted(merged["cluster"].unique()):
        mask = merged["cluster"] == c
        n = mask.sum()
        centroid = centroids_orig[c]
        label = cluster_label(centroid.tolist(), feature_names)
        print(f"\nCluster {c} — {label}  ({n} CC/AS pairs)")
        top_cc = merged.loc[mask, "cc"].value_counts().head(5)
        print(f"  Top CCs: {', '.join(f'{cc}({cnt})' for cc, cnt in top_cc.items())}")
        if "n_ua" in merged.columns:
            print(f"  total_n (UA): {merged.loc[mask, 'n_ua'].sum():,}")
        for i, feat in enumerate(feature_names):
            print(f"  {feat:<30} {centroid[i]:>8.4f}")

def plot_clusters(X_scaled, labels, out_path):
    from sklearn.decomposition import PCA
    pca = PCA(n_components=2, random_state=42)
    coords = pca.fit_transform(X_scaled)
    fig, ax = plt.subplots(figsize=(9, 6))
    sc = ax.scatter(coords[:, 0], coords[:, 1],
                    c=labels, cmap="tab10", s=10, alpha=0.6)
    ax.set_xlabel(f"PC1 ({pca.explained_variance_ratio_[0]*100:.1f}%)")
    ax.set_ylabel(f"PC2 ({pca.explained_variance_ratio_[1]*100:.1f}%)")
    ax.set_title("Combined UA + query-stats clusters (PCA)")
    plt.colorbar(sc, ax=ax, label="cluster")
    plt.tight_layout()
    plt.savefig(out_path, dpi=150)
    print(f"PCA plot saved to {out_path}")

# ── main ──────────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("stats_csv",    help="output of rsv_pdns_stats.py")
    parser.add_argument("ua_json",      help="output of rsv_cc_as_cluster.py")
    parser.add_argument("output_csv",   help="clustered output")
    parser.add_argument("--clusters", type=int, default=0,
                        help="number of clusters (0 = auto via silhouette sweep)")
    parser.add_argument("--sweep", action="store_true",
                        help="print silhouette scores for k=2..12 and exit")
    parser.add_argument("--plot", metavar="FILE",
                        help="save PCA scatter plot to FILE")
    args = parser.parse_args()

    # Load stats CSV
    stats_df = pd.read_csv(args.stats_csv)
    stats_df["as"] = stats_df["as"].astype(str)
    print(f"Stats CSV: {len(stats_df)} CC/AS pairs.")

    # Load UA JSON
    ua_df = load_ua_features(args.ua_json)
    ua_df["as"] = ua_df["as"].astype(str)
    print(f"UA JSON:   {len(ua_df)} CC/AS pairs.")

    # Inner join — only pairs present in both sources
    merged = stats_df.merge(ua_df, on=["cc", "as"], how="inner", suffixes=("", "_ua"))
    # 'n' from ua_df carries traffic volume; rename for clarity
    if "n" in merged.columns:
        merged = merged.rename(columns={"n": "n_ua"})
    print(f"Overlap:   {len(merged)} CC/AS pairs will be clustered.")

    if len(merged) < 10:
        print("Too few overlapping pairs to cluster.")
        sys.exit(1)

    # Build combined feature matrix
    sf = stats_features(merged)
    ua_cols = ["ua_" + d.replace(" ", "_") for d in DEVICE_CLASSES] + ["currency"]
    uf = merged[ua_cols].fillna(0.0)
    features = pd.concat([sf, uf], axis=1)
    feature_names = list(features.columns)

    scaler = StandardScaler()
    X_scaled = scaler.fit_transform(features.values)

    if args.sweep:
        sweep_clusters(X_scaled)
        return

    if args.clusters == 0:
        print("Running silhouette sweep to select k...")
        k = sweep_clusters(X_scaled)
    else:
        k = args.clusters

    print(f"\nFitting k-means with k={k}...")
    km = KMeans(n_clusters=k, n_init=20, random_state=42)
    labels = km.fit_predict(X_scaled)

    describe_clusters(merged, feature_names, labels, km.cluster_centers_, scaler)

    # Write output CSV
    out = merged[["cc", "as", "total_uids"]].copy()
    for extra in ("n_ua", "n"):
        if extra in merged.columns:
            out["n_ua"] = merged[extra]
            break
    out["cluster"] = labels
    for col in feature_names:
        out[col] = features[col].values
    out.to_csv(args.output_csv, index=False)
    print(f"\nWrote {len(out)} rows to {args.output_csv}")

    if args.plot:
        plot_clusters(X_scaled, labels, args.plot)

if __name__ == "__main__":
    main()
