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

Calling ithitools with the option "-h" will produce a standard looking "usage"
page. Further documentation is available in DnsProtocolParametersAnalysis.pdf.

# Building ITHITOOLS

ITHITOOLS was developed in C++, and can be built under Windows or Linux.

## ITHITOOLS on Windows

To build ITHITOOLS on Windows, you need to:

 * Have a version of Visual Studio 2017 installed. The freely available
   "Community" version will work.

 * Clone and compile ITHITOOLS, using the Visual Studio 2017 solution 
   ithitoolsvs.sln included in the sources.

 * You can use the unit tests included in the Visual Studio solution to 
   verify the port.

## ITHITOOLS on Linux

To build ITHITOOLS on Linux, you need to:

 * Clone and compile ITHITOOLS:
~~~
   cmake .
   make
~~~
 * Run the test program "ithitest" to verify the port.

