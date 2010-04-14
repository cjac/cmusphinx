# -*- perl -*-
# cperl-mode breaks on some regexes in this file@#$%!#$%

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
package Text::CMU::InputFilter::ISL;
use File::Basename;
use IO::File;
use base 'Text::CMU::InputFilter';

sub process_transcript {
    my ($self, $transfile) = @_;

    local (*RAWTRANS, $_);
    my ($meeting) = fileparse($transfile, '\..*');
    open RAWTRANS, "<$transfile" or die "Failed to open $transfile: $!";
    my $fillermap = $self->{FillerMap};
    my $opts = $self->{opts};
    my $sentence_breaks = $opts->{sentencebreaks};
    while (<RAWTRANS>) {
	next if /^$/;
	if (/^;\s*(([^:]+):\s*(.*))?/) {
	    if ($1) {
		my ($param, $val) = ($2, $3);
		# But we don't want to do anything with it
	    }
	}
        elsif (/^([^:]+):\s*(.*)$/) {
	    my ($uttid, $text) = ($1, $2);

	    # Effectively it is tokenized already, but we have to
	    # parse it lightly.
	    my @tokens = split " ", $text;
	    my @outtokens;
	    my ($false_start, $correction) = (0,0);
	    my $i = 0;
	    foreach (@tokens) {
		my ($false_start_end, $correction_end);

		# "Parse" false start and correction tags
		$false_start = 1 if s,^-/,,;
		$false_start_end = 1 if s,/-$,,;
		$correction = 1 if s,^\+/,,;
		$correction_end = 1 if s,/\+$,,;

		# Split sentence on punctuation
		if (defined($sentence_breaks) and /^[$sentence_breaks]$/ and @outtokens) {
		    $self->output_sentence(\@outtokens, $uttid);
		    @outtokens = ();
		}

		# Remove all punctuation
		tr/.,?()//d;

		# Change compound word convention
		tr/-/_/;

		# Change partial word convention
		s/[=_]$/-/;
		s/^_/-/;

		# Remove neologism marker
		s/^\*//;

		# Remove mumble marker
		s/%$//;

		# Remove turn markers
		s/<T_>//;
		s/<_T>//;
		$_="" if /^<\*T>/;

		# Remove foreign word tag
		s/^<\*[A-Z]+>//;

		# Convert acronyms
		if (/^[A-Z][A-Z]+$/) {
		    $_ = join "_", split //, $_;
		}

		# Convert filler words
		if (exists $fillermap->{$_}) {
		    $_ = $fillermap->{$_};
		}

		# Now add it to the list unless various conditions are met
		unless ((!($opts->{falsestarts}) and ($false_start))
			or (!($opts->{uttered}) and ($correction))
			or /^\+\+.*\+\+$/ or /^$/) {
		    push @outtokens, $_;
		}

		# Complete "parsing" of false start, corrections
		$false_start = $false_start_end = 0 if $false_start_end;
		$correction = $correction_end = 0 if $correction_end;
	    }
	    continue {
		++$i;
	    }
	    if (@outtokens) {
		$self->output_sentence(\@outtokens, $uttid);
	    }
        }
    }
    return 1;
}

1;
__END__

=head1 NAME

Text::CMU::InputFilter::ISL - Input filter for ISL (VerbMobil) format transcripts

=head1 SYNOPSIS

=head1 OPTIONS

=over 4

=item sentencebreaks

Specifies the set of punctuation which will be used to break
sentences.  Default is none (this leads to some very long sentences,
which may not be what you want for language modeling).

=back

=head1 AUTHOR

=cut
