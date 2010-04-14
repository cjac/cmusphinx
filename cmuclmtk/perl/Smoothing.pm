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
package Text::CMU::Smoothing;

sub new {
    my $this = shift;
    my $class = ref $this || $this;
    my $self = bless {}, $class;
    $self->initialize(@_);
    return $self;
}

sub initialize {
    my ($self, %opts) = @_;
    $self->{opts} = \%opts;
}

sub set_option {
    my ($self, $opt, $arg) = @_;
    $self->{opts}{$opt} = $arg;
}

sub get_option {
    my ($self, $opt) = @_;
    $self->{opts}{$opt};
}

sub args {
    my $self = shift;

    my $opts = $self->{opts};
    my @args;
    if (defined($self->{opts}{cutoffs})) {
	push @args, -cutoffs => @{$self->{opts}{cutoffs}};
    }
    foreach my $arg (qw(oov_fraction zeroton_fraction min_unicount)) {
	if (defined($self->{opts}{$arg})) {
	    push @args, "-$arg" => $self->{opts}{$arg};
	}
    }
    return @args;
}

package Text::CMU::Smoothing::GoodTuring;
use base 'Text::CMU::Smoothing';

sub args {
    my $self = shift;

    my @args = $self->SUPER::args();
    push @args, "-good_turing";
    if (defined($self->{opts}{disc_ranges})) {
	push @args, "-disc_ranges", @{$self->{opts}{disc_ranges}};
    }
    return @args;
}

package Text::CMU::Smoothing::Linear;
use base 'Text::CMU::Smoothing';

sub args {
    my $self = shift;

    my @args = $self->SUPER::args();
    push @args, "-linear";
    return @args;
}

package Text::CMU::Smoothing::Absolute;
use base 'Text::CMU::Smoothing';

sub args {
    my $self = shift;

    my @args = $self->SUPER::args();
    push @args, "-absolute";
    return @args;
}


package Text::CMU::Smoothing::WittenBell;
use base 'Text::CMU::Smoothing';

sub args {
    my $self = shift;

    my @args = $self->SUPER::args();
    push @args, "-witten_bell";
    return @args;
}

1;

__END__

=head1 NAME

=head1 SYNOPSIS

=head1 DESCRIPTION

=head1 OPTIONS

=head1 SEE ALSO

=head1 AUTHOR

David Huggins-Daines E<lt>dhuggins@cs.cmu.eduE<gt>

=cut
