#!/usr/bin/perl
# ====================================================================
#
# Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
# reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer. 
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#
# This work was supported in part by funding from the Defense Advanced 
# Research Projects Agency and the National Science Foundation of the 
# United States of America, and the CMU Sphinx Speech Consortium.
#
# THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
# ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
# NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ====================================================================
#
# Author: Evandro Gouvea
#

use File::Path;
use File::Copy;
use File::Basename;
use File::Spec::Functions;

use lib catdir(dirname($0), updir(), 'lib');
use SphinxTrain::Config cfg => 'etc/sphinx_decode.cfg';
use SphinxTrain::Util;

#************************************************************************
# this script performs decoding.
# it needs as inputs a set of models in s3 format
# a mdef file and cepstra with transcription files.
#************************************************************************

$| = 1; # Turn on autoflushing

die "USAGE: $0 <part> <npart>" if (($#ARGV != ($index + 1)) and ($#ARGV != ($index - 1)));

if ($#ARGV == ($index + 1)) {
  $part = $ARGV[$index];
  $npart = $ARGV[$index + 1];
} else {
  $part = 1;
  $npart = 1;
}

$modelname = $ST::DEC_CFG_MODEL_NAME;
$processname = "decode";

$log_dir = "$ST::DEC_CFG_LOG_DIR/$processname";
mkdir ($log_dir,0777) unless -d $log_dir;
$result_dir = "$ST::DEC_CFG_RESULT_DIR";
mkdir ($result_dir,0777) unless -d $result_dir;

$logfile = "$log_dir/${ST::DEC_CFG_EXPTNAME}-${part}-${npart}.log";
$matchfile = "$result_dir/${ST::DEC_CFG_EXPTNAME}-${part}-${npart}.match";
$statepdeffn = $ST::DEC_CFG_HMM_TYPE; # indicates the type of HMMs

$hmm_dir = "$ST::DEC_CFG_BASE_DIR/model_parameters/$modelname";

$nlines = 0;
open INPUT, "${ST::DEC_CFG_LISTOFFILES}";
while (<INPUT>) {
    $nlines++;
}
close INPUT;

$ctloffset = int ( ( $nlines * ( $part - 1 ) ) / $npart );
$ctlcount = int ( ( $nlines * $part ) / $npart ) - $ctloffset;

$ST::DEC_CFG_WORDPENALTY = 0.2 unless defined($ST::DEC_CFG_WORDPENALTY);

Log("Decoding $ctlcount segments starting at $ctloffset (part $part of $npart) ", 'result');
my $rv = RunTool('sphinx3_decode', $logfile, $ctlcount,
		 -senmgau => $statepdeffn,
		 -hmm => $hmm_dir,
		 -lw => $ST::DEC_CFG_LANGUAGEWEIGHT ,
		 -feat => $ST::DEC_CFG_FEATURE,
		 -beam => $ST::DEC_CFG_BEAMWIDTH,
		 -wbeam => $ST::DEC_CFG_WORDBEAM,
		 -dict => $ST::DEC_CFG_DICTIONARY,
		 -fdict => $ST::DEC_CFG_FILLERDICT,
		 -lm => $ST::DEC_CFG_LANGUAGEMODEL,
		 -wip => $ST::DEC_CFG_WORDPENALTY,
		 -ctl => $ST::DEC_CFG_LISTOFFILES,
		 -ctloffset => $ctloffset,
		 -ctlcount => $ctlcount,
		 -cepdir => $ST::DEC_CFG_FEATFILES_DIR,
		 -cepext => $ST::DEC_CFG_FEATFILE_EXTENSION,
		 -hyp => $matchfile,
		 -agc => $ST::DEC_CFG_AGC,
		 -varnorm => $ST::DEC_CFG_VARNORM,
		 -cmn => $ST::DEC_CFG_CMN,
		 @ST::DEC_CFG_EXTRA_ARGS);

if ($rv) {
  LogError("Failed to start ${ST::DEC_CFG_BIN_DIR}/sphinx3_decode");
}
exit ($rv);
