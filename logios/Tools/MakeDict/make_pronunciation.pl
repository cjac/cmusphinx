#!E/Perl/bin/perl -w
# create a dictionary from a word list
# [20080227] (air)

# dir paths all need to be fully resolved; files do not

use strict;
use File::Spec;
# use Pronounce;  # load dynamically (see below)
use Getopt::Long;

my $usage = "usage: $0 -tools <dir> -dictdir <dir> -words <file> -handdict <file> -dict <file>\n";

my $toolsdir = "";
my $dictdir = "";

my $words = "";  # input word list file
my $handdict = ""; # hand edited pronunciations
my $dict = "";   # output pronunciations file

if (scalar @ARGV lt 10) { die "$usage\n"; }
GetOptions( "words:s", \$words,
	    "dict:s", \$dict,
	    "handdict:s", \$handdict,
	    "tools:s",\$toolsdir,
	    "dictdir:s",\$dictdir,
	  );
if (scalar @ARGV gt 0) { die "$usage\n"; }

my $lib = File::Spec->catfile($toolsdir,'MakeDict','lib','Pronounce.pm');
require $lib;
my $pronounce = Pronounce->new('TOOLS' => $toolsdir,
                               'DICTDIR' => $dictdir,
                               'VOCFN' => $words,
                               'HANDICFN' => $handdict,
                               'OUTFN' => $dict,
                               'LOGFN' => 'pronunciation.log',
			       'SOURCE' => "",
    );
$pronounce->do_pronounce;

