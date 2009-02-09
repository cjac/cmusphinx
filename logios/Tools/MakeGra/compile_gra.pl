#!/usr/local/bin/perl

# compile a grammar into forms and nets files
# produce a "final" version of the grammar (after resolution)
# 'extended' grammar incorporates all class members and is used for parser
# 'abstracted' grammar leaves the classes as stubs and is used by the corpus generator

#
# manage the grammar compilation process, specifically:
# - resolve dynamic classes
# - make abstracted (for parser) and expanded (for lm) grammars
# - create .ctl and .prob files for the language model
#
# *** CALLING SCRIPT NEEDS TO CHDIR TO THE <domain>/Resources/Grammar/ FOLDER

#
# [200710] (air) - based on cmp.pl
# [200801] (air) untangled Tools and Resources, removed all cygwin dependencies
# [20080422] (air) fixed win/linux differentiation
# [20090208] (air) added -pocket flag passthrough to tokenize.pl

use strict;
use Getopt::Long;
use File::Spec;

my ($bindir, $exten);
if ( ($^O =~ /win32/i) or ($^O =~ /cygwin/) ) {$bindir = "bin/x86-nt"; $exten = ".exe"; }
else { $bindir = "bin/x86-linux"; $exten = ""; }  # otherwise assume we're on linux

# all MakeGra executables are supposed to be in a known place (rel to the current script)
my ($drive,$dirs,$exec) = File::Spec->splitpath($0);
my $EXEDIR = File::Spec->catpath($drive,$dirs,"");

# some defaults
my $tools = "";
my $project = "";
my $instance = "";
my @classd = ();
my @classf = ();
my $cf;
my $inpath = "";
my $outpath = "";
my $ingra = "";
my $pocket_flag = "";
my $outgra = "";
my $absgra = "";

if (not GetOptions(
        "tools:s" => \$tools,
        "class" => \@classf,
        "project:s" => \$project,
        "instance:s" => \$instance,
        "inpath:s" => \$inpath,
        "outpath:s" => \$outpath,
	"pocket" => \$pocket_flag,
        ) )
  { die "usage: compile_gra -tools <path> [-class <file>]* [-project <project> -instance <instance> -inpath <dir> -outpath <dir> [-pocket]\n"; }

# can't do this earlier since we don't know where to look
require File::Spec->catfile($tools, 'lib', 'LogiosLog.pm');
LogiosLog::open_logfile(File::Spec->catfile($outpath, 'compile_gra.log'));

my $ingra = File::Spec->catfile($inpath,"$project.gra");
my $outgra = File::Spec->catfile($outpath,"$instance.gra");
my $absgra = File::Spec->catfile($outpath,"${instance}_abs.gra");
LogiosLog::say('compile_gra', "[in ",File::Spec->rel2abs(File::Spec->curdir),"]",
        "$/\tingra->  $ingra",
               "$/\toutgra-> $outgra",
               "$/\tclass->  ",join(" ",@classf));

# see if any ad-hoc class definitions are provided; put copies of the files into GRAMMAR/
foreach $cf (@classf) {
  open(IN,$cf) or die "can't open class file: $cf\n";
  my ($v,$p,$f) = File::Spec->splitpath($cf);
  open(OUT,">".File::Spec->catfile($inpath,$f)) or die "can't open $f for writing!";
  while (<IN>) { s/[\n\r]+//g; print OUT "\t($_)\n"; }
  push @classd, $f;
}

# resolve classes to make "extended" and "abstracted" grammars
LogiosLog::fail("resolve.pl can't complete!") if
  system("$^X \"$EXEDIR/resolve.pl\" --inpath \"$inpath\""
                                  ." --infile \"$ingra\""
                                  ." --expgra \"$outgra\""
                                  ." --absgra \"$absgra\"");

# fish out the net names
open(TTGRA, "$outgra") or die "compile_gra: can't open $outgra!\n";;
open(NETS, ">",File::Spec->catfile($outpath,"nets")) or die "compile_gra: can't open nets!\n";;
while(<TTGRA>) {
  next unless (/^\[([^\]]+)\]/);
  print NETS "$1\n";
}
close TTGRA; close NETS;

# copy over the forms file
open(TTFORMS, File::Spec->catfile($inpath,"$project.forms")) or die "compile_gra: no $project.forms file!\n";
open(FORMS, ">".File::Spec->catfile($outpath,"forms"));
print FORMS <TTFORMS>;
close TTFORMS; close FORMS;

# compile Phoenix grammar
LogiosLog::say('compile_gra', "doing Phoenix compile");
my $COMPILE = File::Spec->catfile($EXEDIR,$bindir,"compile_grammar").$exten;
my $phoenix_cmd_line = "\"$COMPILE\" -SymBufSize 200000 -MaxSymbol 30000 -TokBufSize 200000 -g . -f $instance";
LogiosLog::fail("Phoenix compilation: $phoenix_cmd_line")
  if not defined open(COMPILE, "$phoenix_cmd_line|");
open(LOG, ">".File::Spec->catfile($outpath,"compile_gra.log")); print LOG <COMPILE>; close LOG;
close COMPILE;

if(!-e 'frames' && -e 'forms') {
# concept leaf needs a 'frames' file, but that file might be called 'forms'
  open(FRAMES, '>frames'); open(FORMS, 'forms');
  print FRAMES <FORMS>;
  close FORMS; close FRAMES
}
my $CONCEPT_LEAF = File::Spec->catfile($EXEDIR,$bindir,"concept_leaf").$exten;
LogiosLog::say('compile_gra', "doing Phoenix concept_leaf");
my $concept_cmd_line = "\"$CONCEPT_LEAF\" -SymBufSize 200000 -grammar $instance.net";
#Bug! concept leaf fails!
#LogiosLog::fail("Phoenix concept_leaf: $concept_cmd_line") if 
system($concept_cmd_line);


# generate the class-grammar files
#  .ctl and .prodef class files for decoder; .token for pronunciation; .words for lm
my $cmd =
    "\"".File::Spec->catfile($EXEDIR,"tokenize.pl")."\""
    ." -inpath \"$inpath\""
    ." -grammar \"$absgra\""
    ." -project $instance"
    .(($pocket_flag)? " -pocket": "")
    ;
    LogiosLog::fail("tokenize.pl: $cmd") if
    system("$^X ".$cmd);

# finally, remove any dynamic class files (to avoid littering with stealth classes)
foreach $cf (@classd) { unlink(File::Spec($inpath,"$cf")); }
