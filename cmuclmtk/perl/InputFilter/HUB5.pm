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
package Text::CMU::InputFilter::HUB5;
use File::Basename;
use IO::File;
use base 'Text::CMU::InputFilter';

sub process_transcript {
    my ($self, $transfile) = @_;

    local (*RAWTRANS, $_);
    open RAWTRANS, "<$transfile" or die "Failed to open $transfile: $!";
    my $opts = $self->{opts};
    my $fillermap = $self->{FillerMap};
    my ($fileid) = fileparse($transfile,'\..*');
    while (<RAWTRANS>) {
	# Blank lines
	next if /^\s*$/;
	# Comments
	next if /^#/;

	# Remove timestamps and speaker name
	my ($start, $end, $speaker);
	if (s/^(\d+(?:\.\d+)?)\s+(\d+(?:\.\d+)?)\s+([^:]+):\s+//) {
	    ($start, $end, $speaker) = ($1, $2, $3);
	}
	else {
	    die "No timing or speaker information in line: $_";
	}
	my $uttid = "${fileid}-${start}-${end}-${speaker}";

	# Split on sentence breaks if requested
	my @sentences;
	if (defined($opts->{sentencebreaks})) {
	    @sentences = split /[$opts->{sentencebreaks}]/;
	}
	else {
	    @sentences = $_;
	}
	foreach (@sentences) {
	    # Remove most punctuation
	    tr/,?!;:"//d;

	    # Oh, this sucks.  There are at least two different "HUB5"
	    # conventions.  Fisher uses periods to mark letter names
	    # (like "c. n. n.") and nowhere else.  Other stuff marks
	    # sentence breaks with periods.  Hope this works!

	    # Remove obvious end-of-sentence periods
	    s/([A-Za-z'][A-Za-z'])\.(\s|$)/$1$2/g;

	    # Remove single-quoted quotations (argh!)
	    s/(?:\s|^)'(.*?)'(\s|$)/$1/g;

	    # Remove commentary
	    s/\[\[[^\]]*\]\]//g;

	    # Do filler mapping
	    if (defined($fillermap)) {
		# Vocal sounds
		s{\{([^\}]+)\}} {
		    if (exists($fillermap->{$1})) {
			$fillermap->{$1};
		    }
		    else {
			warn "Unexpected filler word $1 in $transfile";
		    }
		}ge;
		# Non-vocal sounds
		s{\[([^\]]+)\]} {
		    if (exists($fillermap->{$1})) {
			$fillermap->{$1};
		    }
		    else {
			warn "Unexpected filler word $1 in $transfile";
		    }
		}ge;
	    }
	    else {
		# Just remove fillers
		s{\{([^\}]+)\}}{}g;
		s{\[([^\]]+)\]}{}g;
	    }

	    # Unintelligible speech
	    if ($opts->{unintelligible}) {
		s/\(\(([^\)]*)\)\)/$1/g;
	    }
	    else {
		s/\(\(([^\)]*)\)\)//g;
	    }

	    # Asides, crosstalk
	    if ($opts->{crosstalk}) {
		s/<as\/>([^\/]*)<\/as>/$1/g;
		s/\/\/([^\/]*)\/\//$1/g;
		s/\#([^\#]*)\#/$1/g;
	    }
	    else {
		s/<as\/>([^\/]*)<\/as>//g;
		s/\/\/([^\/]*)\/\///g;
		s/\#([^\#]*)\#//g;
	    }

	    # Mispronunciations
	    if ($opts->{intended}) {
		s/\+([^\+]*)\+/$1/g;
	    }
	    else {
		s/\+([^\+]*)\+//g;
	    }

	    # Foreign language
	    if ($opts->{foreign}) {
		s/<(\S+)\s*([^>]*)>/$2/g;
	    }
	    else {
		s/<(\S+)\s*([^>]*)>//g;
	    }

	    # Keep neologisms
	    s/\*\*([^\*]*)\*\*/$1/g;

	    # Keep non-lexemes
	    tr/%//d;

	    # Collapse sequences of letters like &A &B &C to A_B_C
	    s{((?:[&~][A-Z](?:\s|\b)){2,})} {
		my $s = $1;
		$s =~ tr/&//d;
		$s =~ s/\s+/_/g;
		$s =~ s/_$//;
		"$s ";
	    }ge;

	    # Collapse sequences of letters like a. b. c. to A_B_C
	    s{((?:[A-Za-z]\.(?:'?[Ss])?(?:\s|$)){2,})} {
		my $s = $1;
		$s =~ tr/.//d;
		$s =~ s/\s+/_/g;
		$s =~ s/_$//;
		"$s ";
	    }ge;

	    # Remove interrupted term markings
	    s/\s+--\s+/ /g;

	    # Remove any stray periods
	    s/(\s|^)\.(\s|$)/$1$2/g;

	    # Now split it up
	    my @words;
	    foreach (split) {
		# Special case for single letters
		if (/^[&~]([A-Za-z])$/) {
		    push @words, "$1.";
		}
		# SWB Cellular has these acronyms (argh)
		elsif (/^~/) {
		    # Letters pronounced individually
		    push @words, join '_', split //;
		}
		# Fisher sometimes has acronyms like d._v._d. (will it never end)
		elsif (/.+\._.+/) {
		    $_ =~ tr/.//d;
		    push @words, $_;
		}
		else {
		    # Deal with proper/place name/acronym/neologism markings
		    tr/&^@*//d;

		    # Add it to the list
		    push @words, $_;
		}
	    }

	    # And write it out.
	    $self->output_sentence(\@words, $uttid, $fileid . "_$speaker", $start, $end);
	}
    }
}

1;
__END__

=head1 NAME

Text::CMU::InputFilter::HUB5 - Input Filter for HUB5 format transcripts

=head1 SYNOPSIS

=head1 OPTIONS

=head1 AUTHOR

=cut
