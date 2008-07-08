#!/usr/bin/perl
# script to create a n-gram language model and a dict from a Phoenix grammar,
# using a stochastically generated sentence corpus.
# [20070405] (tkharris) Created.

# [20070925] (air) modified to pass class markers through
# [20070926] (air) fixed flat and vocab routines to handle class markers and probs
# [20070927] (air) added additional class file processing

# [20071011] (air) refactored to separate functional units and to generalize


use LWP::UserAgent;
use HTTP::Request::Common;
use File::Spec;
use File::Copy;
use Getopt::Long;
use Cwd;
$ENV{'LC_COLLATE'} = 'C';
$ENV{'LC_ALL'} = 'C';
use locale;
use strict;

my ($bindir, $exten);
if ( ($^O =~ /win32/i) or ($^O =~ /cygwin/) ) {
    $bindir = File::Spec->catdir("bin","x86-nt");
    $exten = ".exe";
  } else {   # otherwise assume we're on linux
    $bindir = File::Spec->catdir("bin","x86-linux");
    $exten = "";
  }

#setup default variables
my $LOGIOS = "";  # root of the Logios tools
my $OLYMODE = 0;  # use Olympus folder-tree structure?
my $INPATH = "";  # if not, where to find stuff
my $OUTPATH= "";  # ... where to put it

my $RESOURCES = File::Spec->rel2abs(File::Spec->curdir);  # run in . by default
my $SOURCE = 'local';  # where to get pronunciation information; could also use lmtool from web
my $SAMPSIZE = 30000;  # size of synthetic corpus
my $LOGFILE = "make_language.log";  # name of log file
my $PROJECT = "";  # name of the project (domain, really) we're in.
my $INSTANCE = "";  # name of the particular language we're building here

sub usage {
  return "usage: $0 [--resources <abspath>] [--samplesize <n>] [--source {local|web}]"
                   ." --project <dname> --instance <pname> [--logfile <fname>] "
		   ." --logios <abspath> --olympus$/";
}


#process command line
if ( not GetOptions(
	   "source=s", \$SOURCE,
	   "logios=s", \$LOGIOS,

	   "olympus!", \$OLYMODE,
	   "resources=s", \$RESOURCES,  # needs to be an abs path to the root
	   "inpath=s", \$INPATH,
	   "outpath=s", \$OUTPATH,

	   "samplesize=s", \$SAMPSIZE,
	   "project=s", \$PROJECT,
	   "instance=s", \$INSTANCE,

	   "logfile=s", \$LOGFILE,
	  ) )
  { die &usage; }

# can't do this earlier since we don't know where to look
require File::Spec->catfile($LOGIOS,'Tools','lib','LogiosLog.pm');
LogiosLog::open_logfile($OLYMODE ?
			File::Spec->catfile($RESOURCES,$LOGFILE) :
			File::Spec->catfile($OUTPATH,$LOGFILE));

if (($PROJECT eq "") or ($INSTANCE eq "")) {
  LogiosLog::fail("You must specify both the PROJECT and the INSTANCE for this language!") }
if ($PROJECT eq $INSTANCE) { LogiosLog::fail("PROJECT and INSTANCE have to be different!") } # don't overwrite!

# done with the first set of preliminaries
&LogiosLog::say( "\nMAKE_LANGUAGE",
      "$/"
     ."\ttools => $LOGIOS$/"
     ."\tresources => $RESOURCES$/"
     ."\tproject => $PROJECT$/"
     ."\tinstance => $INSTANCE$/"
     ."\tsample size => $SAMPSIZE$/"
     ."\tsource => $SOURCE$/"
     ."\tolympus => $OLYMODE$/"
    );
if (not $OLYMODE) { &LogiosLog::say("\tinpath => $INPATH$/\toutpath =>$/"); }



# make temporary folder(s) for holding intermediate results
my $DECODERCONFIG = $OLYMODE ? File::Spec->catdir($RESOURCES, 'DecoderConfig') : $OUTPATH;
my $LMDIR = $OLYMODE ? File::Spec->catfile($DECODERCONFIG, 'LanguageModel') : $OUTPATH;
my $LMTEMP = File::Spec->catdir($LMDIR,'TEMP');
if ( not -e $LMTEMP ) { mkdir($LMTEMP); }

my $TOOLS = File::Spec->catdir($LOGIOS,'Tools');
my $MAKEGRA = File::Spec->catdir($TOOLS,'MakeGra');
my $GRAMMAR = $OLYMODE ? File::Spec->catdir($RESOURCES, 'Grammar/GRAMMAR') : $INPATH;
my $OUTGRAM = $OLYMODE ? File::Spec->catdir($RESOURCES, 'Grammar') : $OUTPATH;
my $GRAMMARFILE = File::Spec->catfile($OUTGRAM, $INSTANCE.'.gra');
my $BASEDIC = File::Spec->catfile($OUTGRAM, 'base.dic');
my $TOKENLIST = File::Spec->catfile($OUTGRAM, $INSTANCE.'.token');







my $FLATGRAMMARFILE = File::Spec->catfile($OUTGRAM, $INSTANCE.'_flat.gra');
my $GRABSFILE = File::Spec->catfile($OUTGRAM, $INSTANCE.'_abs.gra');
my $CORPUSFILE = File::Spec->catfile($LMTEMP, $INSTANCE.'.corpus');

my $ABSDIC = File::Spec->catfile($LMTEMP, $INSTANCE.'.words');  # words for lm
my $TOKENS = File::Spec->catfile($OUTGRAM, $INSTANCE.'.token');  # words for dic
my $DICTDIR = $OLYMODE ? File::Spec->catdir($DECODERCONFIG, 'Dictionary') : $OUTPATH;  # where final dict goes


# compile Domain GRAMMAR into Project grammar, in Phoenix and corpus versions
&LogiosLog::say("\nmake_language", 'COMPILING GRAMMAR...');
my $homedir = Cwd::cwd(); chdir($OUTGRAM); system('chdir');  # need to be there for benefit of Phoenix
&LogiosLog::fail("compile_gra.pl") if
  system("perl ".File::Spec->catfile($MAKEGRA,"compile_gra.pl")
	 ." --tools $TOOLS"
	 ." --project $PROJECT -instance $INSTANCE "
	 ." --inpath $GRAMMAR --outpath $OUTGRAM  "
	 # ." --class "
      );
# the following files will have been created inside compile_gra.pl:
#  .ctl and .prodef class files for decoder; .token for pronunciation; .words for lm
# move some over to LM space
move("$INSTANCE.ctl", $LMDIR);
move("$INSTANCE.probdef", $LMDIR);
move("$INSTANCE.words",$LMTEMP);
move("$INSTANCE.token",$DICTDIR); # make word tokens available for MakeDict

chdir($homedir); system('chdir');  # return to wherever we started


# LANGUAGE MODEL
my $MAKELM = File::Spec->catdir($TOOLS,'MakeLM');
my $TEXT2IDNGRAM = File::Spec->catfile($MAKELM, $bindir, 'text2idngram'.$exten);
my $IDNGRAM2LM = File::Spec->catfile($MAKELM, $bindir , 'idngram2lm'.$exten);
my $RANDOMSAMPS = File::Spec->catfile($MAKELM,'generate_random_samples.pl');

my $IDNGRAM =  File::Spec->catfile($LMTEMP,$INSTANCE.'.idngram');
my $CCS = File::Spec->catfile($LMTEMP,"$INSTANCE.ccs");
my $VOCAB = File::Spec->catfile($LMTEMP,'vocab');
my $LM = File::Spec->catfile($LMDIR, $INSTANCE.'.arpa');

&LogiosLog::say("\nmake_language", 'COMPILING LANGUAGE MODEL...');
&LogiosLog::say('make_language', 'generating corpus...');
&get_corpus($GRABSFILE,$CORPUSFILE);
if ( -z $CORPUSFILE ) { &LogiosLog::fail("make_language.pl: Corpus generation failed!\n"); }

&LogiosLog::say('make_language', 'getting vocabulary...');
&get_vocab($ABSDIC, $VOCAB, $CCS);

&LogiosLog::say('make_language', 'computing ngrams...');
my $cmd = "$TEXT2IDNGRAM -vocab $VOCAB -temp $LMTEMP -write_ascii < $CORPUSFILE > $IDNGRAM";
&LogiosLog::say('make_language', $cmd);
&LogiosLog::fail("text2idngram failed") if system($cmd);
&LogiosLog::say('make_language', 'computing language model...');
$cmd = "$IDNGRAM2LM -idngram $IDNGRAM -vocab $VOCAB -arpa $LM -context $CCS -vocab_type 0"
                 ." -good_turing -disc_ranges 0 0 0 -ascii_input";
&LogiosLog::say('make_language', "$cmd$/");
&LogiosLog::fail("idngram2lm failed") if system($cmd);

# DICTIONARY
my $MAKEDICT = File::Spec->catfile($TOOLS,'MakeDict','make_pronunciation.pl');
my $WORDLIST = File::Spec->catfile($INSTANCE,'.token');
my $HAND_DICT = 'hand.dict';
my $DICT = File::Spec->catfile($INSTANCE.'.dict');

&LogiosLog::say("\nmake_language", 'COMPILING DICTIONARY...');
system("perl $MAKEDICT \
       -tools $TOOLS -dictdir $DICTDIR -words $INSTANCE.token -handdict $HAND_DICT -dict $DICT");





system('chdir');
&LogiosLog::say("$/MAKE_LANGUAGE", "  -----------------  done   ---------------------------------- \n");


exit;
##############################################



# generate a sentence corpus from the grammar
sub get_corpus {
  my $grabsfile = shift;
  my $corpusfile = shift;

  # flatten out the Kleene stars
  open(GRABS, $grabsfile) || &LogiosLog::fail("get_corpus(): Can't open $grabsfile file");
  open(GFLAT, ">$FLATGRAMMARFILE") || &LogiosLog::fail("Can't open grammar flat file");
  print GFLAT &flatten_gra(<GRABS>);  #
  close GRABS;
  close GFLAT;

  # generate corpus
  &LogiosLog::say('get_corpus()', "$RANDOMSAMPS -n $SAMPSIZE -d $GRAMMAR");
  open(RANDOM,
       "perl $RANDOMSAMPS -n $SAMPSIZE -d $OUTGRAM -grammarfile $FLATGRAMMARFILE |") ||
	 &LogiosLog::fail("Cannot execute $RANDOMSAMPS");
  open(CORPUS, ">$corpusfile") || &LogiosLog::fail("Can't open $corpusfile");

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



####################################################
# create a list of lexical tokens for language model
sub get_vocab {
    my $basefile = shift;
    my $vocab = shift;
    my $ccs = shift;
    my @norm = ();
    open(VOCAB, ">$vocab") || &LogiosLog::fail("get_vocab(): can't open vocab @ $vocab");
    binmode VOCAB;
    open(BASE, "<$basefile") || &LogiosLog::fail("get_vocab(): can't open basefile @ $basefile");
    my @base = <BASE>;  # dic now comes pre-processed from tokenize.pl
    close BASE;

    print VOCAB grep !/<\/?S>/, sort(@base);
    print VOCAB "<s>\n";
    print VOCAB "</s>\n";
    close VOCAB;

    open(CCS, ">$ccs") || &LogiosLog::fail("get_vocab(): can't open $ccs");
    binmode CCS;
    print CCS "<s>\n";
    close CCS;
}


#

