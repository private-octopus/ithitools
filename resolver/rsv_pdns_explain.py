# Explain what drives duplication and ISP repetition ratios.
#
# Runs on two datasets (separately, then compared):
#   1. Stats CSV only  (full 4k+ CC/AS sample, no UA features)
#   2. Overlap with UA JSON (841 CC/AS, adds device-class shares and currency)
#
# For each target variable (duplication, rep_short_ISP, rep_long_ISP):
#   - Pearson correlations with all predictors
#   - Random-forest regression importance (permutation-based)
#   - Partial dependence plots for top-5 predictors
#
# Usage:
#   python rsv_pdns_explain.py <stats.csv> [<ua_clusters.json>] [--output-dir DIR]

import sys
import os
import json
import argparse
import warnings
import numpy as np
import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from sklearn.ensemble import RandomForestRegressor
from sklearn.inspection import permutation_importance, PartialDependenceDisplay
from sklearn.model_selection import cross_val_score
from sklearn.preprocessing import StandardScaler

warnings.filterwarnings("ignore", category=UserWarning)

TARGETS = ["duplication", "rep_short_ISP", "rep_long_ISP"]

TRACKED_PDNS = ["googlepdns", "cloudflare", "quad9", "opendns", "level3",
                "neustar", "he", "same_CC", "others"]

DEVICE_CLASSES = ["Windows", "Mac", "iPad", "iPhone",
                  "Android_Phone", "Android_Tablet",
                  "Other_PC", "Other", "Bot"]

# ── feature builders ──────────────────────────────────────────────────────────

def stats_features(df):
    dup = df["duplication"].clip(lower=1e-9)
    feat = pd.DataFrame(index=df.index)
    feat["frac_AAAA"]  = df["frac_AAAA"].clip(0, 1)
    feat["frac_HTTPS"] = df["frac_HTTPS"].clip(0, 1)
    feat["share_ISP4"] = df["frac_ISP4"] / dup
    feat["share_ISP6"] = df["frac_ISP6"] / dup
    feat["share_ISP"]  = feat["share_ISP4"] + feat["share_ISP6"]
    for p in TRACKED_PDNS:
        col = "frac_" + p
        if col in df.columns:
            feat["share_" + p] = df[col] / dup
    return feat.fillna(0.0)

def load_ua_df(json_path):
    with open(json_path, "r") as f:
        data = json.load(f)
    cluster_shares = {cl["cluster"]: cl.get("mean_device_share", {})
                      for cl in data.get("clusters", [])}
    rows = []
    for entry in data.get("cc_as", []):
        shares = cluster_shares.get(entry.get("cluster"), {})
        row = {"cc": entry["cc"], "as": entry["AS"],
               "currency": entry.get("currency", 0.0)}
        for d in DEVICE_CLASSES:
            row["ua_" + d] = shares.get(d.replace("_", " "), 0.0)
        rows.append(row)
    ua_df = pd.DataFrame(rows)
    share_cols = ["ua_" + d for d in DEVICE_CLASSES]
    total = ua_df[share_cols].sum(axis=1).clip(lower=1e-9)
    ua_df[share_cols] = ua_df[share_cols].div(total, axis=0)
    return ua_df

# ── analysis ──────────────────────────────────────────────────────────────────

def pearson_table(X, y, feature_names, target_name):
    corrs = []
    for i, name in enumerate(feature_names):
        c = np.corrcoef(X[:, i], y)[0, 1]
        corrs.append((name, c))
    corrs.sort(key=lambda x: -abs(x[1]))
    return corrs

def run_rf(X, y, feature_names, target_name, out_dir, label):
    rf = RandomForestRegressor(n_estimators=300, max_features="sqrt",
                               n_jobs=-1, random_state=42)
    rf.fit(X, y)
    cv_r2 = cross_val_score(rf, X, y, cv=5, scoring="r2", n_jobs=-1).mean()

    perm = permutation_importance(rf, X, y, n_repeats=20,
                                  random_state=42, n_jobs=-1)
    importance = sorted(zip(feature_names, perm.importances_mean,
                            perm.importances_std),
                        key=lambda x: -x[1])

    print(f"\n  CV R² = {cv_r2:.3f}")
    print(f"  {'Feature':<28} {'Importance':>10}  {'±':>6}")
    for name, imp, std in importance[:15]:
        bar = "█" * max(0, int(imp * 200))
        print(f"  {name:<28} {imp:>10.4f}  {std:>6.4f}  {bar}")

    # Partial dependence for top 5 features by permutation importance
    top5_idx = [feature_names.index(name) for name, _, _ in importance[:5]]
    fig, axes = plt.subplots(1, 5, figsize=(18, 4))
    fig.suptitle(f"{label} — partial dependence for {target_name}", fontsize=11)
    PartialDependenceDisplay.from_estimator(
        rf, X, features=top5_idx, feature_names=feature_names,
        ax=axes, line_kw={"color": "steelblue"})
    plt.tight_layout()
    safe_target = target_name.replace("_", "-")
    safe_label  = label.lower().replace(" ", "_")
    plot_path = os.path.join(out_dir, f"pdp_{safe_label}_{safe_target}.png")
    plt.savefig(plot_path, dpi=150)
    plt.close()
    print(f"  PDP saved → {plot_path}")

    return importance, cv_r2

def analyse(X, y_df, feature_names, label, out_dir):
    print(f"\n{'='*60}")
    print(f"Dataset: {label}  ({len(X)} CC/AS pairs, {len(feature_names)} features)")
    print(f"{'='*60}")

    results = {}
    for target in TARGETS:
        y = y_df[target].values
        print(f"\n── Target: {target} ──")
        print(f"  mean={y.mean():.4f}  median={np.median(y):.4f}  "
              f"p90={np.percentile(y,90):.4f}  max={y.max():.4f}")

        corrs = pearson_table(X, y, feature_names, target)
        print(f"\n  Top Pearson correlations:")
        for name, c in corrs[:10]:
            bar = "▓" * int(abs(c) * 20)
            sign = "+" if c >= 0 else "-"
            print(f"    {name:<28} {sign}{abs(c):.4f}  {bar}")

        print(f"\n  Random-forest permutation importance:")
        importance, cv_r2 = run_rf(X, y_df[target].values, feature_names,
                                   target, out_dir, label)
        results[target] = {"cv_r2": cv_r2, "importance": importance}

    return results

def compare_r2(r2_stats, r2_combined):
    print(f"\n{'='*60}")
    print("R² comparison: stats-only vs combined (stats + UA)")
    print(f"{'='*60}")
    print(f"  {'Target':<20} {'Stats-only':>12}  {'Combined':>10}  {'Δ':>8}")
    for t in TARGETS:
        s = r2_stats[t]["cv_r2"]
        c = r2_combined[t]["cv_r2"]
        print(f"  {t:<20} {s:>12.3f}  {c:>10.3f}  {c-s:>+8.3f}")

# ── main ──────────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("stats_csv")
    parser.add_argument("ua_json", nargs="?", default=None)
    parser.add_argument("--output-dir", default=".")
    args = parser.parse_args()

    os.makedirs(args.output_dir, exist_ok=True)

    stats_df = pd.read_csv(args.stats_csv)
    stats_df["as"] = stats_df["as"].astype(str)

    # ── Dataset 1: stats features only ──
    sf = stats_features(stats_df)
    feature_names_stats = list(sf.columns)
    X_stats = sf.values
    r2_stats = analyse(X_stats, stats_df[TARGETS], feature_names_stats,
                       "Stats only", args.output_dir)

    # ── Dataset 2: stats + UA features (overlap) ──
    if args.ua_json:
        ua_df = load_ua_df(args.ua_json)
        ua_df["as"] = ua_df["as"].astype(str)
        merged = stats_df.merge(ua_df, on=["cc", "as"], how="inner")
        print(f"\nOverlap with UA data: {len(merged)} CC/AS pairs.")

        sf2 = stats_features(merged)
        ua_cols = ["ua_" + d for d in DEVICE_CLASSES] + ["currency"]
        uf = merged[ua_cols].fillna(0.0)
        feat_combined = pd.concat([sf2, uf], axis=1)
        feature_names_comb = list(feat_combined.columns)
        X_comb = feat_combined.values
        r2_combined = analyse(X_comb, merged[TARGETS], feature_names_comb,
                              "Stats + UA", args.output_dir)

        compare_r2(r2_stats, r2_combined)

if __name__ == "__main__":
    main()
