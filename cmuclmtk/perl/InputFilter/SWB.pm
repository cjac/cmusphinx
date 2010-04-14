# -*- cperl -*-

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

use strict;
package Text::CMU::InputFilter::SWB;
use base 'Text::CMU::InputFilter';

sub process_transcript {
    my ($self, $transfile) = @_;

    local (*RAWTRANS, $_);
    # Open raw transcription
    open RAWTRANS, "<$transfile" or die "Failed to open $transfile: $!";

    #skip header
    $_=<RAWTRANS>;
    while ($_!~/\=\=\=\=\=\=/) {
	$_=<RAWTRANS>;
    }

    my $sent="";
    my @tokens=();
    while (<RAWTRANS>) {
	#start at first line after "======"
	#@tokens=split;
	#$_=join(' ',@tokens);
	$_=~s/[\n\r\f]//g;

	#3 types of lines: start of sentence (A:,B:,@A,@@B), sentence continuation, blank line
	if (/^[ \@\*]*A\:/ || /^[ \@\*]*B\:/) {
	    if ($sent ne "") {
		$self->normalize_text($sent);
		$sent="";
	    }
	    $sent=$_;
	} elsif (/^$/) {
	    #blank line, print previous sentence
	    if ($sent ne "") {
		$self->normalize_text($sent);
		$sent="";
	    }
	} else {
	    #sentence continuation check with previous line
	    if ($sent eq "") {
		print "ERROR: missing spaker marker $_\n";
	    } else {
		$sent=$sent." ".$_;
	    }
	}
    }

    #print the last one
    if ($sent ne "") {
	$self->normalize_text($sent);
    }
    return 1;
}


#normalize the input sentence
sub normalize_text{
    my ($self, $line) = @_;
    my @tokens=();

    #remove speaker markers
    $line =~ s/^\s+//g;        #remove preceeding space
    @tokens=split(/\s+/,$line);
    $line=join(' ',@tokens[1...$#tokens]);

    $line=~s/[\"\?\.\:\!]//g;    #remove punctuation
    $line=~s/\#//g;            #remove marker for simultaneous talking
    $line=~s/3//g;             #accent marker
    $line=~s/\.\.\.//g;        #continuation
    $line=~s/\-\-[\-]*//g;
    $line=~s/\(\(//g;          #not clear speech
    $line=~s/\)\)//g;

    #remove fillers
    $line=~s/\[([^\]])+\]//g;   #remove filler in [xxx]

    #remove comment
    $line=~s/\{([^\}])+\}//g;   #remove comment in {xxx}

    $line=~s/\,//g;             #remove , first

    #group single letter
    $line=~s/\s+/ /g;           # Replace multiple spaces or tabs with a single space
    $line=~s/^\s+//g;
    $line=~s/\s+$//g;
    if($line=~/ /){
	my @words=split(' ',$line);
	# Try to get individual letters as A. B. C. etc
	if ($words[0] =~ /^[^AIai]$/
	    and ($#words == 0 or $words[1] !~/^[A-Z]$/)) {
	    $line=$words[0].".";
	}
	else {
	    $line=$words[0];
	}
	my $i;
	for($i=1;$i<=$#words;$i++){
	    # Join letter sequences to make acronyms (not necessarily a good idea, but...)
	    if($words[$i-1]=~/^[A-Za-z]$/ && $words[$i]=~/^[A-Za-z]$/){
		$line=$line."_".$words[$i];
	    }
	    # Try to get individual letters too
	    elsif ($words[$i] =~ /^[^AIai]$/
		   and ($i == $#words or $words[$i+1] !~/^[A-Za-z]$/)) {
		$line=$line." ".$words[$i].".";
	    }
	    else{
		$line=$line." ".$words[$i];
	    }
	}
    }

    #connect inflection
    $line=~s/ \-/\-/g;

    # Split it again? (the above stuff is wacky)
    $self->output_sentence([split " ", $line]);
}

1;
__END__

=head1 NAME

Text::CMU::InputFilter::SWB - Input filter for Switchboard format transcripts

=head1 SYNOPSIS

=head1 OPTIONS

=head1 AUTHOR

=cut
