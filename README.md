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
More information on configuring dnscap and the ithicap plugin can be found on
the [DBS capture settings wiki page](https://github.com/private-octopus/ithitools/wiki/Capture-of-DNS-statistics-using-dnscap-and-ithicap).

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
   git clone https://github.com/private-octopus/ithitools/
   cd ithitools
   cmake .
   make
~~~
 * Run the test program "ithitest" to verify the port.

Of course, if you want to just update to the latest release, you don't need to install
again. You will do something like:
~~~
   cd ithitools
   git pull --all
   cmake .
   make
~~~


### Build dependencies on Linux

Building ITHITOOLS on Linux requires installation of CMAKE and of a C++ compiler. 
The installation tools depend on the Linux version. We have tested the installation
on Ubuntu (16 and 17) and on Centos (6.9 and 7).

#### Ubuntu
On Ubuntu, the recommended way to install the GCC/C++ compiler is by installing
the "build essentials":
~~~
   sudo apt-get install build-essential
~~~
GIT and CMAKE can of course be installed as:
~~~
   sudo apt-get install cmake
   sudo apt-get install git
~~~

#### Centos
On Centos, you will need the GCC/C++ compiler, CMAKE and git:
~~~
   sudo yum install gcc
   sudo yum install gcc-c++
   sudo yum install cmake
   sudo yum install git
~~~

#### Compiling with CLANG
By default, CMAKE will create a make file that reference the default C/C++
compilers for your system, typically gcc/g++. If you want to use a different
compiler, for example CLANG, you can either change your system's defaults,
or set explicit arguments to CMAKE, such as:
~~~
   cmake -D CMAKE_C_COMPILER="/usr/bin/clang" -D CMAKE_CXX_COMPILER="/usr/bin/clang++" .
~~~
The exact value of the arguments depend of course of where the compilers
are installed.

# Binary downloads

Binary downloads are supported on selected distributions of Linux.

## Linux CentOS and Fedora

Ithitools can be obtained through the [COPR](https://pagure.io/copr/copr) service on CentOS versions 6 and 7,
and on Fedora versions 26, 27, 28 and Rawhide. The process has three steps:

1) Subscribe to the ithitools project:
```
dnf copr enable chuitema/ithitools
```
2) Install ithitools:
```
dnf install ithitools
```
3) Once the package is installed, Linux will occasionally prompt for updates. This
should be automatic, but just in case the command will be:
```
dnf update ithitools
```
On older systems, the "dnf" command is not supported -- use "yum" instead. These are
system commands, so you may need to use "sudo" as appropriate.

## Ubuntu

Ithitools can be obtained through the [Launchpad](https://launchpad.net/) service for recent Ubuntu builds.
The process has two steps:

1) Subscribe to the ithitools project and get it:
```
sudo add-apt-repository ppa:chuitema/ppa
sudo apt-get update
```
2) Updates should arrive as part of the regular updates for Ubuntu packages.

## Other distributions

Sorry, but for the other distributions you will have to clone the sources from Github and compile.