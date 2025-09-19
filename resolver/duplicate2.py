#!/usr/bin/env python3
import argparse
import sys
from pathlib import Path
from io import BytesIO

import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.ticker import PercentFormatter, AutoLocator, AutoMinorLocator
from PIL import Image

# --- Column aliases ------------------------------------------------------------

COLUMN_ALIASES = {
    "slice_time": {"slice_time", "timestamp", "time", "slice-timestamp"},
    "rr_name": {"rr_name", "rr", "rrtype", "rr_type", "record_type", "dns_rr"},
    "dups_metric": {"dups_metric", "dups", "duplicates_metric", "dup_metric", "dup_rate"},
    "uids": {"uids", "uid", "users", "unique_ids", "nb_uids", "nb_users"},
}

RR_TARGETS = {
    "HTTPS": {"HTTPS"},
    "A": {"A"},
    "AAAA": {"AAAA"},
}

# --- Helpers -------------------------------------------------------------------

def resolve_columns(df: pd.DataFrame):
    cols_lower = {c.lower(): c for c in df.columns}
    resolved = {}
    for target, candidates in COLUMN_ALIASES.items():
        found = None
        for cand in candidates:
            if cand in cols_lower:
                found = cols_lower[cand]
                break
        if not found and target in cols_lower:
            found = cols_lower[target]
        if not found:
            raise KeyError(
                f"Missing required column for '{target}'. "
                f"Tried any of: {sorted(candidates)}. Present columns: {list(df.columns)}"
            )
        resolved[target] = found
    return resolved

def unix_to_datetime(series: pd.Series) -> pd.Series:
    s_clean = pd.to_numeric(series, errors="coerce")
    if s_clean.dropna().empty:
        return pd.to_datetime(series, errors="coerce")
    median_val = s_clean.dropna().median()
    unit = "ms" if median_val >= 1e12 else "s"
    return pd.to_datetime(s_clean, unit=unit, errors="coerce")

def to_percentage(series: pd.Series) -> pd.Series:
    s = pd.to_numeric(series, errors="coerce")
    if s.max() is not None and pd.notna(s.max()) and s.max() <= 1.0:
        return s * 100.0
    return s

def save_fig_as_jpeg(fig: plt.Figure, out_path: Path, dpi: int = 150, quality: int = 95):
    buf = BytesIO()
    fig.savefig(buf, format="png", dpi=dpi, bbox_inches="tight", facecolor="white")
    buf.seek(0)
    with Image.open(buf) as im:
        if im.mode in ("RGBA", "LA", "P"):
            im = im.convert("RGB")
        im.save(out_path, "JPEG", quality=quality, optimize=True, progressive=True, subsampling=0)

# --- Plotting ------------------------------------------------------------------

def make_plot(df: pd.DataFrame, file_path: Path, cols: dict):
    use = df[[cols["slice_time"], cols["rr_name"], cols["dups_metric"], cols["uids"]]].copy()
    use["__dt"] = unix_to_datetime(use[cols["slice_time"]])
    use = use.dropna(subset=["__dt"]).sort_values("__dt")

    rr_series = use[cols["rr_name"]].astype(str).str.upper().str.strip()

    def subset_rr(rr_value_set):
        mask = rr_series.isin(rr_value_set)
        sub = use.loc[mask, ["__dt", cols["dups_metric"]]].copy()
        if sub.empty:
            return sub
        sub["y"] = to_percentage(sub[cols["dups_metric"]])
        return sub[["__dt", "y"]]

    data_https = subset_rr(RR_TARGETS["HTTPS"])
    data_a = subset_rr(RR_TARGETS["A"])
    data_aaaa = subset_rr(RR_TARGETS["AAAA"])

    y_uids = pd.to_numeric(use[cols["uids"]], errors="coerce")

    if (data_https.empty and data_a.empty and data_aaaa.empty) and y_uids.dropna().empty:
        print(f"[WARN] {file_path.name}: no valid data to plot; skipping.", file=sys.stderr)
        return

    fig, ax_left = plt.subplots(figsize=(10, 5), dpi=150)

    lines = []
    labels = []

    if not data_https.empty:
        l_https, = ax_left.plot(data_https["__dt"], data_https["y"], lw=0.8, color="red", label="dups_metric HTTPS (%)")
        lines.append(l_https); labels.append("dups_metric HTTPS (%)")

    if not data_a.empty:
        l_a, = ax_left.plot(data_a["__dt"], data_a["y"], lw=0.8, color="blue", label="dups_metric A (%)")
        lines.append(l_a); labels.append("dups_metric A (%)")

    if not data_aaaa.empty:
        l_aaaa, = ax_left.plot(data_aaaa["__dt"], data_aaaa["y"], lw=0.8, color="green", label="dups_metric AAAA (%)")
        lines.append(l_aaaa); labels.append("dups_metric AAAA (%)")

    ax_left.set_ylabel("Percentage (%)")
    ax_left.yaxis.set_major_formatter(PercentFormatter(xmax=100))
    ax_left.yaxis.set_major_locator(AutoLocator())
    ax_left.yaxis.set_minor_locator(AutoMinorLocator())

    # Right axis (uids): light grey dotted line
    ax_right = ax_left.twinx()
    l_uids, = ax_right.plot(use["__dt"], y_uids, lw=0.8, linestyle=":", color="lightgrey", label="uids")
    ax_right.set_ylabel("uids", color="grey")
    ax_right.tick_params(axis="y", labelcolor="grey")

    lines.append(l_uids); labels.append("uids")

    ax_left.set_xlabel("date")
    fig.autofmt_xdate(rotation=30, ha="right")
    ax_left.set_title(file_path.name)
    ax_left.grid(True, which="major", linestyle="--", linewidth=0.5, alpha=0.6)

    ax_left.legend(lines, labels, loc="upper left")

    out_path = file_path.with_suffix(".jpg")
    plt.tight_layout()
    save_fig_as_jpeg(fig, out_path)
    plt.close(fig)
    print(f"[OK] Saved: {out_path}")

# --- Main ----------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Plot dups_metric (HTTPS/A/AAAA) vs. time (left axis, %), and uids (right axis, light grey dotted)."
    )
    parser.add_argument("files", nargs="+")
    parser.add_argument("--delimiter", "-d", default=None)
    parser.add_argument("--encoding", "-e", default=None)
    args = parser.parse_args()

    for f in args.files:
        path = Path(f)
        try:
            df = pd.read_csv(path, sep=args.delimiter, encoding=args.encoding)
            cols = resolve_columns(df)
            make_plot(df, path, cols)
        except Exception as e:
            print(f"[ERROR] {path.name}: {e}", file=sys.stderr)

if __name__ == "__main__":
    main()
