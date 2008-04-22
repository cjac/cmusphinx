#!/usr/local/bin/perl -w

#
# ====================================================================
# Copyright (C) 1999-2008 Carnegie Mellon University and Alexander
# Rudnicky. All rights reserved.
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
# Research Projects Agency, the Office of Naval Research and the National
# Science Foundation of the United States of America, and by member
# companies of the Carnegie Mellon Sphinx Speech Consortium. We acknowledge
# the contributions of many volunteers to the expansion and improvement of
# this dictionary.
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

# [20050309] (air) Created.
# strip out stress marks from a cmudict, producing a "SphinxPhones_40" dictionary
# [20080420] (air) Changed to pass comments.
#                  Fixed output collation sequence; DOS eol's
#


if ( scalar @ARGV ne 2 ) { die "usage: make_baseform <input> <output>\n"; }

open(IN, $ARGV[0]) || die "can't open $ARGV[0] for reading!\n";
open(OUT,">$ARGV[1]") || die "can't open $ARGV[1] for writing!\n";

@header = ();  # header comment lines (passed through)
%dict = ();    # words end up in here
%histo = ();   # some statistics on variants

get_dict(\%dict,\@header,IN);  # process the entries

# print special comments (copyright, etc.)
foreach $h (@header) { print OUT "$h\n"; }

# print out each entry
foreach $w (sort keys %dict) {
  $var=0;
  foreach $p (keys %{$dict{$w}}) {
    if ($var eq 0) {
      print  OUT "$w\t$p\n";
      $var++;
    }  else {
      print  OUT "$w\t$p\n";
      $var++;
    }
  }
}

close(IN);
close(OUT);

#
#
# read in a dictionary
sub get_dict {
  my $dict = shift;  # data structure with dictionary entries
  my $header = shift;
  my $target = shift;  # input file

  while (<$target>) {
    s/[\r\n]+$//g;  # DOS-robust chomp;

    # process comments; blank lines ignored
    # presume that ";;; #" will be collected and emitted at the top
    if ($_ =~ /^;;; \#/) { push @$header, $_; next; }  # save header info
    elsif ( $_ =~ /^;;;/ ) { next; }  # ignore plain comments
    elsif ( $_ =~ /^\s*$/ ) { next; }  # ignore blank lines

    ($word,$pron) = /(.+?)\s+(.+?)$/;
    if (! defined $word) { print STDERR "$_\n"; next; }
    if ($word =~ /\)$/) { # variant
      ($root,$variant) = ($word =~ m/(.+?)\((.+?)\)/);
    } else {
      $root = $word;
      $variant = 0;
    }
    $pron = &strip_stress($pron);
    if ($dict->{$root}{$pron}) {  # remove duplicate entries
      print STDERR "duplicate entry: $root ($variant) $pron\n";
    } elsif ( $variant eq 0 ) {
      $dict->{$root}{$pron} = $variant;
    } else {
      $dict->{$root."($variant)"}{$pron} = $variant; # note the variant index
      }
    $histo{$variant}++;
  }
}

# strip stress marks from phonetic symbols
sub strip_stress {
  @pron = split " ", $_[0];
  my $p;
  foreach $p (@pron) { if ( $p =~ /\d$/) { $p =~ s/(\d+)$//; } }
  return ( join(" ",@pron));
}

#
