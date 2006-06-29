#!/usr/bin/perl 

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

# by Arthur Chan 2006
# Simple conversion of arpa LM fro/to lower case to/fro upper case.
use strict;

if(@ARGV < 3)
{
    print "$0 <inputlm> <outputlm> <lowercase: 0, uppercase:1>\n";
    exit(-1);
}

my ($inputlm,$outputlm,$case)=@ARGV;

die "third argument should be 0 or 1" unless ($case == 0 || $case==1);

open(INPUT,"$inputlm") ||die "Cannot open input LM, $inputlm\n";
open(OUTPUT,">$outputlm") ||die "Cannot open output LM, $outputlm\n";
my $line;
my $process=0;

while($line = <INPUT>)
{
    if($line =~ /^\\data\\/){
	$process =1;
    }

    if($process==1){
	if($line =~ /\\data\\/||   #\data\
	   $line =~ /ngram [0-9]=/|| #eg ngram 1, work up to 9-gram
	   $line =~ /\\[0-9]\-grams:/ || #eg \1-grams, work up to 9-gram
	   $line =~ /\\end\\/
	   ){
	    #don't touch them
	}else{
	    if($case == 0){
		$line =~ tr/A-Z/a-z/ ;
	    }elsif($case==1){
		$line =~ tr/a-z/A-Z/ ;
	    }
	    $line =~ s/<unk>/<UNK>/g; #make sure <UNK> is upper case
	    $line =~ s/<S>/<s>/g; #make sure <s> is lower case
	    $line =~ s/<\/S>/<\/s>/g; #make sure </s> is lower case
	}
    }else{
	#don't touch them
    }
    
    print OUTPUT $line;

}
close(OUTPUT);
close(INPUT);
