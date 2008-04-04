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
# use IO::Handle;
# use IPC::Open2;
use Getopt::Long;
$ENV{'LC_COLLATE'} = 'C';
$ENV{'LC_ALL'} = 'C';
use locale;
use strict;


#setup default variables
my $RESOURCES = File::Spec->rel2abs(File::Spec->curdir);  # MUST be running in Resources
my $SOURCE = 'local';  # where to get pronunciation information; could also use lmtool from web
my $SAMPSIZE = 30000;  # size of synthetic corpus
my $LOGFILE = "";  # name of log file
my $PROJECT = "";  # name of the project (domain, really) we're in.
my $INSTANCE = "";  # name of the particular language we're building here

sub usage {
  return "usage: $0 [--resources <abspath>] [--samplesize <n>] [--source {local|web}]"
                   ." --project <dname> --instance <pname> [--logfile]$/";
}
#process command line
GetOptions("resources=s", \$RESOURCES,  # needs to be an abs path
	   "samplesize=s", \$SAMPSIZE,
	   "source=s", \$SOURCE,
	   "project=s", \$PROJECT,
	   "instance=s", \$INSTANCE,
	   "logfile=s", \$LOGFILE,
	  );
fail(&usage) if @ARGV;
fail("You must specify both the PROJECT and the INSTANCE for this language!")
  if ($PROJECT eq "") or ($INSTANCE eq "");
fail("PROJECT and INSTANCE have to be different!")
  if $PROJECT eq $INSTANCE;  # don't overwrite!

# create some useful paths and files
chdir($RESOURCES); system('chdir');
my $PROJECTROOT = File::Spec->rel2abs(File::Spec->updir());
my $TOOLS = File::Spec->catdir($PROJECTROOT,'Tools');

open(LOG, ">$LOGFILE") if $LOGFILE ne "";  #open log file

# done with the first set of preliminaries
&say('make_language', "$/project => $PROJECT$/resource dir => $RESOURCES$/sample"
     ."size => $SAMPSIZE$/source => $SOURCE$/instance => $INSTANCE$/");


# compile Domain grammar into Project grammar, in Phoenix and corpus versions
my $MAKEGRA = File::Spec->catdir($TOOLS,'MakeGra');
my $GRAMMAR = File::Spec->catdir($RESOURCES, 'Grammar');
my $PHOENIX = File::Spec->catfile($TOOLS, 'MakeGra','bin','compile.exe');
my $GRAMMARFILE = File::Spec->catfile($GRAMMAR, $INSTANCE.'.gra');
my $BASEDIC = File::Spec->catfile($GRAMMAR, 'base.dic');
my $TOKENLIST = File::Spec->catfile($GRAMMAR, $INSTANCE.'.token');

my $FLATGRAMMARFILE = File::Spec->catfile($GRAMMAR, $INSTANCE.'_flat.gra');
my $GRABSFILE = File::Spec->catfile($GRAMMAR, $INSTANCE.'_abs.gra');
my $CORPUSFILE = File::Spec->catfile($GRAMMAR, $INSTANCE.'.corpus');

my $DECODERCONFIG = File::Spec->catdir($RESOURCES, 'DecoderConfig');

my $ABSDIC = File::Spec->catfile($GRAMMAR, $INSTANCE.'.words');  # words for lm
my $TOKENS = File::Spec->catfile($GRAMMAR, $INSTANCE.'.token');  # words for dic
my $DICTDIR = File::Spec->catdir($DECODERCONFIG, 'Dictionary');  # where final dict goes

&say('make_language', 'compiling grammar...');
chdir($GRAMMAR); system('chdir');
&say(" > executing in: ",File::Spec->rel2abs(File::Spec->curdir));
&fail("compile_gra.pl") if
  system("perl ".File::Spec->catfile($MAKEGRA,"compile_gra.pl").
	 " -p $PROJECT -instance $INSTANCE -class ".
	 "-ingra $PROJECT.gra -outgra $INSTANCE.gra -absgra $GRABSFILE"
      );
# the following files should have been created inside compile_gra.pl:
#  .ctl and .prodef class files for decoder; .token for pronunciation; .words for lm

# move some of these over to lm space
my $LMDIR = File::Spec->catfile($DECODERCONFIG, 'LanguageModel');
my $LMTEMP = File::Spec->catdir($LMDIR,'TEMP');
copy("$INSTANCE.ctl", $LMDIR);
copy("$INSTANCE.probdef", $LMDIR);

# make word tokens available for MakeDict
copy("$INSTANCE.token",$DICTDIR);

# language model
my $MAKELM = File::Spec->catdir($TOOLS,'MakeLM');
my $TEXT2IDNGRAM = File::Spec->catfile($MAKELM,'cmucuslmtk', 'bin', 'text2idngram');
my $IDNGRAM2LM = File::Spec->catfile($MAKELM,'cmucuslmtk', 'bin', 'idngram2lm');
my $RANDOMSAMPS = File::Spec->catfile($MAKELM,'generate_random_samples.pl');
my $IDNGRAM =  File::Spec->catfile($LMTEMP,$INSTANCE.'.idngram');
my $CCS = File::Spec->catfile($LMTEMP,"$INSTANCE.ccs");
my $VOCAB = File::Spec->catfile($LMTEMP,'vocab');
my $LM = File::Spec->catfile($LMDIR, $INSTANCE.'.arpa');

&say('make_language', 'COMPILING LANGUAGE MODEL...');
chdir($MAKELM); system('chdir');
&say('make_language', 'generating corpus...');
&get_corpus($GRABSFILE,$CORPUSFILE);
&say('make_language', 'getting vocabulary...');
&get_vocab($ABSDIC, $VOCAB, $CCS);
&say('make_language', 'computing ngrams...');
my $cmd = "$TEXT2IDNGRAM -vocab $VOCAB -temp $LMTEMP -write_ascii < $CORPUSFILE > $IDNGRAM";
&say('make_language', $cmd);
&fail("text2idngram failed") if system($cmd);
&say('make_language', 'computing language model...');
$cmd = "$IDNGRAM2LM -idngram $IDNGRAM -vocab $VOCAB -arpa $LM -context $CCS -vocab_type 0"
                 ." -good_turing -disc_ranges 0 0 0 -ascii_input";
&say('make_language', "$cmd$/");
&fail("idngram2lm failed") if system($cmd);

#get dictionary
my $MAKEDICT = File::Spec->catfile($TOOLS,'MakeDict','make_pronunciation.pl');
my $WORDLIST = File::Spec->catfile($INSTANCE,'.token');
my $HAND_DICT = 'hand.dict';
my $DICT = File::Spec->catfile($INSTANCE.'.dict');

&say('make_language', 'compiling dictionary...');
chdir($MAKEDICT); system('chdir');
system("perl $MAKEDICT \
       -tools $TOOLS -resources $DICTDIR -words $INSTANCE.token -handdict $HAND_DICT -dict $DICT");
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
  &say('compile', "$RANDOMSAMPS -n $SAMPSIZE -d $GRAMMAR");
  open(RANDOM,
       "perl $RANDOMSAMPS -n $SAMPSIZE -d $GRAMMAR -grammarfile $FLATGRAMMARFILE |") ||
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
    open(VOCAB, ">$vocab") || &fail("get_vocab(): can't open $vocab");
    binmode VOCAB;
    open(BASE, "<$basefile") || &fail("get_vocab(): can't open $basefile");
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


##  utilities  ########################################
sub say {
    my ($system, $txt) = @_;
    print STDERR "$system: $txt$/";
    print LOG "$system: $txt$/" if $LOGFILE;
}

sub fail { my $reason = shift; &say('fail', $reason); die; }


#

