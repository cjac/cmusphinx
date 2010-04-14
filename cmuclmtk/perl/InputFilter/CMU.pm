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
package Text::CMU::InputFilter::CMU;
use XML::Parser;
use IO::File;
use base 'Text::CMU::InputFilter';

sub initialize {
    my $self = shift;

    $self->SUPER::initialize(@_);
    $self->{parser} =
	XML::Parser->new(Handlers =>
			 { Start => sub { _tag_start($self, @_) },
			   Char => sub { _tag_text($self, @_) } });
}


sub process_transcript {
    my ($self, $transfile) = @_;
    $self->{parser}->parsefile($transfile);
    return 1;
}

sub _tag_start {
    my ($self, $expat, $tag, %attr) = @_;
    if ($tag eq 'Trans') {
	$self->{utt_file} = $attr{audio_filename};
	# Have to remove spaces from the utt file due to control file format...
	$self->{utt_file} =~ s/\s/_/g;
	$self->{sync_time} = 0;
	$self->{utt_text} = undef;
    }
    elsif ($tag eq 'Sync') {
	if (defined($self->{utt_text})) {
	    $self->normalize_utt($attr{time});
	    $self->{utt_text} = undef;
	}
	# Ignore bogus sync values
	$self->{sync_time} = $attr{time} unless $attr{time} < $self->{sync_time};
    }
}

sub _tag_text {
    my ($self, $expat, $data) = @_;
    $self->{utt_text} .= $data;
}

sub normalize_utt {
    my ($self, $end) = @_;
    my $start = $self->{sync_time};
    my $text = $self->{utt_text};
    my $dict = $self->{dict};
    # FIXME: We should standardize uttids in a way which makes sclite happy
    my $uttid = "$self->{utt_file}_${start}-${end}";

    # This appears somewhere
    if ($text =~ /#+\s+stopped checking\s+#+/) {
	return;
    }

    # Separate fillers from words
    my $fillermap = $self->{FillerMap};
    if (keys %$fillermap) {
	foreach my $f (keys %$fillermap) {
	    my $ff = quotemeta $f;
	    $text =~ s/$ff/ $f /g;
	}
    }
    else {
	$text =~ s,(#[^# ]+#), $1 ,g;
	$text =~ s,(/[^/ ]+/), $1 ,g;
    }

    # Fix broken fillers
    $text =~ s{/((?:[^/\s]+\s+)+)([^/\s]+)/}
	{ my ($f, $l) = ($1, $2);
	  $f =~ s:\s+:/ /:g;
	  "/$f$l/" }ge;

    # Delete #marked# sections #marked# unless otherwise requested
    my $opts = $self->{opts};
    unless ($opts->{crosstalk}) {
	$text =~ s/(#begin_(crosstalk|remark|comment)#)[^#]+#end_\2#//g;
    }
    unless ($opts->{feed}) {
	$text =~ s/(#begin_(feed|grouch)#)[^#]+#end_\2#//g;
    }
    $text =~ s/#[^# ]+#//g;

    # Remove false starts unless otherwise requested
    if ($opts->{falsestarts}) {
	$text =~ tr/<>/  /;
    }
    else {
	$text =~ s/<[^>]+>//g;
    }

    # Transcribers sometimes type {} instead of [].  Thankfully none
    # of our markup uses curly braces.
    $text =~ tr/{}/[]/;

    # Transcribers sometimes type (foo[bar]) instead of [foo(bar)].
    $text =~ s/\(([^[]+)\[([^]]+)\]\)/[$1($2)]/g;

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
    }
    else {
	# Use "intended" word (second part)
	$text =~ s/\[\s*([^(]+?)\s*\(([^)]+)\)\s*\]/$2/g;
    }

    # Sometimes there are [words] left
    $text =~ tr/[]//d;

    # Normalize compound words (from FOO.BAR to FOO_BAR)
    $text =~ s/~([A-Za-z]+)/ join '_', split '', $1 /ge;
    $text =~ s/([A-Za-z])\.(?=[A-Za-z])/$1_/g;
    $text =~ s/_\b//g;

    # Now do filler mapping
    my @words = split ' ', $text;
    my @outwords;
    foreach my $word (@words) {
	my $nodash = $word;
	$nodash =~ s/-$//;
	$word = $nodash if exists $fillermap->{$nodash};

	if (exists $fillermap->{$word}) {
	    my $outword = $fillermap->{$word};

	    # For LM don't bother with ++FILLERS++
	    push @outwords, $outword
		unless $outword =~ /^\+\+.*\+\+$/ and !$opts->{fillers};
	}
	elsif ($word =~ m,^/.*/$, or $word =~ /^#.*#$/) {
	    warn "Unexpected filler word $word in $self->{utt_file}";
	}
	elsif ($word eq '-') {
	    # Special case ... skip these
	}
	else {
	    # Remove unexpected characters (FIXME: what about non-ASCII...) */
	    $word =~ tr/-A-Za-z_'//cd;
	    # Correct bogus A_ B_ C_ things
	    $word =~ s/^(.)_$/$./;
	    push @outwords, $word;
	}
    }

    # Skip empty utterances
    return unless @outwords;

    # Output an utterance
    $self->output_sentence(\@outwords, $uttid, $self->{utt_file}, $start, $end);
}

1;
__END__

=head1 NAME

Text::CMU::InputFilter::CMU - Input filter for CMU format transcripts

=head1 SYNOPSIS

=head1 OPTIONS

=over 4

=item fillermap

A file listing a mapping of filler words from the input to filler
words in the output.  Each line has an input and an output separated
by whitespace.

=item dict

A dictionary to use for constructing a list of OOVs.

=item crosstalk

Whether to include cross-talk segments

=item feed

Whether to include system messages

=item falsestarts

Whether to include false starts

=item uttered

Whether to use "uttered" or "intended" word for mispronunciations

=item split

Whether to split compound words

=item partials

Whether to include partial words

=item filledpauses

Whether to include filled pauses

=item fillers

Whether to include filler words

=item noisy

Whether to include noise-only utterances

=back

=head1 AUTHOR

David Huggins-Daines E<lt>dhuggins@cs.cmu.eduE<gt>.

=cut
