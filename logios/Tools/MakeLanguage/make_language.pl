#!/usr/bin/perl
# script to create a n-gram language model and a dict from a Phoenix grammar,
# using a stochastically generated sentence corpus.
# [20070405] (tkharris) Created.

# [20070925] (air) modified to pass class markers through
# [20070926] (air) fixed flat and vocab routines to handle class markers and probs
# [20070927] (air) added additional class file processing

# [20071011] (air) refactored to separate functional units and to generalize
# [20080812] (tkharris) refactored to use perl module
# [20080919] (air) minor fixes

use File::Spec;
use Getopt::Long;
use strict;

sub usage {
  return "usage: $0 [--resources <abspath>] [--samplesize <n>] [--source {local|web}]"
    ." --project <dname> --instance <pname> [--logfile <fname>] "
      ." --logios <abspath> --olympus --force$/";
}

#process command line
my ($SOURCE, $LOGIOS, $OLYMODE, $RESOURCES, $INPATH, $OUTPATH, $SAMPSIZE, $FORCE,
    $PROJECT, $INSTANCE, $LOGFILE);
if ( not GetOptions(
	   "source=s", \$SOURCE,
	   "logios=s", \$LOGIOS,

	   "olympus!", \$OLYMODE,
	   "force!",   \$FORCE,  # cause language to be always recompiled

	   "resources=s", \$RESOURCES,  # needs to be an abs path to the root
	   "inpath=s", \$INPATH,
	   "outpath=s", \$OUTPATH,

	   "samplesize=s", \$SAMPSIZE,
	   "project=s", \$PROJECT,
	   "instance=s", \$INSTANCE,

	   "logfile=s", \$LOGFILE,
	  ) )
  { die &usage; }

die &usage if not $LOGIOS;
require File::Spec->catfile($LOGIOS, 'scripts', 'Logios.pm');
my $logios = new Logios('SOURCE' => $SOURCE,
                        'LOGIOS' => $LOGIOS,
                        'OLYMODE' => $OLYMODE,
			'FORCE' => $FORCE,
                        'RESOURCES' => $RESOURCES,
                        'INPATH' => $INPATH,
                        'OUTPATH' => $OUTPATH,
                        'SAMPSIZE' => $SAMPSIZE,
                        'PROJECT' => $PROJECT,
                        'INSTANCE' => $INSTANCE,
                        'LOGFILE' => $LOGFILE);

$logios->compile_grammar;
$logios->makelm;
$logios->makedict;

#
