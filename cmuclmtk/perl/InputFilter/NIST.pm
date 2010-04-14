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
package Text::CMU::InputFilter::NIST;
use File::Basename;
use base 'Text::CMU::InputFilter';

sub process_transcript {
    my ($self, $transfile) = @_;

    local (*RAWTRANS, $_);
    my ($meeting) = fileparse($transfile, '\..*');
    open RAWTRANS, "<$transfile" or die "Failed to open $transfile: $!";
    while (<RAWTRANS>) {
	my $start="";
	my $end="";
	my $speaker="";
	my $sent="";

	$_=~s/[\n\r\f]//g;
	next if /^$/;

	($start, $end, $speaker, $sent) = /^(\S+)\s+(\S+)\s+([^:]+):\s+(.*)$/;
	my $uttid="${meeting}-${speaker}-$start-$end";

	# Normalize text
	$sent =~ tr/"?,.;//d; #remove " ? , . ;
	$sent =~ s!~([A-Za-z])(\W|$)!$1.$2!g;
	$sent =~ s!~(\w+)!join('_', split('', $1))!ge; # ~DC -> D_C
	$sent =~ s!\^(\w+)!$1!g; # remove ^

	#mark fillers
	#$sent =~ s!{\s*([^}]+)\s*}! ++$1++ !g; # {filler} -> ++filler++
	#$sent =~ s!<([^>]+)>! ++$1++ !g; # <filler> -> ++filler++
	#$sent =~ s/\(\(\s*\)\)/ ++GARBAGE++ /g; # (( )) -> ++GARBAGE++

        #remove fillers
	$sent =~ s!{\s*([^}]+)\s*}! !g; # {filler}
	$sent =~ s!<([^>]+)>! !g; # <filler>
	$sent =~ s/\(\(\s*\)\)/ /g; # remove (( ))

	$sent =~ s/\s*--\s*/ /g; # remove --
	$sent =~ s/(^|\s+)-\s*/ /g; # remove - 
	#$sent =~ s!\+\+/NOISE\+\+!\+\+NOISE\+\+!g; # ++/NOISE++ -> ++NOISE++
	$sent =~ tr/()~^//d; #remove ( ) ~ ^

        # Replace multiple spaces or tabs with a single space
        $sent =~ s/\s+/ /g;
        $sent =~ s/^\s+//g;
        $sent =~ s/\s+$//g;

        # Skip noise only utterance
        if ($sent eq "") { next; }

	my @words = split ' ', $sent;
	$self->output_sentence(\@words, $uttid);
    }
    return 1;
}

1;
__END__

=head1 NAME

Text::CMU::InputFilter::NIST - Input filter for NIST format transcripts

=head1 SYNOPSIS

=head1 OPTIONS

=head1 AUTHOR

=cut
