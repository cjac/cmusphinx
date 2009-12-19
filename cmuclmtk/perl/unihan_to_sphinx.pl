#!/usr/bin/perl -w
# -*- mode: cperl; encoding: utf-8 -*-

# Copyright (c) 2006 Carnegie Mellon University.  All rights
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

# unihan_to_sphinx.pl: Build a Sphinx dictionary out of the Unihan
# database.  Used for pronunciation generation in ngram_pronounce.

use strict;
use utf8;
use open qw(:std :utf8);

# Technically W and Y are not initials, but they work okay here
my @initials = qw(B P M F T D N L G K H J Q X ZH CH SH R Z C S H W Y);
my $initrx = join '|', @initials;

sub pinyin_to_sphinx {
    my $pinyin = shift;
    my ($initial, $final);
    # First deal with Pinyin rules for palatal initials
    if ($pinyin =~ /^([JQX])(.*)$/) {
	($initial, $final) = map lc, ($1, $2);
	# Front the vowel
	$final =~ s/^u/ux/;
    }
    # Y does stupid and unpredictable things in our dictionary
    elsif ($pinyin =~ /^(Y)(.*)$/) {
	($initial, $final) = map lc, ($1, $2);
	# Front the vowel but only if it's not u
	$final =~ s/^u(\D+\d)/ux$1/;
    }
    # Deal with null initials
    elsif ($pinyin =~ /^(([AEO]).*)$/) {
	$initial = lc $2 . "s";
	$final = lc $1;
    }
    # Deal with null finals
    elsif ($pinyin =~ /^([ZCS]H)(I\d)/) {
	($initial, $final) = map lc, ($1, $2);
	# retroflex
	$final =~ s/i/ib/;
    }
    elsif ($pinyin =~ /^([ZCS])(I\d)/) {
	($initial, $final) = map lc, ($1, $2);
	$final =~ s/i/if/;
    }
    else {
	$pinyin =~ s/Ü/UX/g;
	($initial, $final) = map lc, ($pinyin =~ /^($initrx)(.*)$/)
    }
    return "$initial $final";
}

while (<>) {
    next if /^#/;
    next if /^\s*$/;
    chomp;
    my ($code, $field, $val) = /^(\S+)\s+(\S+)\s+(.*)$/;
    if ($field eq 'kMandarin') {
	my @readings = split " ", $val;
	foreach (@readings) {
	    $_ = pinyin_to_sphinx($_);
	}
	$code =~ s/U\+/0x/;
	my $char = chr(oct($code));
	my $i = 1;
	foreach (@readings) {
	    # Sadly no neutral tone in our phoneset (no idea why...)
	    next if /5$/;
	    # And this just doesn't exist either for some reason
	    next if /uxn3/;
	    if ($i > 1) {
		print "$char($i) $_\n";
	    }
	    else {
		print "$char $_\n";
	    }
	    ++$i;
	}
    }
}
