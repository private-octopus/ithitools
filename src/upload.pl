#!/usr/bin/perl
use CGI;
$user = $ENV{"REMOTE_USER"};
if ($user eq "whois") {
$upload_dir = "/home/ubuntu/upload/m1/";
} elsif ($user eq "daar") {
$upload_dir = "/home/ubuntu/upload/m2/";
} else {
$upload_dir = "/home/ubuntu/upload/tmp/";
}
$query = new CGI;
$filename = $query->param("whois");
$filename =~ s/.*[\/\\](.*)/$1/;
$upload_filehandle = $query->upload("whois");

open UPLOADFILE, ">$upload_dir/$filename";
while ( <$upload_filehandle> )
{
print UPLOADFILE;
}
close UPLOADFILE;

print $query->header ();

print <<END_HTML;
<HTML>
<HEAD>
<TITLE>Thanks!</TITLE>
</HEAD>
<BODY>
<P>Thanks for uploading $filename to $upload_dir</P>
</BODY>
</HTML>
END_HTML
