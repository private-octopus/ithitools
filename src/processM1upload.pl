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
my $ff1;
my $ff2;
my $fy;
my $fy4;
my $fm;
my $fmi;
my $fd;
my $target;
my $moved;
my @md = ( 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 );
my @m2 = ( "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12");

printf "Nb arg: $#ARGV\n";

if ($#ARGV != 1) {
    print "\nUsage: processM1upload.pl upload_dir target_dir\n";
    exit;
}

$dirname = $ARGV[0];
$targetdir = $ARGV[1];
$processdir = "$dirname/../processed";

print "Dir: $dirname, Target: $targetdir\n";

opendir(DIR, $dirname) or die "Could not open $dirname\n";

while ($filename = readdir(DIR)) {
  if  (length($filename) == 51) {
      $ff1 = substr($filename, 0, 25);
      $fm = substr($filename, 25, 3);
      $fy = substr($filename, 28, 4);
      $ff2 = substr($filename, 32, 19);
      if ($ff1 eq "Compliance Data for ITHI_" &&
          $ff2 eq " Rr_Ry_Tab3_CSV.csv") {
          $fmi = index("JanFebMarAprMayJunJulAugSepOctNovDec", $fm);
          if ($fmi >= 0) {
              $fmi /= 3;
              $fy4 = $fy % 4;
              if ($fy4 == 0 && $fmi == 1){
                  $fd = 29;
              } else {
                  $fd = $md[$fmi];
              }
              $target = "$targetdir/M1-$fy-$m2[$fmi]-$fd-compliance.csv";
              $filepath="$dirname/$filename";
              copy($filepath, $target) or die "Cannot copy $filename to $target\n";
              print "Copied $filename to $target\n";
              $moved = "$processdir/$filename";
              move($filepath, $moved) or die "Cannot move $filename to $moved\n";
              printf "Moved $filename to $processdir\n";
          }
      }
  }
}

closedir(DIR);
