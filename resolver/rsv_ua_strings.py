# Analysis of the UA strings in the HTTPD logs from APNIC.
# The first step is to analyze the prevalence of US strings
# by As and query.
#
# Each log line contains a series of space separated fields:
# Example: am.rand.apnic.net 2601:8c3:8700:9600:6c1f:8904:3e73:586b [15/Jul/2026:00:00:01 +0000] "GET /q1x1.png?ub7be8866-s1784073601-i00000000.am.h3q HTTP/3.0" 200 68 "https://tpc.googlesyndication.com/" "Mozilla/5.0 (iPhone; CPU iPhone OS 18_7 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Mobile/15E148 Version/26.5.0" "TLSv1.3/TLS_AES_256_GCM_SHA384" "h3" 0.000 https 1784073601.743 3du-ub7be8866-c233-a1ef2-s1784073601-i00000000-0.am.dotnxdomain.net
#
# 0. server: am.rand.apnic.net
# 1. source-IP: 2601:8c3:8700:9600:6c1f:8904:3e73:586b
# 2. date-time: [15/Jul/2026:00:00:01
# 3. time_zone: +0000]
# 4. http-query-string: "GET /q1x1.png?ub7be8866-s1784073601-i00000000.am.h3q HTTP/3.0"
# 5. some-number: 200
# 6. another-number: 68
# 7. URL: "https://tpc.googlesyndication.com/"
# 8. UA-string: "Mozilla/5.0 (iPhone; CPU iPhone OS 18_7 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Mobile/15E148 Version/26.5.0"
# 9. TLS: "TLSv1.3/TLS_AES_256_GCM_SHA384"
# 10. ALPN: "h3"
# 11. yet-another-number: 0.000
# 12. method: https
# 13. some-date: 1784073601.743
# 14. query-string: 3du-ub7be8866-c233-a1ef2-s1784073601-i00000000-0.am.dotnxdomain.net
#
# The query string (3du-ub7be8866-c233-a1ef2-s1784073601-i00000000-0.am.dotnxdomain.net) has components similar to DNS queries.
#
# Some of the HTTP requests are not triggered by APNIC advertisements. An example would be:
#
# Example : am.rand.apnic.net 27.151.156.246 [15/Jul/2026:00:00:01 +0000] "GET /hkcfg/ad.py?A=VINT&N&R&F HTTP/2.0" 200 2546 "https://tpc.googlesyndication.com/" "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/108.0.5359.95 Safari/537.36 QIHU 360SE" "TLSv1.3/TLS_AES_256_GCM_SHA384" "" u1b746749-s1784073601 c48-a1026 https 1784073601.76
# 0 -> am.rand.apnic.net
# 1 -> 27.151.156.246
# 2 -> [15/Jul/2026:00:00:01
# 3 -> +0000]
# 4 -> GET /hkcfg/ad.py?A=VINT&N&R&F HTTP/2.0
# 5 -> 200
# 6 -> 2546
# 7 -> https://tpc.googlesyndication.com/
# 8 -> Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/108.0.5359.95 Safari/537.36 QIHU 360SE
# 9 -> TLSv1.3/TLS_AES_256_GCM_SHA384
# 10 ->""
# 11 -> u1b746749-s1784073601
# 12 -> c48-a1026
# 13 -> https
# 14 -> 1784073601.761
# 15 -> cfg.dotnxdomain.net
#
# Note that the h3 ALPN is absent (empty string). The "query name" is not there either. Such log lines should be ignored.
#
# Example: am.rand.apnic.net 2806:261:2481:833d:900:9239:1719:29ee [15/Jul/2026:00:00:01 +0000] "GET /q1x1.png?uf90e36e5-s1784073601-i00000000.am.h3q HTTP/2.0" 200 68 "https://tpc.googlesyndication.com/" "Mozilla/5.0 (Linux; Android 12; moto g51 5G Build/S2RYAS32.58-13-12-5-3-3; wv) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/149.0.7827.160 Mobile Safari/537.36 (Mobile; afma-sdk-a-v260480999.254715000.1)" "TLSv1.3/TLS_AES_256_GCM_SHA384" "" 0.000 https 1784073601.766 3du-uf90e36e5-c156-a36af-s1784073601-i00000000-0.am.dotnxdomain.net
#
# 0 -> am.rand.apnic.net
# 1 -> 2806:261:2481:833d:900:9239:1719:29ee
# 2 -> [15/Jul/2026:00:00:01
# 3 -> +0000]
# 4 -> GET /q1x1.png?uf90e36e5-s1784073601-i00000000.am.h3q HTTP/2.0
# 5 -> 200
# 6 -> 68
# 7 -> https://tpc.googlesyndication.com/
# 8 -> Mozilla/5.0 (Linux; Android 12; moto g51 5G Build/S2RYAS32.58-13-12-5-3-3; wv) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/149.0.7827.160 Mobile Safari/537.36 (Mobile; afma-sdk-a-v260480999.254715000.1)
# 9 -> TLSv1.3/TLS_AES_256_GCM_SHA384
# 10 -> ""
# 11 -> 0.000
# 12 -> https
# 13 -> 1784073601.766
# 14 -> 3du-uf90e36e5-c156-a36af-s1784073601-i00000000-0.am.dotnxdomain.net


import bz2
import sys
import os
import traceback
import rsv_log_parse
import concurrent.futures
import time
import rsv_candidates
import json
import glob

def usage():
    print("Usage: rsv_us_strings.py <year> <month> <day> <dest-dir> <source> ... ")
    exit(-1)

def parse_with_quotes(line):
    p_raw = line.split(' ')
    parts = []
    quoting = False
    quoted = ""
    first_quote = False
    for x in p_raw:
        if not quoting:
            if x.startswith('"'):
                x = x[1:]
                quoted = ""
                quoting = True
        if quoting:
            if x.endswith('"'):
                x = x[:-1]
                quoting = False
            if len(quoted) > 0:
                quoted += ' '
            quoted += x
            if not quoting:
                parts.append(quoted)
        else:
            parts.append(x)
    return parts

class ua_string_data:
    def __init__(self):
        self.as_0du = dict()
        self.n = 0

    def add_data(self, experiment, query_cc, query_AS):
        if experiment == "0du":
            self.n += 1
            key = query_cc + "-" + query_AS
            if not key in self.as_0du:
                self.as_0du[key] = 0
            self.as_0du[key] += 1

class ua_string_log:
    def __init__(self):
        self.uas = dict()
        self.ln = 0
        self.nb_ignored = 0
        self.nb_invalid = 0
        self.nb_processed = 0

    def add_log_line(self,s):
        self.ln += 1
        line = s.strip()
        parts = parse_with_quotes(line)
        if len(parts) != 15:
            if self.nb_ignored < 5:
                if parts[4].startswith("GET "):
                    print("Ignoring: " + parts[4])
                else:
                    print("Ignoring: " + line)
            self.nb_ignored += 1
        else:
            qn = rsv_log_parse.query_name_class()
            is_valid = qn.parse_query_name(parts[14])
            if not is_valid:
                if nb_invalid < 5:
                    print("Invalid query name in:\n" + line)
                self.nb_invalid += 1
            elif qn.query_experiment == "0du":
                if not parts[8] in self.uas:
                    self.uas[parts[8]] = ua_string_data()
                uasd = self.uas[parts[8]]
                uasd.add_data(qn.query_experiment, qn.query_cc, qn.query_AS)
                self.nb_processed += 1

    def to_json(self, F):
        F.write("{\"log\": [")
        sep = "\n"
        for ua_string in self.uas:
            F.write(sep + "{\"ua\": \"" +  ua_string + "\", \"n\":" + str(self.uas[ua_string].n) + ", \"cc_as\": [")
            sep_cc_as = ""
            for key in self.uas[ua_string].as_0du:
                query_cc = key[:2]
                query_AS = key[3:]
                F.write(sep_cc_as + "{ \"cc\": \"" + query_cc + "\",\"AS\": \"" + query_AS + "\", \"n\": " +
                        str(self.uas[ua_string].as_0du[key]) + "}")
                sep_cc_as = ","
            F.write("]}")
            sep = ",\n"
        F.write("\n]}\n")

    def from_json_obj(self, json_obj):
        """
        Merge data from a JSON object (as produced by to_json) into this instance.
        """
        for entry in json_obj.get("log", []):
            ua = entry["ua"]
            n = entry["n"]
            cc_as_list = entry.get("cc_as", [])
            if ua not in self.uas:
                # Create new ua_string_data and add all cc_as
                uasd = ua_string_data()
                uasd.n = n
                for cc_as in cc_as_list:
                    key = cc_as["cc"] + "-" + cc_as["AS"]
                    uasd.as_0du[key] = cc_as["n"]
                self.uas[ua] = uasd
            else:
                # Merge into existing ua_string_data
                uasd = self.uas[ua]
                uasd.n += n
                for cc_as in cc_as_list:
                    key = cc_as["cc"] + "-" + cc_as["AS"]
                    if key in uasd.as_0du:
                        uasd.as_0du[key] += cc_as["n"]
                    else:
                        uasd.as_0du[key] = cc_as["n"]

class log_file_bucket:
    def __init__(self, output_file, source_file, bucket_id, time_start):
        self.output_file = output_file
        self.source_file = source_file
        self.bucket_id = bucket_id
        self.time_start = time_start

    def load(self):
        usl = ua_string_log()

        try:
            with bz2.open(self.source_file, "rt") as F:
                print("Opened " + self.source_file + " for reading.")
                for line in F:
                    usl.add_log_line(line)
        except Exception as exc:
            traceback.print_exc()
            print("Opening " + source_file + " generated an exception: %s" + str(exc))
            usage()

        print("Processed: %d, Ignored: %d, Invalid: %d"%(usl.nb_processed, usl.nb_ignored, usl.nb_invalid))

        try:
            with bz2.open(self.output_file, "wt") as F:
                print("Opened " + self.output_file + " for writing.")
                usl.to_json(F)
        except Exception as exc:
            traceback.print_exc()
            print("Cannot write: " + self.output_file + " Exception: "  + str(exc))
            usage()

def load_bucket(bucket):
    print("Bucket[" + str(bucket.bucket_id) + "] loading " +
              bucket.source_file + " -> " + bucket.output_file)
    bucket.load()
    return True

servers=[ 'am', 'ap', 'eu', 'in', 'hk', 'la' ]
server_set= set(servers)
def server_from_source_file(fname):
    srv=""
    if fname[2:] == "-httpd-access.log.bz2":
        x = fname[:2]
        if x in server_set:
            srv=x
    return srv

def output_file_path_from_srv(outdir, srv, year, month, day):
    f_name="ua-strings-" + srv + "-" + year + "-" + month + "-" + day + ".json.bz2"
    f_path = os.path.join(outdir, f_name)
    return f_path

def get_daily_files_for_month(month_dir):
    if not os.path.isdir(month_dir):
        print("Not a valid directory: " + month_dir)
        return []
    pattern = os.path.join(
        month_dir, "ua-strings-*-" + year + "-" + month + "-*.json.bz2")
    files = glob.glob(pattern)
    return files

# main

if __name__ == "__main__":
    if len(sys.argv) < 5:
        usage()

    year=sys.argv[1]
    month=sys.argv[2]
    day=sys.argv[3]
    outdir=sys.argv[4]
    indir=sys.argv[5]
    d = rsv_candidates.date_of_day(year, month, day)
    d15 = rsv_candidates.getNdays(d, 15)

    # TODO: we ought to take year/month/day as input, then get a list of
    # the 15 previous days. Compute the source and output file names
    # accordingly, the output file names belonging to a temp file
    # per month. Then, derive the months for which the total
    # as change, and compute a ua string summary for these months.

    if not os.path.isdir(outdir):
        print("Not a valid directory: " + outdir)
        usage()

    if not os.path.isdir(indir):
        print("Not a valid directory: " + indir)
        usage()

    time_start=time.time()
    bucket_list=[]
    bucket_months = set()
    bucket_id = 0
    for dd in d15:
        s_dir = rsv_candidates.day_folder(indir, dd)
        if not os.path.isdir(s_dir):
            print("Not a valid directory: " + s_dir)
        else:
            sources = [
                os.path.join(s_dir, x) for x in os.listdir(s_dir)
                if x.endswith("-httpd-access.log.bz2")
            ]
            for source_path in sources:
                if not os.path.isfile(source_path):
                    print("Not a file: " + source_path)
                else:
                    source_file = os.path.basename(source_path)
                    srv = server_from_source_file(source_file)
                    if len(srv) != 2:
                        print("File name does not match template: " + source_file)
                    else:
                        o_dir = rsv_candidates.month_folder(outdir, dd)

                        output_path = output_file_path_from_srv(
                            o_dir, srv, year, month, day)
                        if not os.path.exists(output_path):
                            lfb = log_file_bucket(output_path, source_path, bucket_id, time_start)
                            bucket_list.append(lfb)
                            bucket_id += 1
                            bucket_month = str(dd.year) + "-" + "{:02d}".format(d.month)
                            if not bucket_month in bucket_months:
                                bucket_months.add(bucket_month)
                        else:
                            print("Output file already exists: " + output_path)


    nb_process = len(bucket_list)
    print("Will use " + str(nb_process) + " processes.")
    for lfb in bucket_list:
        print("Bucket[" + str(lfb.bucket_id) + "] " + lfb.source_file + " -> " +
              lfb.output_file)

    if nb_process > 0:
        with concurrent.futures.ProcessPoolExecutor(max_workers = nb_process) as executor:
            future_to_bucket = {executor.submit(load_bucket, bucket):bucket for bucket in bucket_list }
            for future in concurrent.futures.as_completed(future_to_bucket):
                bucket = future_to_bucket[future]
                try:
                    data = future.result()
                    print('Bucket %d complete' % (bucket.bucket_id))
                except Exception as exc:
                    traceback.print_exc()
                    print('Bucket %d generated an exception: %s' % (bucket.bucket_id, exc))
            bucket_time = time.time()
            print("all buckets completed after " + str(bucket_time - time_start) +
                  " seconds.")

    # All the buckets have been processed. Now, create the tables.

    for bucket_month in bucket_months:
        year = bucket_month[:4]
        month = bucket_month[5:7]
        year_dir = os.path.join(outdir, year)
        month_dir = os.path.join(year_dir, month)
        daily_files = get_daily_files_for_month(month_dir)
        if daily_files is None or len(daily_files) == 0:
            print("No daily files for month: " + bucket_month)
            continue
        month_output_path = os.path.join(
            month_dir, "ua-strings-summary-" + bucket_month + ".json.bz2")
        print("Creating monthly summary: " + month_output_path)
        uasl_total = ua_string_log()
        for daily_file in daily_files:
            if os.path.isfile(daily_file):
                try:
                    with bz2.open(daily_file, "rt") as F:
                        json_obj = json.load(F)
                        uasl_total.from_json_obj(json_obj)
                except Exception as exc:
                    traceback.print_exc()
                    print("Cannot read: " + daily_file + " Exception: " +
                          str(exc))
                    usage()
            else:
                print("Cannot access daily file: " + daily_file)
        try:
            with bz2.open(month_output_path, "wt") as F:
                uasl_total.to_json(F)
        except Exception as exc:
            traceback.print_exc()
            print("Cannot write: " + month_output_path + " Exception: " +
                  str(exc))
            usage()

        print("Monthly summary created: " + month_output_path)