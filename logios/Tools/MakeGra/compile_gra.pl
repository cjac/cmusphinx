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
my $ingra = "";
my $outgra = "";
my $absgra = "";

if (not GetOptions( "class:s" => \@classf,
		    "project:s" => \$project,
		    "instance:s" => \$instance,
		    "ingra:s" => \$ingra,
		    "outgra:s" => \$outgra,
		    "absgra:s" => \$absgra,
		    "tools:s" => \$tools,
		  ) )
  { die "usage: compile_gra --tools <path> [--class <file> ...] [--project <project> --instance <instance> --ingra <.gra> --outgra <.gra> --absgra <_abs.gra>\n"; }

require "$tools/lib/LogiosLog.pm";
my $outgra = "$instance.gra";
print STDERR "compile_gra:  ingra->$ingra  outgra->$outgra\n";
print STDERR "class-> ",join(" ",@classf),"\n";


# see if any ad-hoc class definitions are provided; put copies of the files into GRAMMAR/
foreach $cf (@classf) {
  open(IN,$cf) or die "can't open class file: $cf\n";
  my ($v,$p,$f) = File::Spec->splitpath($cf);
  open(OUT,">Classes/$f") or die "can't open GRAMMAR/$f for writing!";
  while (<IN>) { s/[\n\r]+//g; print OUT "\t($_)\n"; }
  push @classd, $f;
}

# resolve classes to make "extended" and "abstracted" grammars
LogiosLog::fail("resolve.pl can't complete!") if
  system("perl $EXEDIR/resolve.pl -i $ingra -e $outgra -a $absgra");

# fish out the net names
open(TTGRA, "$outgra") or die "compile_gra: can't open $outgra!\n";;
open(NETS, ">nets") or die "compile_gra: can't open nets!\n";;
while(<TTGRA>) {
  next unless (/^\[([^\]]+)\]/);
  print NETS "$1\n";
}
close TTGRA; close NETS;

# copy over the forms file
open(TTFORMS, "$project.forms") or die "compile_gra: no $project.forms file!\n";
open(FORMS, ">forms");
open(FRAMES, ">frames");
for(<TTFORMS>) {
  print FORMS $_;
  print FRAMES $_;
}
close TTFORMS; close FORMS; close FRAMES;

# compile Phoenix grammar
my $COMPILE = File::Spec->catfile($EXEDIR,$bindir,"compile_grammar").$exten;
LogiosLog::fail("Phoenix compilation!") if not defined open(COMPILE, "$COMPILE -g . -f $instance |");
open(LOG, ">log"); print LOG <COMPILE>; close LOG;
close COMPILE;

my $CONCEPT_LEAF = File::Spec->catfile($EXEDIR,$bindir,"concept_leaf").$exten;
# print STDERR $CONCEPT_LEAF,"\n";
#LogiosLog::fail("Phoenix concept_leaf!") if 
system("$CONCEPT_LEAF -grammar $instance.net");

# finally, generate the class-grammar files
#  .ctl and .prodef class files for decoder; .token for pronunciation; .words for lm
LogiosLog::fail("tokenize.pl!") if
  system("perl ".File::Spec->catfile($EXEDIR,"tokenize.pl")." -g $absgra -p $instance");


# finally, remove any dynamic class files (to avoid littering with stealth classes)
foreach $cf (@classd) { unlink("$cf"); }

#
