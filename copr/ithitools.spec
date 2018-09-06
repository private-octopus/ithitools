Summary: DNS capture tools for the ITHI metrics defined by ICANN
Name: ithitools
Version: 1.03
Release: 1

License: MIT
URL: https://github.com/private-octopus/ithitools
Source0: https://github.com/private-octopus/ithitools/archive/master.tar.gz#/%{name}-%{version}-%{release}.tar.gz

BuildRequires: cmake
BuildRequires: gcc
BuildRequires: gcc-c++

%description
The ITHITOOLS program include a set of tools designed to compute some of
"Identifier Technology Health Indicators" (ITHI) metrics defined by ICANN.
The identifiers include Names, Numbers, and Protocol Parameters. A general
presentation of the ITHI work is available here: https://www.icann.org/ithi.
The capture part of the tools are focused on a subset of the ITHI problem,
specifically the computation of the IHTI metrics M3, M4, M6 and M7.


%prep
%autosetup -n %{name}-master

%build
cmake .
make

%install
make install DESTDIR=$RPM_BUILD_ROOT

%files
/usr/local/bin/ithitools
/usr/local/bin/ithitools-1.03
/usr/local/include/ithicap.h
/usr/local/lib/libithicap.so
/usr/local/lib/libithicap.so.1.03

%changelog
* Wed Sep 5 2018 Christian Huitema <huitema@huitema.net> 1.03-2
- Fix warnings and update CMakefile for version 1.03
* Sat Sep 1 2018 Christian Huitema <huitema@huitema.net> 1.03-1
- Add revised metric M8 and new definition of M3, M4
* Sat Aug 4 2018 Christian Huitema <huitema@huitema.net> 1.02-4
- Add metric M8 and add -g flag to CXX compile options
* Sat Aug 4 2018 Christian Huitema <huitema@huitema.net> 1.02-3
- Add metric M5 plus bug fixes
* Fri Jun 29 2018 Christian Huitema <huitema@huitema.net> 1.02-2
- Add metric M5 plus bug fixes
* Tue Jun 12 2018 Christian Huitema <huitema@huitema.net> 1.02-1
- First ithitools package

