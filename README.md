# ITHITOOLS

The ITHITOOLS program include a set of tools designed to compute some of
"Identifier Technology Health Indicators" (ITHI) metrics defined by ICANN.
The identifiers include Names, Numbers, and Protocol Parameters.
A general presentation of the ITHI work is available here: https://www.icann.org/ithi.
The capture part of the tools are focused on a subset of the ITHI problem, 
specifically the computation of the IHTI metrics M3, M4 and M6:

* overhead in root traffic (M3.1, and M3.2 for duplicate requests contained in single PCAP file)
* leakage of RFC6761 names and other undelegated names (M4)
* usage of DNS protocol parameters defined in IANA registries (M6) plus list of unregistered parameters and frequencies.

The metric part of the tools reads data from the ITHI input folders, and produces the
ITHI metrics.

# Usage

The project builds a single executable, "ithitools.exe", which can be used in two modes:

* analysis of a single PCAP file to produce a summary file (CSV format) containing the counts of
  interesting parameter found in the file.

* aggregation of several summary files and extraction of supported metrics.

Calling ithitools with the option "-h" will produce a standard looking "usage"
page. Further documentation is available in DnsProtocolParametersAnalysis.pdf.

On Linux systems, the project also builds a shared library, "ithicap" (libithicap.so). This
library is meant to be used as an extension to "dnscap". A typical usage would be:
~~~
    dnscap <dnscap-parameters> -P libithicap.so -o <ithi-capture-file.csv>
~~~
The "ithicap" capture options can be displayed with the option -h, as in:
~~~
    dnscap <dnscap-parameters> -P libithicap.so -h
~~~ 


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

### Build dependencies on Linux

Building ITHITOOLS on Linux requires installation of CMAKE and of a C++ compiler. 
The installation tools depend on the Linux version.

On Ubuntu, the recommended way to install the GCC/C++ compiler is by installing
the "build essentials":
~~~
   sudo apt-get install build-essential
~~~
The CMAKE package can of course be installed as:
~~~
   sudo apt-get install cmake
~~~
By default, CMAKE will create a make file that reference the default C/C++
compilers for your system, typically gcc/g++. If you want to use a different
compiler, for example CLANG, you can either change your system's defaults,
or set explicit arguments to CMAKE, such as:
~~~
   cmake -D CMAKE_C_COMPILER="/usr/bin/clang" -D CMAKE_CXX_COMPILER "/usr/bin/clang++" .
~~~
The exact value of the arguments depend of course of where the compilers
are installed.
