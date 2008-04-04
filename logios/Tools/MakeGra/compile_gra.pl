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

BEGIN { push @INC, "../lib"; }

use strict;
use Getopt::Long;
use File::Spec;

# all MakeGra executables are supposed to be in a known place (rel to the current script)
my ($drive,$dirs,$exec) = File::Spec->splitpath($0);
my $EXEDIR = File::Spec->catpath($drive,$dirs,"");

# some defaults
my $project = "";
my $instance = "";
my $classflag = 0;
my $ingra = "";
my $outgra = "";
my $absgra = "";

if (not GetOptions( "class" => \$classflag,
		    "project:s" => \$project,
		    "instance:s" => \$instance,
		    "ingra:s" => \$ingra,
		    "outgra:s" => \$outgra,
		    "absgra:s" => \$absgra,
		  ) )
  { die "usage: compile_gra [-class] [-project <project> -instance <instance> -ingra <.gra> -outgra <.gra> -absgra <_abs.gra>\n"; }


my $outgra = "$instance.gra";
print STDERR "compile_gra: class->$classflag  ingra->$ingra  outgra->$outgra\n";

# check if a robot names file is available, copy into class file (note DOS)
# HARDWIRED!! This should really be driven through a config file.
if ( $classflag and -e 'TeamTalkRobots' ) {
  open(IN,"TeamTalkRobots") or die "compile_gra: can't open TeamTalkRobots!\n";
  open(OUT,">GRAMMAR/DynamicRobotName.class")
    or die "compile_gra: can't open DynamicRobotName.class!\n";
  while (<IN>) { chomp; print OUT "\t($_)\n"; }
}

# resolve classes to make "extended" and "abstracted" grammars
&fail("resolve.pl can't complete!") if
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
open(TTFORMS, "GRAMMAR/$project.forms") or die "compile_gra: no $project.forms file!\n";
open(FORMS, ">forms");
print FORMS <TTFORMS>;
close TTFORMS; close FORMS;

# compile Phoenix grammar
&fail("Phoenix compilation!") if not defined
  open(COMPILE, File::Spec->catfile($EXEDIR,"bin","compile.exe")." -g . -f $instance |");
open(LOG, ">log"); print LOG <COMPILE>; close LOG;
close COMPILE;

&fail("Phoenix concept_leaf!") if
  system(File::Spec->catfile($EXEDIR,"bin","concept_leaf")." -grammar $instance.net");


# finally, generate the class-grammar files
#  .ctl and .prodef class files for decoder; .token for pronunciation; .words for lm
&fail("tokenize.pl!") if
  system("perl ".File::Spec->catfile($EXEDIR,"tokenize.pl")." -g $absgra -p $instance");



##  utilities  ########################################
sub say {
    my ($system, $txt) = @_;
    print STDERR "$system: $txt$/";
    #print LOG "$system: $txt$/" if $LOGFILE;
}

sub fail { my $reason = shift; &say('fail', $reason); die; }


# exit 1;
#
