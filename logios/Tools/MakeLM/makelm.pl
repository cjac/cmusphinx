#!/usr/bin/perl
# script to create a n-gram language model and a dict from a Phoenix grammar,
# using a stochastically generated sentence corpus.
# [20070405] (tkharris) Created.

# [20070925] (air) modified to pass class markers through
# [20070926] (air) fixed flat and vocab routines to handle class markers and prob comments
# [20070927] (air) added additional class file processing


BEGIN {
  # check that paths have been set
  use File::Spec;
  if ( not defined $ENV{TOOLS_ROOT} )
    { die "Pronounce.pm: enviromnmental variable TOOLS_ROOT must be defined!\n"; }
  if ( not defined $ENV{RESOURCES_ROOT} )
    { die "Pronounce.pm: enviromnmental variable RESOURCES_SROOT must be defined!\n"; }
  push @INC, File::Spec->catdir($TOOLS_ROOT,"lib");
};


use LWP::UserAgent;
use HTTP::Request::Common;
use File::Copy;
use IO::Handle;
use IPC::Open2;
use Getopt::Long;

use Pronounce;
use LogiosLog;

$ENV{'LC_COLLATE'} = 'C';
$ENV{'LC_ALL'} = 'C';
use locale;
use strict;

#setup default variables
my $MAKELMDIR = File::Spec->rel2abs(File::Spec->curdir); #needs to run from makelm directory
my $RESOURCESDIR = File::Spec->catdir(File::Spec->updir, File::Spec->updir, 'Resources');
#my $SOURCE = 'fife';
my $SOURCE = 'lexdata';
my $SAMPSIZE = 30000;

# default input and ouput language IDs
my $DOMAIN = "TeamTalkTask";
my $PROJECT;  # 'TeamTalk', usually
{
    #guess the project name from the file name of the project directory
    my @dir = File::Spec->splitdir((File::Spec->splitpath($MAKELMDIR))[1]);
    $PROJECT = $dir[$#dir-2];
}

sub usage {
    return "usage: $0 [--resources <dir>] [--samplesize <n>]"
                    ."[--source <pronounce>] [--project <name>]$/";
}
#process command line
GetOptions("resourcesdir=s", \$RESOURCESDIR,
	   "samplesize=s", \$SAMPSIZE,
	   "source=s", \$SOURCE,
	   "projectname=s", \$PROJECT
	  );
fail(&usage) if @ARGV;

#open log file
my $LOGFILE = 'log.txt';
open(LOG, ">$LOGFILE") if $LOGFILE;

#need access to cygwin dir so that cygwin1.dll can be found
$ENV{'Path'} .= ';'.($ENV{'CYGWIN_DIR'} || 'C:\cygwin').'\bin';


&say('system', "project => $PROJECT$/resource dir => $RESOURCESDIR$/sample"
     ."size => $SAMPSIZE$/source => $SOURCE$/");

# various globals...
my $GRAMMARDIR = File::Spec->catdir($RESOURCESDIR, 'Grammar');
my $PHOENIX = File::Spec->catfile($GRAMMARDIR, 'compile.exe');
my $GRAMMARFILE = File::Spec->catfile($GRAMMARDIR, $PROJECT.'.gra');
my $BASEDIC = File::Spec->catfile($GRAMMARDIR, 'base.dic');
my $TOKENLIST = File::Spec->catfile($GRAMMARDIR, $PROJECT.'.token');

my $FLATGRAMMARFILE = File::Spec->catfile($GRAMMARDIR, $PROJECT.'_flat.gra');
my $GRABSFILE = File::Spec->catfile($GRAMMARDIR, $PROJECT.'_abs.gra');
my $CORPUSFILE = File::Spec->catfile($GRAMMARDIR, $PROJECT.'.corpus');

my $VOCAB = 'vocab';
my $ABSDIC = File::Spec->catfile($GRAMMARDIR, $PROJECT.'.words');  # words for lm
my $TOKENS = File::Spec->catfile($GRAMMARDIR, $PROJECT.'.token');  # words for dic
my $CCS = 'temp.ccs';

my $TEXT2IDNGRAM = File::Spec->catfile('CMU-Cam_Toolkit_v2', 'bin', 'text2idngram');
my $IDNGRAM2LM = File::Spec->catfile('CMU-Cam_Toolkit_v2', 'bin', 'idngram2lm');
my $RANDOMSAMPS = 'generate_random_samples.pl';
my $IDNGRAM = $PROJECT.'.idngram';
my $DECODERCONFIGDIR = File::Spec->catdir($RESOURCESDIR, 'DecoderConfig');
my $LMDIR = File::Spec->catfile($DECODERCONFIGDIR, 'LanguageModel');
my $LM = File::Spec->catfile($LMDIR, $PROJECT.'.arpa');

my $PRONOUNCE = 'pronounce';
my $LEXDATA = 'lexdata';
my $DICTDIR = File::Spec->catdir($DECODERCONFIGDIR, 'Dictionary');
my $HAND_DICT = File::Spec->catfile($DICTDIR, 'hand.dict');
my $DICT = File::Spec->catfile($DICTDIR, $PROJECT.'.dict');
my $REDUCED_DICT = $DICT.'.reduced_phoneset';
#


# compile Domain grammar into Project grammar, in Phoenix and corpus versions
&say('compile', 'compiling grammar...');
chdir($GRAMMARDIR); system('chdir');
system("perl compile_gra.pl -domain $DOMAIN -project $PROJECT -class ".
       "-ingra $DOMAIN.gra -absgra $GRABSFILE" )
      || system 'cmp.bat';

# create at .ctl and .prodef file for decoder; .token for pronunciation; .words for lm
chdir ($GRAMMARDIR); system('chdir');
&fail("tokenize failed!") if system("perl tokenize.pl -g $GRABSFILE -p $PROJECT");
copy("$PROJECT.ctl", $LMDIR);
copy("$PROJECT.probdef", $LMDIR);

# language model
&say('compile', 'COMPILING LANGUAGE MODEL...');
chdir($MAKELMDIR); system('chdir');
&say('compile', 'generating corpus...');
&get_corpus($GRABSFILE,$CORPUSFILE);
&say('compile', 'getting vocabulary...');
&get_vocab($ABSDIC, $VOCAB, $CCS);
&say('compile', 'computing ngrams...');
my $cmd = "$TEXT2IDNGRAM -vocab $VOCAB -temp . -write_ascii < $CORPUSFILE > $IDNGRAM";
&say('compile', $cmd);
&fail("text2idngram failed") if system($cmd);
&say('compile', 'computing language model...');
$cmd = "$IDNGRAM2LM -idngram $IDNGRAM -vocab $VOCAB -arpa $LM -context $CCS -vocab_type 0"
                 ." -good_turing -disc_ranges 0 0 0 -ascii_input";
&say('compile', "$cmd$/");
&fail("idngram2lm failed") if system($cmd);

#get dictionary
&say('compile', 'compiling dictionary...');
chdir($MAKELMDIR); system('chdir');
&get_dict($DICT, $REDUCED_DICT, $TOKENS);
&say('compile', "done\n");
close(LOG) if $LOGFILE;

exit;
##############################################


# generate a sentence corpus from the grammar
sub get_corpus {
  my $grabsfile = shift;
  my $corpusfile = shift;

  # flatten out the Kleene stars
  open(TEAMTALKGRA, $grabsfile) || &fail("get_corpus(): Can't open $grabsfile file");
  open(TEAMTALKFLAT, ">$FLATGRAMMARFILE") || &fail("Can't open grammar flat file");
  print TEAMTALKFLAT &flatten_gra(<TEAMTALKGRA>);  #
  close TEAMTALKGRA;
  close TEAMTALKFLAT;

  # generate corpus
  &say('compile', "$RANDOMSAMPS -n $SAMPSIZE -d $GRAMMARDIR");
  open(RANDOM,
       "perl $RANDOMSAMPS -n $SAMPSIZE -d $GRAMMARDIR -grammarfile $FLATGRAMMARFILE |") ||
	 &fail("Cannot execute $RANDOMSAMPS");
  open(CORPUS, ">$corpusfile") || &fail("Can't open $corpusfile");

  # normalize sentences for output
  binmode CORPUS;
  while (<RANDOM>) {
    my @line = ();
    my @words = ();
    chomp;
    #	$_ = uc($_);
    s/<\/?[sS]> //g; # remove any sentence delimiters
    @words = split /\s+/,$_;
    push @line,"<s>";
    foreach my $w (@words) { # do not uppercase protected tokens (%[Foo]%)
      if ( $w =~ /%(\[.+?\])%/ ) { push @line,$1; } else { push @line,uc($w); }
    }
    push @line,"</s>";
    print CORPUS join( " ", @line),"\n";
  }
  close CORPUS;
  close RANDOM;
}


# create a list of lexical tokens for language model
sub get_vocab {
    my $basefile = shift;
    my $vocab = shift;
    my $ccs = shift;
    my @norm = ();
    open(VOCAB, ">$vocab") || &fail("get_vocab(): can't open $vocab");
    binmode VOCAB;
    open(BASE, "<$basefile") || &fail("get_vocab(): can't open $basefile");
 #   my @base = map { /(.*) .*/? "$1" : () } <BASE>; # base.dic file has "token nodeid\n"
    my @base = <BASE>;  # dic now comes pre-processed from tokenize.pl
    close BASE;

    print VOCAB grep !/<\/?S>/, sort(@base);
    print VOCAB "<s>\n";
    print VOCAB "</s>\n";
    close VOCAB;

    open(CCS, ">$ccs") || &fail("get_vocab(): can't open $ccs");
    binmode CCS;
    print CCS "<s>\n";
    close CCS;
}


# create pronouncing dictionary
&Pronounce::get_dict($vocab,$handdict,$dict);


# flatten out all instances of concepts with Kleene stars
# this is presumably because the random sentence generator can't deal with it
# if there's a probability (eg, #%%0.5%%), it's split evenly across lines
sub flatten_gra {
  my @unflat = @_;
  my @result;
  my ($entry, $com, $prob,$newprob);

  for (@unflat) {
    # check for comments on line: pass through as it
    chomp;
    if ( /^[\[#;]/ or /^\s+$/ ) { push @result,"$_\n"; next; } # ignore non-concept lines
    if ( /#/ ) { # is there a comment?
      ($entry,$com) = ( $_ =~ /(.+?)(#.*)/);
      # is the comment a prob?
      if ( $com =~ /%%(\d\.\d+)%%/ ) { $prob = $1; } else { $prob = undef; }
    } else { $entry = $_; $com = ""; $prob = undef; }
    if (! ($entry =~ s/^\s*\((.*)\)/$1/) ) {
      push @result, $entry.$com."\n";
    } else {
      # concept entry: do flattening if needed
      my @stack;
      my %flathash;
      push(@stack, [split /\s+/,$entry]);
      while (my $buffref = shift @stack) {
	my $i = 0;  # index of token within line
	my @buff = @$buffref;
	my $flat;
	for (@buff) {
	  if (/^\*(.*)/) {
	    $flat .= "$1 ";
	    push(@stack, [ @buff[0..$i-1], @buff[$i+1..$#buff] ]);
	  } else {
	    $flat .= "$_ ";
	  }
	  $i++;
	}
	$flathash{$flat} = 1;
      }
      if (defined $prob) {  # distribute prob uniformly over variants
	my $variants = scalar keys %flathash;
	$newprob = sprintf "#%%%%%7.5f%%%%", $prob/$variants;  # dangerous if probs small...
      } else { $newprob = ""; }
      foreach (keys %flathash) {
	push @result, "\t( $_) $newprob\n";
      }
    }
  }
  @result;
}


#
