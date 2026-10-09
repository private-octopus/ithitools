# Compute per-CC/AS statistics from the compressed JSON produced by rsv_pdns_file.py.
#
# Metrics computed per CC/AS:
#   frac_AAAA           - fraction of AAAA unique queries over total unique queries
#   frac_HTTPS          - fraction of HTTPS unique queries over total unique queries
#   first_<prov>        - fraction of queries for which this provider received the absolute first
#                         query (abs["0ms"]); fractions across all providers sum to ~100%
#   frac_<prov>         - fraction of unique queries sent to this provider at least once
#                         (rel["0ms"]); fractions sum to the duplication ratio
#   duplication         - sum of frac_<prov> across all providers (>1 = same query sent to
#                         multiple providers)
#   rep_short_ISP       - short-term ISP repetition ratio: UIDs whose first arrival at ISP4 or
#                         ISP6 was within 100ms of query_ad_time (abs buckets u10ms..u100ms),
#                         divided by total ISP-reaching UIDs (uids_isp summed across RR types)
#   rep_long_ISP        - long-term ISP repetition ratio: same but 100ms < delay <= 30s

import sys
import os
import bz2
import json
import csv

Prov_names = [
    'ISP4', 'ISP6', 'googlepdns', 'cloudflare', 'opendns', 'quad9',
    'level3', 'neustar', 'he', 'onedns', 'dnspai', '114dns', 'dnspod',
    'cnnic', 'twnic', 'quad101', 'greenteamdns', 'freenom', 'uncensoreddns',
    'vrsgn', 'same_CC', 'others'
]

# abs buckets that represent short-term cross-protocol repeats (0 < delay <= 100ms)
SHORT_REP_BUCKETS = ["u10ms", "u30ms", "u100ms"]
# abs buckets that represent long-term cross-protocol repeats (100ms < delay <= 30s)
LONG_REP_BUCKETS  = ["u300ms", "u1s", "u3s", "u10s", "u30s"]

def bucket_count(val):
    """Return integer count from a time-slice value (plain int or {count:N,...} dict)."""
    if isinstance(val, dict):
        return val.get("count", 0)
    return val if val else 0

def prov_first_queries_abs(prov_j):
    """UIDs for which this provider received the absolute first query (abs["0ms"]).
    Mutually exclusive across providers: fractions sum to ~100% of total UIDs."""
    return bucket_count(prov_j.get("abs", {}).get("0ms", 0))

def prov_first_queries_rel(prov_j):
    """UIDs that sent at least one query to this provider (rel["0ms"]).
    A UID may appear in multiple providers: fractions sum to the duplication ratio."""
    return bucket_count(prov_j.get("rel", {}).get("0ms", 0))

def prov_abs_rep_counts(prov_j):
    """Return (short_rep, long_rep) from abs time slices.

    abs[bucket] counts UIDs whose first query to this provider arrived at that delay
    relative to query_ad_time.  Buckets beyond "0ms" therefore represent UIDs that
    first contacted another provider, then came back to this one — cross-protocol repeats
    when comparing ISP4 vs ISP6.
    """
    abs_j = prov_j.get("abs", {})
    short = sum(bucket_count(abs_j.get(b, 0)) for b in SHORT_REP_BUCKETS)
    long  = sum(bucket_count(abs_j.get(b, 0)) for b in LONG_REP_BUCKETS)
    return short, long

def compute_stats(asn_j):
    """Return a dict of statistics for one CC/AS entry."""
    cc    = asn_j.get("cc", "??")
    asn   = asn_j.get("as", "??")
    total = asn_j.get("uids", 0)
    rr    = asn_j.get("rr", {})

    uids_AAAA  = rr.get("AAAA",  {}).get("uids", 0)
    uids_HTTPS = rr.get("HTTPS", {}).get("uids", 0)

    def frac(n):
        return n / total if total > 0 else 0.0

    row = {
        "cc":         cc,
        "as":         asn,
        "total_uids": total,
        "frac_AAAA":  frac(uids_AAAA),
        "frac_HTTPS": frac(uids_HTTPS),
    }

    # Aggregate provider counts across all RR types.
    prov_abs  = {p: 0 for p in Prov_names}  # abs["0ms"]: absolute first query per provider
    prov_rel  = {p: 0 for p in Prov_names}  # rel["0ms"]: at-least-once per provider
    isp_short = 0   # UIDs whose first arrival at ISP4 or ISP6 was within 100ms
    isp_long  = 0   # UIDs whose first arrival at ISP4 or ISP6 was 100ms–30s
    uids_isp  = 0   # total UIDs reaching ISP4 or ISP6 (denominator for rep ratios)

    for rr_name, rr_j in rr.items():
        uids_isp += rr_j.get("uids_isp", 0)
        for prov_name, prov_j in rr_j.get("providers", {}).items():
            if prov_name in prov_abs:
                prov_abs[prov_name] += prov_first_queries_abs(prov_j)
                prov_rel[prov_name] += prov_first_queries_rel(prov_j)
            if prov_name in ("ISP4", "ISP6"):
                s, l = prov_abs_rep_counts(prov_j)
                isp_short += s
                isp_long  += l

    for p in Prov_names:
        row["first_" + p] = frac(prov_abs[p])
        row["frac_"  + p] = frac(prov_rel[p])

    row["duplication"] = frac(sum(prov_rel.values()))

    # ISP repetition ratios use uids_isp as denominator (unique UIDs reaching ISP4 or ISP6)
    row["rep_short_ISP"] = isp_short / uids_isp if uids_isp > 0 else 0.0
    row["rep_long_ISP"]  = isp_long  / uids_isp if uids_isp > 0 else 0.0

    return row

def process_file(json_file, output_file, min_uids=0):
    if json_file.endswith(".bz2"):
        F = bz2.open(json_file, "rt")
    else:
        F = open(json_file, "r")
    with F:
        data = json.load(F)

    rows = []
    for asn_j in data.get("asns", []):
        if asn_j.get("uids", 0) >= min_uids:
            rows.append(compute_stats(asn_j))

    if not rows:
        print("No data found.")
        return

    fieldnames = ["cc", "as", "total_uids", "frac_AAAA", "frac_HTTPS"] + \
                 ["first_" + p for p in Prov_names] + \
                 ["frac_"  + p for p in Prov_names] + \
                 ["duplication", "rep_short_ISP", "rep_long_ISP"]

    with open(output_file, "w", newline="") as out:
        writer = csv.DictWriter(out, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)

    print(f"Wrote {len(rows)} rows to {output_file}")

def usage():
    print("Usage: python rsv_pdns_stats.py <input.json[.bz2]> <output.csv> [min_uids]")
    print("  min_uids: minimum total unique queries to include a CC/AS (default 0)")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        usage()
        sys.exit(1)
    input_file  = sys.argv[1]
    output_file = sys.argv[2]
    min_uids    = int(sys.argv[3]) if len(sys.argv) > 3 else 0

    if not os.path.exists(input_file):
        print(f"Input file not found: {input_file}")
        sys.exit(1)

    process_file(input_file, output_file, min_uids)
