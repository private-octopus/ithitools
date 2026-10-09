# Cluster CC/AS entries from the statistics CSV produced by rsv_pdns_stats.py.
#
# Features used (all normalized to comparable scales):
#   frac_AAAA, frac_HTTPS       - query-type adoption ratios
#   share_ISP                   - (frac_ISP4+frac_ISP6) / duplication
#   share_googlepdns, _cloudflare, _quad9, _opendns, _same_CC, _others
#                               - PDNS provider shares within total provider mix
#   duplication                 - mean number of providers per unique query
#   rep_short_ISP, rep_long_ISP - ISP cross-protocol repetition ratios
#
# Usage:
#   python rsv_pdns_cluster.py <stats.csv> <output.csv> [--clusters N] [--sweep]
#
#   --clusters N  use N clusters (default: chosen from silhouette sweep)
#   --sweep       print silhouette scores for k=2..12 and exit (helps pick k)

import sys
import os
import argparse
import pandas as pd
import numpy as np
from sklearn.preprocessing import StandardScaler
from sklearn.cluster import KMeans
from sklearn.metrics import silhouette_score
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

# Providers to include as individual "share" features; others are grouped.
TRACKED_PDNS = ["googlepdns", "cloudflare", "quad9", "opendns", "level3",
                "neustar", "he", "same_CC", "others"]

def build_features(df):
    dup = df["duplication"].clip(lower=1e-9)

    features = pd.DataFrame(index=df.index)
    features["frac_AAAA"]  = df["frac_AAAA"].clip(0, 1)
    features["frac_HTTPS"] = df["frac_HTTPS"].clip(0, 1)

    # ISP share of the total provider mix
    features["share_ISP"] = (df["frac_ISP4"] + df["frac_ISP6"]) / dup

    # Per-PDNS shares within total provider mix
    for p in TRACKED_PDNS:
        col = "frac_" + p
        if col in df.columns:
            features["share_" + p] = df[col] / dup

    features["duplication"]   = dup
    features["rep_short_ISP"] = df["rep_short_ISP"].clip(lower=0)
    features["rep_long_ISP"]  = df["rep_long_ISP"].clip(lower=0)

    return features.fillna(0.0)

def sweep_clusters(X_scaled, k_min=2, k_max=12):
    print(f"{'k':>4}  {'inertia':>14}  {'silhouette':>12}")
    print("-" * 36)
    best_k, best_sil = k_min, -1
    for k in range(k_min, k_max + 1):
        km = KMeans(n_clusters=k, n_init=20, random_state=42)
        labels = km.fit_predict(X_scaled)
        inertia = km.inertia_
        sil = silhouette_score(X_scaled, labels, sample_size=min(5000, len(X_scaled)))
        print(f"{k:>4}  {inertia:>14.1f}  {sil:>12.4f}")
        if sil > best_sil:
            best_sil, best_k = sil, k
    print(f"\nBest k by silhouette: {best_k} (score={best_sil:.4f})")
    return best_k

def describe_clusters(df, features, labels, feature_names):
    df = df.copy()
    df["cluster"] = labels
    features = features.copy()
    features["cluster"] = labels

    print("\n=== Cluster summary ===")
    for c in sorted(df["cluster"].unique()):
        mask = df["cluster"] == c
        n = mask.sum()
        print(f"\nCluster {c}  ({n} CC/AS pairs)")
        top_cc = df.loc[mask, "cc"].value_counts().head(5)
        print(f"  Top CCs: {', '.join(f'{cc}({cnt})' for cc, cnt in top_cc.items())}")
        centroid = features.loc[mask, feature_names].mean()
        for feat, val in centroid.items():
            print(f"  {feat:<22} {val:.4f}")

def plot_clusters(X_scaled, labels, out_path):
    from sklearn.decomposition import PCA
    pca = PCA(n_components=2, random_state=42)
    coords = pca.fit_transform(X_scaled)
    fig, ax = plt.subplots(figsize=(9, 6))
    scatter = ax.scatter(coords[:, 0], coords[:, 1],
                         c=labels, cmap="tab10", s=8, alpha=0.5)
    ax.set_xlabel(f"PC1 ({pca.explained_variance_ratio_[0]*100:.1f}%)")
    ax.set_ylabel(f"PC2 ({pca.explained_variance_ratio_[1]*100:.1f}%)")
    ax.set_title("CC/AS clusters (PCA projection)")
    plt.colorbar(scatter, ax=ax, label="cluster")
    plt.tight_layout()
    plt.savefig(out_path, dpi=150)
    print(f"PCA plot saved to {out_path}")

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("input_csv")
    parser.add_argument("output_csv")
    parser.add_argument("--clusters", type=int, default=0,
                        help="number of clusters (0 = auto-select via silhouette sweep)")
    parser.add_argument("--sweep", action="store_true",
                        help="print silhouette scores for k=2..12 and exit")
    parser.add_argument("--plot", metavar="FILE",
                        help="save PCA scatter plot to FILE (e.g. clusters.png)")
    args = parser.parse_args()

    df = pd.read_csv(args.input_csv)
    print(f"Loaded {len(df)} rows, {len(df.columns)} columns.")

    features = build_features(df)
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

    describe_clusters(df, features, labels, feature_names)

    df_out = df[["cc", "as", "total_uids"]].copy()
    df_out["cluster"] = labels
    # Append the feature values so the output is self-contained
    for col in feature_names:
        df_out[col] = features[col].values
    df_out.to_csv(args.output_csv, index=False)
    print(f"\nWrote {len(df_out)} rows to {args.output_csv}")

    if args.plot:
        plot_clusters(X_scaled, labels, args.plot)

if __name__ == "__main__":
    main()
