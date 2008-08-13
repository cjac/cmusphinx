package Logios;
# perl module for creating an n-gram language model and a dict from a Phoenix grammar,
# using a stochastically generated sentence corpus.
# [20070405] (tkharris) Created.

# [20070925] (air) modified to pass class markers through
# [20070926] (air) fixed flat and vocab routines to handle class markers and probs
# [20070927] (air) added additional class file processing

# [20071011] (air) refactored to separate functional units and to generalize
# [20080812] (tkharris) refactored into perl module

use LWP::UserAgent;
use HTTP::Request::Common;
use File::Spec;
use File::Copy;
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

sub new {
  my $class = shift;
  my %params = @_;

  my $objref = {#run in . by default
                'RESOURCES' => File::Spec->rel2abs($params{'RESOURCES'} || File::Spec->curdir),
                # where to get pronunciation information; could also use lmtool from web
                'SOURCE' => $params{'SOURCE'} || 'local',
                #root of the Logios tools
                'LOGIOS' => File::Spec->rel2abs($params{'LOGIOS'}),
                #use Olympus folder-tree structure?
                'OLYMODE' => $params{'OLYMODE'},
                #if not, where to find stuff
                'INPATH' => $params{'INPATH'},
                #... where to put it
                'OUTPATH' => $params{'OUTPATH'},
                #size of the synthetic corpus
                'SAMPSIZE' => $params{'SAMPSIZE'} || 30000,
                #name of the project (domain, really) we're in
                'PROJECT' => $params{'PROJECT'},
                #name of the particular language we're building here
                'INSTANCE' => $params{'INSTANCE'},
                #name of the log file
                'LOGFILE' => $params{'LOGFILE'} || 'make_language.log'};

  die "Need to know the LOGIOS root." if !defined $objref->{'LOGIOS'};
  # can't do this earlier since we don't know where to look
  require File::Spec->catfile($objref->{'LOGIOS'},'Tools','lib','LogiosLog.pm');
  LogiosLog::open_logfile($objref->{'OLYMODE'} ?
                          File::Spec->catfile($objref->{'RESOURCES'},$objref->{'LOGFILE'}) :
                          File::Spec->catfile($objref->{'OUTPATH'},$objref->{'LOGFILE'}));

  LogiosLog::fail("You must specify both the PROJECT and the INSTANCE for this language!")
      if !$objref->{'PROJECT'} or !$objref->{'INSTANCE'};
  # don't overwrite!
  #LogiosLog::fail("PROJECT and INSTANCE have to be different!") 
  #    if $objref->{'PROJECT'} eq $objref->{'INSTANCE'};

  LogiosLog::fail("Need to know either OLYMODE or the INPATH and OUTPATH.")
      if !$objref->{'OLYMODE'} && !($objref->{'INPATH'} && $objref->{'OUTPATH'});

  # done with the first set of preliminaries
  for my $param (keys %$objref) {
    &LogiosLog::say('Logios', "\t$param => $objref->{$param}");
  }
  &LogiosLog::say('Logios', $/);

  # make temporary folder(s) for holding intermediate results
  $objref->{'DECODERCONFIG'} = $objref->{'OLYMODE'} ? 
    File::Spec->catdir($objref->{'RESOURCES'}, 'DecoderConfig') : $objref->{'OUTPATH'};
  $objref->{'LMDIR'} = $objref->{'OLYMODE'} ? 
    File::Spec->catfile($objref->{'DECODERCONFIG'}, 'LanguageModel') : $objref->{'OUTPATH'};
  $objref->{'LMTEMP'} = File::Spec->catdir($objref->{'LMDIR'},'TEMP');
  mkdir $objref->{'LMTEMP'} if not -e $objref->{'LMTEMP'};

  $objref->{'TOOLS'} = File::Spec->catdir($objref->{'LOGIOS'},'Tools');
  $objref->{'MAKEGRA'} = File::Spec->catdir($objref->{'TOOLS'},'MakeGra');
  $objref->{'GRAMMAR'} = $objref->{'OLYMODE'} ? 
    File::Spec->catdir($objref->{'RESOURCES'}, 'Grammar/GRAMMAR') : $objref->{'INPATH'};
  $objref->{'OUTGRAM'} = $objref->{'OLYMODE'} ? 
    File::Spec->catdir($objref->{'RESOURCES'}, 'Grammar') : $objref->{'OUTPATH'};
  $objref->{'GRAMMARFILE'} = File::Spec->catfile($objref->{'OUTGRAM'}, $objref->{'INSTANCE'}.'.gra');
  $objref->{'BASEDIC'} = File::Spec->catfile($objref->{'OUTGRAM'}, 'base.dic');
  $objref->{'TOKENLIST'} = File::Spec->catfile($objref->{'OUTGRAM'}, $objref->{'INSTANCE'}.'.token');

  $objref->{'FLATGRAMMARFILE'} =
    File::Spec->catfile($objref->{'OUTGRAM'}, $objref->{'INSTANCE'}.'_flat.gra');
  $objref->{'GRABSFILE'} = 
    File::Spec->catfile($objref->{'OUTGRAM'}, $objref->{'INSTANCE'}.'_abs.gra');
  $objref->{'CORPUSFILE'} = 
    File::Spec->catfile($objref->{'LMTEMP'}, $objref->{'INSTANCE'}.'.corpus');

  # words for lm
  $objref->{'ABSDIC'} = File::Spec->catfile($objref->{'LMTEMP'}, $objref->{'INSTANCE'}.'.words');
  # words for dic
  $objref->{'TOKENS'} = File::Spec->catfile($objref->{'OUTGRAM'}, $objref->{'INSTANCE'}.'.token');
  # where final dict goes
  $objref->{'DICTDIR'} = $objref->{'OLYMODE'} ? 
    File::Spec->catdir($objref->{'DECODERCONFIG'}, 'Dictionary') : $objref->{'OUTPATH'};

  bless $objref, $class;
}

# compile Domain GRAMMAR into Project grammar, in Phoenix and corpus versions
sub compile_grammar {
  my $self = shift;

  &LogiosLog::say('Logios', 'COMPILING GRAMMAR...');
  # need to be there for benefit of Phoenix
  my $homedir = Cwd::cwd(); chdir($self->{'OUTGRAM'});
  &LogiosLog::fail("compile_gra.pl") if
    system("perl ".File::Spec->catfile($self->{'MAKEGRA'},"compile_gra.pl")
           ." --tools $self->{'TOOLS'}"
           ." --project $self->{'PROJECT'} --instance $self->{'INSTANCE'} "
           ." --inpath $self->{'GRAMMAR'} --outpath $self->{'OUTGRAM'}  "
           # ." --class "
          );
  # the following files will have been created inside compile_gra.pl:
  #  .ctl and .prodef class files for decoder; .token for pronunciation; .words for lm
  # move some over to LM space
  move("$self->{'INSTANCE'}.ctl", $self->{'LMDIR'});
  move("$self->{'INSTANCE'}.probdef", $self->{'LMDIR'});
  move("$self->{'INSTANCE'}.words",$self->{'LMTEMP'});
  move("$self->{'INSTANCE'}.token",$self->{'DICTDIR'}); # make word tokens available for MakeDict

  chdir($homedir); system('chdir');  # return to wherever we started
}

# LANGUAGE MODEL
sub makelm {
  my $self = shift;

  my $MAKELM = File::Spec->catdir($self->{'TOOLS'},'MakeLM');
  my $TEXT2IDNGRAM = File::Spec->catfile($MAKELM, $bindir, 'text2idngram'.$exten);
  my $IDNGRAM2LM = File::Spec->catfile($MAKELM, $bindir , 'idngram2lm'.$exten);
  my $RANDOMSAMPS = File::Spec->catfile($MAKELM,'generate_random_samples.pl');

  my $IDNGRAM =  File::Spec->catfile($self->{'LMTEMP'},$self->{'INSTANCE'}.'.idngram');
  my $CCS = File::Spec->catfile($self->{'LMTEMP'},"$self->{'INSTANCE'}.ccs");
  my $VOCAB = File::Spec->catfile($self->{'LMTEMP'},'vocab');
  my $LM = File::Spec->catfile($self->{'LMDIR'}, $self->{'INSTANCE'}.'.arpa');

  &LogiosLog::say('Logios', 'COMPILING LANGUAGE MODEL...');
  &LogiosLog::say('Logios', 'generating corpus...');
  $self->get_corpus($RANDOMSAMPS, $self->{'GRABSFILE'},$self->{'CORPUSFILE'});
  &LogiosLog::fail("Logios.pl: Corpus generation failed!\n")
    if -z $self->{'CORPUSFILE'};

  &LogiosLog::say('Logios', 'getting vocabulary...');
  &get_vocab($self->{'ABSDIC'}, $VOCAB, $CCS);

  &LogiosLog::say('Logios', 'computing ngrams...');
  my $cmd = "$TEXT2IDNGRAM -vocab $VOCAB -temp $self->{'LMTEMP'} -write_ascii "
    ."< $self->{'CORPUSFILE'} > $IDNGRAM";
  &LogiosLog::say('Logios', $cmd);
  &LogiosLog::fail("text2idngram failed") if system($cmd);
  &LogiosLog::say('Logios', 'computing language model...');
  $cmd = "$IDNGRAM2LM -idngram $IDNGRAM -vocab $VOCAB -arpa $LM -context $CCS -vocab_type 0"
    ." -good_turing -disc_ranges 0 0 0 -ascii_input";
  &LogiosLog::say('Logios', "$cmd");
  &LogiosLog::fail("idngram2lm failed") if system($cmd);
}

# DICTIONARY
sub makedict {
  my $self = shift;

  &LogiosLog::say('Logios', 'COMPILING DICTIONARY...');
  require File::Spec->catfile($self->{'TOOLS'}, 'MakeDict', 'lib', 'Pronounce.pm');
  my $pronounce = Pronounce->new('TOOLS' => $self->{'TOOLS'},
                                 'DICTDIR' => $self->{'DICTDIR'},
                                 'VOCFN' => "$self->{'INSTANCE'}.token",
                                 'HANDICFN' => 'hand.dict',
                                 'OUTFN' => "$self->{'INSTANCE'}.dict",
                                 'LOGFN' => 'pronunciation.log');
  $pronounce->do_pronounce;
}

sub DESTROY {
  my $self = shift;

  &LogiosLog::say('Logios', "  -----------------  done   ---------------------------------- \n");
}

# generate a sentence corpus from the grammar
sub get_corpus {
  my $self = shift;

  my $RANDOMSAMPS = shift;
  my $grabsfile = shift || $self->{'GRABSFILE'};
  my $corpusfile = shift || $self->{'CORPUSFILE'};

  # flatten out the Kleene stars
  open(GRABS, $grabsfile) || &LogiosLog::fail("get_corpus(): Can't open $grabsfile file");
  open(GFLAT, ">$self->{'FLATGRAMMARFILE'}") || 
    &LogiosLog::fail("Can't open grammar flat file");
  print GFLAT &flatten_gra(<GRABS>);  #
  close GRABS;
  close GFLAT;

  # generate corpus
  my $rs_cmd = "$RANDOMSAMPS -n $self->{'SAMPSIZE'} -d $self->{'OUTGRAM'}"
    ." -grammarfile $self->{'FLATGRAMMARFILE'}";
  &LogiosLog::say('Logios', $rs_cmd);
  open(RANDOM, "perl $rs_cmd|") ||
    &LogiosLog::fail("Cannot execute $rs_cmd");
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

1;
