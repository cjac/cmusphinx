#!E/Perl/bin/perl -w
# create a dictionary from a word list
# [20080227] (air)

BEGIN { push @INC, "./lib", "MakeDict/lib"; }
use strict;
use Pronounce;
use File::Spec;

use Getopt::Long;

my $usage = "usage: $0 -tools <dir> -resources <dir> -words <file> -handdict <file> -dict <file>\n";

my $toolsdir = "";  # where to find MakeDict/ tree
my $resources = ""; # where to find word list and hand.dict

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

&Pronounce::make_dict('loc',$toolsdir,$resources,$words,$handdict,$dict,"pronunciation.log");

exit 1;
#
