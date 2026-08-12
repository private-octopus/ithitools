# Classification of CC+AS traffic by client characteristics extracted
# from the "UA string" summaries produced by rsv_ua_strings.py.
#
# Input: one or more JSON (or bz2-compressed JSON) files with the format
# produced by rsv_ua_strings.py / ua_string_log.to_json():
#
# { "log": [
#     { "ua": "<user agent string>", "n": <total count>,
#       "cc_as": [ { "cc": "<CC>", "AS": "AS<n>", "n": <count> }, ... ] },
#     ...
# ]}
#
# For each UA string we use the "user_agents" library (pip install user-agents)
# to derive a device class (Windows, Mac, iPad, iPhone, Android phone/tablet,
# other, or bot), the OS family/version, and the browser family/version. We
# then accumulate these classifications per CC+AS, weighted by the "n" count
# of each (ua, cc_as) pair.
#
# "How fast are clients updated" is approximated by comparing, for each
# OS family, the major version used by a given CC+AS against the most
# recent major version observed anywhere in the input (the "global" latest).
# This is a relative measure -- it tells us whether an AS's clients lag
# behind the rest of the observed population, not whether they are behind
# the vendor's actual current release.
#
# Usage:
#   python rsv_cc_as_ua.py <output.json> <input-file> [<input-file> ...]
#
# Input files may be plain JSON or bz2-compressed JSON (as produced for the
# daily and monthly "ua-strings" summaries). The output is always plain JSON.

import bz2
import concurrent.futures
import json
import os
import re
import sys
import time
import traceback
from collections import defaultdict

try:
    from user_agents import parse as parse_ua
except ImportError:
    print("The 'user_agents' package is required: pip install user-agents")
    raise


def usage():
    print("Usage: python rsv_cc_as_ua.py <output.json> <input-file> [<input-file> ...]")
    print("  <output.json>   path of the JSON file to create")
    print("  <input-file>    one or more UA-string summary files, as produced by")
    print("                  rsv_ua_strings.py. May be plain JSON or bz2-compressed JSON.")
    exit(-1)


# ua_string_log.to_json() in rsv_ua_strings.py builds JSON by string
# concatenation and does not escape quotes or backslashes that occur inside
# the UA string itself (e.g. a device model like `TAB Q10"s`). That breaks
# strict JSON parsing. Since the writer always emits one record per line in
# a fixed shape, we can recover the fields from a line-oriented regex when
# json.loads() fails, instead of choking on the malformed file.
_RECORD_RE = re.compile(r'^\{"ua": "(.*)", "n":(\d+), "cc_as": \[(.*)\]\}$')
_CC_AS_RE = re.compile(r'\{\s*"cc":\s*"([A-Za-z]{2})","AS":\s*"(AS\d+)",\s*"n":\s*(\d+)\s*\}')


def parse_log_text_lenient(raw_text):
    log = []
    nb_skipped = 0
    for line in raw_text.splitlines():
        line = line.strip()
        if not line.startswith('{"ua":'):
            continue
        if line.endswith(','):
            line = line[:-1]
        m = _RECORD_RE.match(line)
        if not m:
            nb_skipped += 1
            continue
        ua_string = m.group(1)
        n = int(m.group(2))
        cc_as_list = [
            {"cc": cm.group(1), "AS": cm.group(2), "n": int(cm.group(3))}
            for cm in _CC_AS_RE.finditer(m.group(3))
        ]
        log.append({"ua": ua_string, "n": n, "cc_as": cc_as_list})
    if nb_skipped:
        print("Lenient parser skipped %d unparsable record line(s)." % nb_skipped)
    return {"log": log}


def load_json_file(path):
    """Load a JSON object from a plain or bz2-compressed file."""
    try:
        with bz2.open(path, "rt", encoding="utf-8") as F:
            raw_text = F.read()
    except OSError:
        with open(path, "rt", encoding="utf-8") as F:
            raw_text = F.read()
    # Some daily files carry stray escaped quote/backslash artifacts,
    # as already worked around in rsv_ua_strings.py.
    raw_text = raw_text.replace("\\x22", "").replace("\\x5C", "")
    try:
        return json.loads(raw_text)
    except json.JSONDecodeError as exc:
        print("Strict JSON parse of " + path + " failed (" + str(exc) +
              "); falling back to a lenient line parser because the UA-string "
              "writer does not escape embedded quotes.")
        return parse_log_text_lenient(raw_text)


def major_version(version_tuple):
    if not version_tuple:
        return ""
    return str(version_tuple[0])


class ua_classifier:
    """Parses and caches the classification of UA strings."""

    def __init__(self):
        self.cache = dict()

    def classify(self, ua_string):
        c = self.cache.get(ua_string)
        if c is None:
            c = self._classify(ua_string)
            self.cache[ua_string] = c
        return c

    def _classify(self, ua_string):
        ua = parse_ua(ua_string)
        os_family = ua.os.family or "Other"
        os_major = major_version(ua.os.version)
        browser_family = ua.browser.family or "Other"

        if ua.is_bot:
            device_class = "Bot"
        elif os_family == "iOS":
            device_class = "iPad" if "iPad" in (ua.device.family or "") else "iPhone"
        elif os_family == "Mac OS X":
            device_class = "Mac"
        elif os_family == "Windows":
            device_class = "Windows"
        elif os_family == "Android":
            device_class = "Android Tablet" if ua.is_tablet else "Android Phone"
        elif ua.is_pc:
            device_class = "Other PC"
        else:
            device_class = "Other"

        return {
            "device_class": device_class,
            "os_family": os_family,
            "os_major": os_major,
            "os_version": ua.os.version_string or "",
            "browser_family": browser_family,
            "browser_version": ua.browser.version_string or "",
            "is_bot": ua.is_bot,
        }


class group_stats:
    """Accumulates classified UA counts for one group (a CC+AS, or the global total)."""

    def __init__(self):
        self.n = 0
        self.device_class = defaultdict(int)
        self.os_family = defaultdict(int)
        self.browser_family = defaultdict(int)
        # os_family -> os_major -> count
        self.os_versions = defaultdict(lambda: defaultdict(int))

    def add(self, c, n):
        self.n += n
        self.device_class[c["device_class"]] += n
        self.os_family[c["os_family"]] += n
        self.browser_family[c["browser_family"]] += n
        if c["os_major"]:
            self.os_versions[c["os_family"]][c["os_major"]] += n

    def os_update_summary(self, latest_major_by_family):
        summary = dict()
        for os_family, versions in self.os_versions.items():
            latest = latest_major_by_family.get(os_family)
            total = sum(versions.values())
            if total == 0:
                continue
            n_latest = versions.get(latest, 0) if latest is not None else 0
            sorted_majors = sorted(
                (v for v in versions if v.isdigit()), key=int, reverse=True
            )
            n_latest_or_prev = n_latest
            if latest is not None and latest in sorted_majors:
                idx = sorted_majors.index(latest)
                if idx + 1 < len(sorted_majors):
                    n_latest_or_prev += versions.get(sorted_majors[idx + 1], 0)
            summary[os_family] = {
                "latest_major_observed": latest,
                "pct_on_latest": round(n_latest / total, 4),
                "pct_on_latest_or_previous": round(n_latest_or_prev / total, 4),
                "versions": dict(versions),
            }
        return summary

    def to_json_obj(self, latest_major_by_family=None):
        obj = {
            "n": self.n,
            "device_class": dict(self.device_class),
            "os_family": dict(self.os_family),
            "browser_family": dict(self.browser_family),
        }
        if latest_major_by_family is not None:
            obj["os_update"] = self.os_update_summary(latest_major_by_family)
        return obj


def compute_latest_major_by_family(global_stats):
    latest = dict()
    for os_family, versions in global_stats.os_versions.items():
        numeric_majors = [v for v in versions if v.isdigit()]
        if numeric_majors:
            latest[os_family] = max(numeric_majors, key=int)
    return latest


def classify_batch(ua_list):
    """Classify a batch of UA strings. Runs in a worker process."""
    classifier = ua_classifier()
    return [(ua, classifier.classify(ua)) for ua in ua_list]


def chunked(seq, n_chunks):
    n_chunks = max(1, n_chunks)
    size = max(1, -(-len(seq) // n_chunks))  # ceil division
    return [seq[i:i + size] for i in range(0, len(seq), size)]


def classify_all(ua_strings, max_workers=None):
    """Classify every distinct UA string, in parallel if there are enough of them.

    Real monthly summaries carry hundreds of thousands of distinct UA strings,
    and each classification is a set of regex matches, so this step is the
    bottleneck; splitting it across processes is the same pattern already
    used in rsv_ua_strings.py for parsing the raw log files.
    """
    ua_list = list(ua_strings)
    if len(ua_list) == 0:
        return {}
    if max_workers is None:
        max_workers = os.cpu_count() or 1
    if len(ua_list) < 5000 or max_workers <= 1:
        classifier = ua_classifier()
        return {ua: classifier.classify(ua) for ua in ua_list}

    chunks = chunked(ua_list, max_workers)
    result = dict()
    with concurrent.futures.ProcessPoolExecutor(max_workers=len(chunks)) as executor:
        for batch_result in executor.map(classify_batch, chunks):
            for ua, c in batch_result:
                result[ua] = c
    return result


def process_files(input_files, max_workers=None):
    all_entries = []  # list of (ua_string, cc_as_list)
    ua_set = set()

    for path in input_files:
        print("Loading " + path)
        try:
            json_obj = load_json_file(path)
        except Exception as exc:
            traceback.print_exc()
            print("Cannot load " + path + ": " + str(exc))
            continue

        for entry in json_obj.get("log", []):
            ua_string = entry["ua"]
            cc_as_list = entry.get("cc_as", [])
            all_entries.append((ua_string, cc_as_list))
            ua_set.add(ua_string)

    print("Loaded %d (ua, cc_as-list) entries, %d distinct UA strings." %
          (len(all_entries), len(ua_set)))

    print("Classifying %d distinct UA strings..." % len(ua_set))
    t0 = time.time()
    classifications = classify_all(ua_set, max_workers)
    print("Classified %d UA strings in %.1f seconds." % (len(classifications), time.time() - t0))

    global_stats = group_stats()
    cc_as_stats = dict()  # (cc, AS) -> group_stats
    nb_entries = 0
    for ua_string, cc_as_list in all_entries:
        c = classifications[ua_string]
        for cc_as in cc_as_list:
            key = (cc_as["cc"], cc_as["AS"])
            n = cc_as["n"]
            if key not in cc_as_stats:
                cc_as_stats[key] = group_stats()
            cc_as_stats[key].add(c, n)
            global_stats.add(c, n)
            nb_entries += 1

    print("Accumulated %d (ua, cc_as) entries across %d distinct CC+AS." %
          (nb_entries, len(cc_as_stats)))
    return global_stats, cc_as_stats


def build_report(input_files, global_stats, cc_as_stats):
    latest_major_by_family = compute_latest_major_by_family(global_stats)

    cc_as_list = []
    for (cc, AS), stats in cc_as_stats.items():
        record = {"cc": cc, "AS": AS}
        record.update(stats.to_json_obj(latest_major_by_family))
        cc_as_list.append(record)
    cc_as_list.sort(key=lambda r: r["n"], reverse=True)

    report = {
        "generated": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "sources": input_files,
        "global": global_stats.to_json_obj(latest_major_by_family),
        "cc_as": cc_as_list,
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


# main

if __name__ == "__main__":
    if len(sys.argv) < 3:
        usage()

    output_path = sys.argv[1]
    input_files = sys.argv[2:]

    for f in input_files:
        if not os.path.isfile(f):
            print("Not a valid file: " + f)
            usage()

    time_start = time.time()
    global_stats, cc_as_stats = process_files(input_files)
    report = build_report(input_files, global_stats, cc_as_stats)
    write_report(report, output_path)
    print("Wrote " + output_path + " in %.1f seconds." % (time.time() - time_start))
