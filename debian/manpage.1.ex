.\" (C) Copyright 2018 Christian Huitema <huitema@huitema.net>,
.TH Ithitools 7 "May 23 2018"
.\" Please adjust this date whenever revising the manpage.
.SH NAME
ITHITOOLS -- a tool for ITHI data extraction and metric computation.
.SH SYNOPSIS
.B ithitools
.RI [ options ] -c " files.PCAP " ...
.br
.B ithitools
.RI [ options ] -s " files.csv" ...
.SH DESCRIPTION
This manual page documents briefly the
.B ithitools
command. TODO: Should also document the libithicap.so plug-in.
.PP
.SH OPTIONS
These programs follow the simplified command line syntax, with
single character options starting with one dashes (`-').
A summary of options is included below.
For a complete description, see the Info files.
.TP
.B \-? \-h
Show summary of options
.TP
.B \-c
process DNS traffic capture files in PCAP format,
PCAP files listed the input files arguments.
.TP
.B \-s
process summary files, from previous captures.
CSV files listed the input files arguments,
or in a text file, see -S argument description.
.TP
Options used in capture mode:
.TP
.B \-o file.csv
output file containing the computed summary.
.TP
.B \-R root-addr.txt
text file containing the list of root server addresses.
.TP
.B \-a res-addr.txt
allowed list of resolver addresses. Traffic to or from
addresses in this list will not be filtered out by the
excessive traffic filtering mechanism.
.TP
.B \-x res-addr.txt
excluded list of resolver addresses. Traffic to or from
these addresses will be ignored when extracting traffic.
.TP
.B \-f
Filter out address sources that generate too much traffic.
.TP
.B \-T
Capture a list of TLD found in user queries.
.TP
.B \-t tld-file.txt
Text file containing a list of registered TLD, one per line.
.TP
.B \-u tld-file.txt
Text file containing special usage TLD (RFC6761).
.TP
.B \-n number
Number of strings in the list of leaking domains(M4).
.TP
Options used in summary mode:
.TP
.B \-o file.csv
output file containing the computed summary.
.TP
.B \-S filelist.txt
process summary files listed in file list.
.SH SEE ALSO
.BR dnscap (1)
.br

