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
package Text::CMU::InputFilter::MS98;
use base 'Text::CMU::InputFilter';

sub process_transcript {
    my ($self, $transfile) = @_;

    # Much of the work has already been done for us by the nice folks
    # at ISIP.  This is much better than the original Switchboard format.
    local (*INFILE, $_);
    open INFILE, "<$transfile" or die "Failed to open $transfile: $!";
    while (<INFILE>) {
	chomp;
	my ($uttid, $start, $end, @words) = split;

	my ($fileid) = ($uttid =~ /^([^-]+)/);
	$fileid =~ s/([AB])$/_$1/;
	$fileid =~ s/sw(\d)/sw0$1/;
	# Convert [] conventions to ours (this means that we lose some
	# information for partial words and mispronunciations - we
	# might want to change our convention!)
	my $aside = 0;
	my @newwords;
	foreach (@words) {
	    # Do filler mapping
	    if (exists $self->{FillerMap}{$_}) {
		$_ = $self->{FillerMap}{$_};
	    }
	    # Skip asides if requested
	    $aside = 1 if $_ eq '<b_aside>';
	    $aside = 0 if $_ eq '<e_aside>';
	    next if /<._aside>/;
	    next if $aside and !$self->{opts}{crosstalk};

	    # Use "uttered" not "intended"
	    s/\[([^\/]+)\/(.*)\]/$1/g;
	    # Use "laughter-corrupted" words as is
	    s/\[laughter-(.*)\]/$1/;
	    # Truncate partial words
	    s/\[[^\]]+\]//g;
	    # Remove pronunciation variants (again ... we might want
	    # to keep this info)
	    s/_\d+$//;
	    # Remove "neologism" marker
	    tr/{}//d;
	    push @newwords, $_;
	}
	$self->output_sentence(\@newwords, $uttid, $fileid, $start, $end);
    }
}

1;
__END__

=head1 NAME

Text::CMU::InputFilter::MS98 - Input filter for ISIP MS98 Switchboard transcripts

=head1 SYNOPSIS

=head1 OPTIONS

=head1 AUTHOR

=cut
