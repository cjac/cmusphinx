#!/usr/bin/perl
# ====================================================================
# Copyright (c) 2000 Carnegie Mellon University.  All rights reserved.
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
#  Script that launches the main decoder script
#
# ====================================================================

use File::Copy;

if (lc($ARGV[0]) eq '-cfg') {
    $cfg_file = $ARGV[1];
} else {
    $cfg_file = "etc/sphinx_decode.cfg";
}

if (! -s "$cfg_file") {
    print ("unable to find default configuration file, use -cfg file.cfg or create etc/sphinx_decode.cfg for default\n");
    exit -3;
}

require $cfg_file;


#************************************************************************
# this script launches the decoder scripts.
#************************************************************************

$| = 1; # Turn on autoflushing

die "USAGE: $0" if ($#ARGV > 1);

&DEC_Log ("MODULE: DECODE Decoding using models previously trained\n");

for (my $i = 1; $i <= $DEC_CFG_NPART; $i++) {
  system("perl $DEC_CFG_SCRIPT_DIR/decode/s3decode.pl $i $DEC_CFG_NPART");
}

&compute_acc();

sub compute_acc {
  $result_dir = "$DEC_CFG_BASE_DIR/result";
  $match_file = "$result_dir/${DEC_CFG_EXPTNAME}.match";

  &concat_hyp($match_file);
  $tmp_hyp = &condition_text($match_file);
  $tmp_ref = &condition_text($DEC_CFG_TRANSCRIPTFILE);
  &align_hyp($tmp_ref, $tmp_hyp);
  unlink $tmp_ref, $tmp_hyp;
}

sub concat_hyp {
  my $match_file = shift;
  open (MATCH, ">$match_file") or die "Can't open file $match_file\n";
  for (my $i = 1; $i <= $DEC_CFG_NPART; $i++) {

    $hypfile = "$result_dir/${DEC_CFG_EXPTNAME}-${i}-${DEC_CFG_NPART}.match";

    open (HYP, "< $hypfile") or ((warn "Can't open $hypfile\n" and next));
    while (<HYP>) {
      print MATCH "$_";
    }
    close(HYP);
  }
  close(MATCH);
}

sub condition_text {
  my $fn = shift;
  my $tmpfn = $fn.$$;
  my $fileid = $DEC_CFG_LISTOFFILES;

  open (IN, "< $fn") or die "Can't open $fn for reading\n";
  open (OUT, "> $tmpfn") or die "Can't open $tmpfn for writing\n";
  open (LIST, "< $fileid") or die "Can't open $fileid for reading\n";

  while (<IN>) {
    m/^(.*)\((\S+)\)\s*$/;
# Make them uppercase
    my $text = uc($1);
    my $id = uc($2);
# Removing leading spaces
    $text =~ s/^\s+//;
# Removing trailing spaces
    $text =~ s/\s+$//;
# Removing double spaces;
    $text =~ s/\s+/ /g;
# Removing some unwanted strings
    $text =~ s/_/ /g;
    $text =~ s/\./ /g;
    $text =~ s/\+\+UM\+\+/ /g;
    $text =~ s/\+\+UH\+\+/ /g;
    $text =~ s/\(\d+\)\b/ /g;
    my $file = <LIST>;
    @path = split /[\\\/]/, $file;
    my $user;
    if ($#path > 0) {
      $user = $path[$#path - 1];
    } else {
      $user = "user";
    }
    print OUT "$text ($user-$id)\n";
  }
  close(LIST);
  close(OUT);
  close(IN);
  return $tmpfn;
}

sub align_hyp {
  my $ref = shift;
  my $hyp = shift;
  my $align = $DEC_CFG_ALIGN;
  my $rline;
  my $hline;
  my $result;

  if ($align eq 'builtin') {
    my $count = 0;
    my $error = 0;
    open (REF, "<$ref") or die "Can't open $ref\n";
    open (HYP, "<$hyp") or die "Can't open $hyp\n";
    my $outfile = "$DEC_CFG_BASE_DIR/result/${DEC_CFG_EXPTNAME}.align";
    open (OUT, "> $outfile") or die "Can't open $outfile for writing\n";
    while (my $refline = <REF>) {
      $count++;
      my $hypline = <HYP>;
      chomp($refline);
      chomp($hypline);
      if ($refline ne $hypline) {
	$rline = uc($refline);
	$hline = uc($hypline);
	$result = "ERROR";
	$error++;
      } else {
	$rline = lc($refline);
	$hline = lc($hypline);
	$result = "CORRECT";
      }
      print OUT "Sentence $count : $result\n";
      print OUT "\tREF: $rline\n";
      print OUT "\tHYP: $hline\n\n";
    }
    close(REF);
    close(HYP);
    my $pct;
    if ($count > 0) {
      $pct = ($error/$count * 100);
    } else {
      $pct = 0;
    }
    &DEC_Log("SENTENCE ERROR: " . (sprintf "%.3f%", $pct) . 
	    (sprintf " (%d/%d)\n", $error, $count));
    print OUT "\n\nSENTENCE ERROR: " . (sprintf "%.3f%", $pct) . 
	    (sprintf " (%d/%d)\n", $error, $count);
    close(OUT);
  } elsif ($align =~ m/sclite/i) {
    my $outfile = "$DEC_CFG_BASE_DIR/result/${DEC_CFG_EXPTNAME}.align";
    my ($word_total, $word_err, $sent_total, $sent_err);
    open (OUT, "> $outfile") or die "Can't open $outfile for writing\n";
    if (open (PIPE, "\"$align\" " .
	  "-i rm " .
	  "-o rsum pralign dtl stdout " .
	  "-f 0 " .
	  "-r \"$ref\" " .
	  "-h \"$hyp\" 2>&1 |")) {
      while (<PIPE>) {
	print OUT "$_";
	if (m/\|\s*Sum\s*\|/) {
	  my @fields = split /\|/;
	  ($sent_total, $word_total) = ($fields[2] =~ m/(\S+)/g);
	  my @detail = split /\s+/, $fields[3];
	  $word_err = $detail[$#detail - 1];
	  $sent_err = $detail[$#detail];
	}
      }
    }
    close(OUT);
    close(PIPE);
    my $ser;
    if ($sent_total > 0) {
      $ser = ($sent_err/$sent_total * 100);
    } else {
      $ser = 0;
    }
    my $wer;
    if ($word_total > 0) {
      $wer = ($word_err/$word_total * 100);
    } else {
      $wer = 0;
    }
    copy "$DEC_CFG_GIF_DIR/green-ball.gif", "$DEC_CFG_BASE_DIR/.align.state.gif";
    &DEC_HTML_Print ("\t" . &DEC_ImgSrc("$DEC_CFG_BASE_DIR/.align.state.gif") . " ");   
    &DEC_Log("SENTENCE ERROR: " . (sprintf "%.3f%", $ser) . 
	    (sprintf " (%d/%d)", $sent_err, $sent_total) .
	    "   WORD ERROR RATE: " . (sprintf "%.3f%", $wer) . 
	    (sprintf " (%d/%d) ", $word_err, $word_total));
    &DEC_HTML_Print (&DEC_FormatURL("$outfile", "Log File"));
    &DEC_Log("\n");
  } else {
    &DEC_Log("Accuracy not computed, please visually compare the decoder output with the reference file");
  }
}
