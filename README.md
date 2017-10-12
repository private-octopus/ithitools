# ITHITOOLS

The ITHITOOLS program include a set of tools designed to compute some of
"Identifier Technology Health Indicators" (ITHI) metrics defined by ICANN.
The identifiers include Names, Numbers, and Protocol Parameters.
A general presentation of the ITHI work is available here: https://www.icann.org/ithi.
The tools are focused on a subset of the ITHI problem, 
specifically the computation of the IHTI metrics M3, M4 and M6:

* overhead in root traffic (M3.1, and M3.2 for duplicate requests contained in single PCAP file)
* leakage of RFC6761 names and other undelegated names (M4)
* usage of DNS protocol parameters defined in IANA registries (M6) plus list of unregistered parameters and frequencies.

# Usage

The project builds a single executable, "ithitools.exe", which can be used in two modes:

* analysis of a single PCAP file to produce a summary file (CSV format) containing the counts of
  interesting parameter found in the file.

* aggregation of several summary files and extraction of supported metrics.

# Development

The tool is developed as a Visual Studio project, and should run under either Windows or Unix.
That is, this will be true as soon as we reach a sufficient stage of development...


