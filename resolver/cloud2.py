#!/usr/bin/env python3
import argparse
import sys
from pathlib import Path
from io import BytesIO

import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.ticker import PercentFormatter, AutoLocator, AutoMinorLocator
from PIL import Image

# --- Utilities ----------------------------------------------------------------

COLUMN_ALIASES = {
    "slice_time": {"slice_time", "timestamp", "time", "slice-timestamp"},
    "cloud_unique_metric": {"cloud_unique_metric", "cloud_uniqe_metric", "cloud_unique", "cloud_metric"},
    "uids": {"uids", "uid", "users", "unique_ids"},
}

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
            raise KeyError(f"Missing required column for '{target}'. Present: {list(df.columns)}")
        resolved[target] = found
    return resolved

def unix_to_datetime(series: pd.Series) -> pd.Series:
    s_clean = pd.to_numeric(series, errors="coerce")
    if s_clean.dropna().empty:
        return pd.to_datetime(series, errors="coerce")
    unit = "ms" if s_clean.dropna().median() >= 1e12 else "s"
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
    use = df[[cols["slice_time"], cols["cloud_unique_metric"], cols["uids"]]].copy()
    use["__dt"] = unix_to_datetime(use[cols["slice_time"]])
    use = use.dropna(subset=["__dt"]).sort_values("__dt")

    y_percent = to_percentage(use[cols["cloud_unique_metric"]])
    y_uids = pd.to_numeric(use[cols["uids"]], errors="coerce")

    mask_valid = ~(y_percent.isna() & y_uids.isna())
    use, y_percent, y_uids = use.loc[mask_valid], y_percent.loc[mask_valid], y_uids.loc[mask_valid]

    if use.empty:
        print(f"[WARN] {file_path.name}: no valid data.", file=sys.stderr)
        return

    fig, ax_left = plt.subplots(figsize=(10, 5), dpi=150)

    line_left, = ax_left.plot(use["__dt"], y_percent, lw=0.8, label="cloud_unique_metric (%)", color="red")
    ax_left.set_ylabel("cloud_unique_metric (%)", color="red")
    ax_left.yaxis.set_major_formatter(PercentFormatter(xmax=100))
    ax_left.tick_params(axis="y", labelcolor="red")
    ax_left.yaxis.set_major_locator(AutoLocator())
    ax_left.yaxis.set_minor_locator(AutoMinorLocator())

    ax_right = ax_left.twinx()
    line_right, = ax_right.plot(
        use["__dt"], y_uids, lw=0.8, linestyle=":", label="uids", color="black"
    )
    ax_right.set_ylabel("uids", color="black")
    ax_right.tick_params(axis="y", labelcolor="black")

    ax_left.set_xlabel("date")
    fig.autofmt_xdate(rotation=30, ha="right")
    ax_left.set_title(file_path.name)
    ax_left.grid(True, linestyle="--", linewidth=0.5, alpha=0.6)

    ax_left.legend([line_left, line_right], ["cloud_unique_metric (%)", "uids"], loc="upper left")

    out_path = file_path.with_suffix(".jpg")
    plt.tight_layout()
    save_fig_as_jpeg(fig, out_path)
    plt.close(fig)
    print(f"[OK] Saved: {out_path}")

# --- Main ----------------------------------------------------------------------

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
