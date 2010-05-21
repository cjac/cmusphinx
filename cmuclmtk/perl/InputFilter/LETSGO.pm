# -*- cperl -*-

# Copyright (c) 2010 Carnegie Mellon University.  All rights
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
package Text::CMU::InputFilter::LETSGO;
use File::Basename;
use File::Spec;
use IO::File;
use base 'Text::CMU::InputFilter';

sub process_transcript {
    my ($self, $transfile) = @_;

    local (*RAWTRANS, $_);
    open RAWTRANS, "<$transfile" or die "Failed to open $transfile: $!";
    my $opts = $self->{opts};
    my $fillermap = $self->{FillerMap};
    my ($name, $path) = fileparse($transfile,'\..*');
    my $uttid = File::Spec->catfile($path, $name);
    while (<RAWTRANS>) {
	# Blank lines
	next if /^\s*$/;
	# Comments
	next if /^#/;
	if (/^U:\s+(.*)\s*$/) {
	    my $text = $1;

	    # Fix some class tags
	    $text =~ s/: ([a-z])(?=$|\s)/:$1/g;
	    $text =~ s/:([A-Z])\b/\L:$1/g;
	    $text =~ s/:bus/b/g;

	    unless ($opts->{crosstalk}) {
		$text =~ s/(#begin_(background|noise|crosstalk|remark|comment)#)[^#]+#end_\2#//g;
	    }
	    unless ($opts->{feed}) {
		$text =~ s/(#begin_(feed|grouch)#)[^#]+#end_\2#//g;
	    }

	    # Remove false starts unless otherwise requested
	    if ($opts->{falsestarts}) {
		$text =~ tr/<>/  /;
	    } else {
		$text =~ s/<[^>]+>//g;
	    }

	    # Split apart mispronounced segments so we can use regexes on them
	    $text =~ s/(\[[^]]+\])/my $foo=$1; $foo =~ s,\)\s+(?!]),)] [,g; $foo/ge;

	    # Deal with mispronunciations/partial words
	    if ($opts->{uttered}) {
		# Use "uttered" word (first part) and treat as partial
		#
		# This is really not quite correct because some of these are
		# partial word and others are mispronunciations, and there is
		# no way to distinguish them.
		$text =~ s/\[\s*([^(]+?)\s*\(([^)]+)\)\s*\]/$1-/g;
		# Note that this will produce spurious '-' in cases where the
		# mispronunciation is already marked as a filler word, so we
		# have to remove that later on (guh)
	    } else {
		# Use "intended" word (second part)
		$text =~ s/\[\s*([^(]+?)\s*\(([^)]+)\)\s*\]/$2/g;
	    }

	    # Sometimes there are [words] left
	    $text =~ tr/[]//d;

	    my @tokens = split " ", $text;
	    my @outtokens;
	    foreach (@tokens) {
		# Convert filler words
		if (exists $fillermap->{$_}) {
		    $_ = $fillermap->{$_};
		}
		# Convert class tags to internal format
		if (s/[;:]([^;:]+)$//) {
		    $_ = "<$1>$_</$1>";
		}
		# For LM don't bother with ++FILLERS++
		push @outtokens, $_
		    unless /^\+\+.*\+\+$/ and !$opts->{fillers};
	    }
	    $self->output_sentence(\@outtokens, $uttid);
	}
    }
}

1;
__END__

=head1 NAME

Text::CMU::InputFilter::LETSGO - Input Filter for Let's Go transcripts

=head1 SYNOPSIS

=head1 OPTIONS

=head1 AUTHOR

=cut
