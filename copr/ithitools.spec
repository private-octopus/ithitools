Summary: DNS capture tools for the ITHI metrics defined by ICANN
Name: ithitools
Version: 1.02
Release: 1

License: MIT
URL: https://github.com/private-octopus/ithitools
Source: {{{ git_pack }}}

BuildRequires: cmake

%description
The ITHITOOLS program include a set of tools designed to compute some of
"Identifier Technology Health Indicators" (ITHI) metrics defined by ICANN.
The identifiers include Names, Numbers, and Protocol Parameters. A general
presentation of the ITHI work is available here: https://www.icann.org/ithi.
The capture part of the tools are focused on a subset of the ITHI problem,
specifically the computation of the IHTI metrics M3, M4, M6 and M7.


%prep
{{{ git_setup_macro }}}

%build
cmake .
make

%install
make install DESTDIR=$RPM_BUILD_ROOT

%changelog
{{{ git_changelog }}}

