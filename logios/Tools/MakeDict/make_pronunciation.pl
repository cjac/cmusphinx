#!E/Perl/bin/perl -w
# create a dictionary from a word list
# [20080227] (air)

# dir paths all need to be fully resolved; files are not

use strict;
use File::Spec;
# use Pronounce;  # load dynamically (see below)
use Getopt::Long;

my $usage = "usage: $0 -tools <dir> -resources <dir> -words <file> -handdict <file> -dict <file>\n";

my $toolsdir = "";
my $resources = "";

my $words = "";  # input word list file
my $handdict = ""; # hand edited pronunciations
my $dict = "";   # output pronunciations file

if (scalar @ARGV lt 10) { die "$usage\n"; }
GetOptions( "words:s", \$words,
	    "dict:s", \$dict,
	    "handdict:s", \$handdict,
	    "tools:s",\$toolsdir,
	    "resources:s",\$resources,
	  );
if (scalar @ARGV gt 0) { die "$usage\n"; }

my $lib = File::Spec->catfile($toolsdir,'MakeDict','lib','Pronounce.pm');
require $lib;
&Pronounce::make_dict($toolsdir,$resources,$words,$handdict,$dict,"pronunciation.log");

