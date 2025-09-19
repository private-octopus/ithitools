#!/usr/bin/env python3
import argparse
import sys
from pathlib import Path
from io import BytesIO

import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.ticker import PercentFormatter, AutoLocator, AutoMinorLocator
from PIL import Image

COLUMN_ALIASES = {
    "slice_time": {"slice_time", "timestamp", "time"},
    "https_isp_metric": {"https_isp_metric", "https-isp-metric", "https_isp"},
    "https_pnds_metric": {"https_pnds_metric", "https-pnds-metric", "https_pnds"},
    "nb_uids": {"nb_uids", "uids", "nb_users", "users"},
}

def resolve_columns(df):
    cols_lower = {c.lower(): c for c in df.columns}
    resolved = {}
    for target, candidates in COLUMN_ALIASES.items():
        found = None
        for cand in candidates:
            if cand in cols_lower:
                found = cols_lower[cand]
                break
        if not found:
            raise KeyError(f"Missing column {target}, got {list(df.columns)}")
        resolved[target] = found
    return resolved

def unix_to_datetime(series):
    s_clean = pd.to_numeric(series, errors="coerce")
    if s_clean.dropna().empty:
        return pd.to_datetime(series, errors="coerce")
    unit = "ms" if s_clean.dropna().median() >= 1e12 else "s"
    return pd.to_datetime(s_clean, unit=unit, errors="coerce")

def to_percentage(series):
    s = pd.to_numeric(series, errors="coerce")
    if s.max() is not None and pd.notna(s.max()) and s.max() <= 1.0:
        return s * 100
    return s

def save_fig_as_jpeg(fig, out_path: Path, dpi=150, quality=95):
    buf = BytesIO()
    fig.savefig(buf, format="png", dpi=dpi, bbox_inches="tight", facecolor="white")
    buf.seek(0)
    with Image.open(buf) as im:
        if im.mode in ("RGBA", "LA", "P"):
            im = im.convert("RGB")
        im.save(out_path, "JPEG", quality=quality, optimize=True, progressive=True, subsampling=0)

def make_plot(df, file_path: Path, cols):
    use = df[[cols["slice_time"], cols["https_isp_metric"], cols["https_pnds_metric"], cols["nb_uids"]]].copy()
    use["__dt"] = unix_to_datetime(use[cols["slice_time"]])
    use = use.dropna(subset=["__dt"]).sort_values("__dt")

    y_isp = to_percentage(use[cols["https_isp_metric"]])
    y_pnds = to_percentage(use[cols["https_pnds_metric"]])
    y_uids = pd.to_numeric(use[cols["nb_uids"]], errors="coerce")

    mask = ~(y_isp.isna() & y_pnds.isna() & y_uids.isna())
    use, y_isp, y_pnds, y_uids = use.loc[mask], y_isp.loc[mask], y_pnds.loc[mask], y_uids.loc[mask]

    if use.empty:
        print(f"[WARN] {file_path.name}: no valid data.")
        return

    fig, ax_left = plt.subplots(figsize=(10, 5), dpi=150)

    line_isp, = ax_left.plot(use["__dt"], y_isp, lw=0.8, label="https_isp_metric (%)", color="red")
    line_pnds, = ax_left.plot(use["__dt"], y_pnds, lw=0.8, label="https_pnds_metric (%)", color="blue")
    ax_left.set_ylabel("Percentage (%)")
    ax_left.yaxis.set_major_formatter(PercentFormatter(xmax=100))
    ax_left.tick_params(axis="y")
    ax_left.yaxis.set_major_locator(AutoLocator())
    ax_left.yaxis.set_minor_locator(AutoMinorLocator())

    ax_right = ax_left.twinx()
    line_uids, = ax_right.plot(
        use["__dt"], y_uids, lw=0.8, linestyle=":", label="nb_uids", color="black"
    )
    ax_right.set_ylabel("nb_uids", color="black")
    ax_right.tick_params(axis="y", labelcolor="black")

    ax_left.set_xlabel("date")
    fig.autofmt_xdate(rotation=30, ha="right")
    ax_left.set_title(file_path.name)
    ax_left.grid(True, linestyle="--", linewidth=0.5, alpha=0.6)

    ax_left.legend([line_isp, line_pnds, line_uids],
                   ["https_isp_metric (%)", "https_pnds_metric (%)", "nb_uids"],
                   loc="upper left")

    out_path = file_path.with_suffix(".jpg")
    plt.tight_layout()
    save_fig_as_jpeg(fig, out_path)
    plt.close(fig)
    print(f"[OK] Saved: {out_path}")

def main():
    parser = argparse.ArgumentParser()
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
