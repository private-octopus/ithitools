#!/usr/bin/perl
use strict;
use warnings;
use File::Copy qw(copy);
use File::Copy qw(move);

my $dirname;
my $filename;
my $filepath;
my $targetdir;
my $processdir;
my $target;
my $moved;

printf "Nb arg: $#ARGV\n";

if ($#ARGV != 1) {
    print "\nUsage: processM2upload.pl upload_dir target_dir\n";
    exit;
}

$dirname = $ARGV[0];
$targetdir = $ARGV[1];
$processdir = "$dirname/../processed";

print "Dir: $dirname, Target: $targetdir\n";

opendir(DIR, $dirname) or die "Could not open $dirname\n";

while ($filename = readdir(DIR)) {
    $filepath="$dirname/$filename";
    $target="$targetdir/$filename";
    copy($filepath, $target) or die "Cannot copy $filename to $target\n";
    print "Copied $filename to $target\n";
    $moved = "$processdir/$filename";
    move($filepath, $moved) or die "Cannot move $filename to $moved\n";
    printf "Moved $filename to $processdir\n";
}

closedir(DIR);
