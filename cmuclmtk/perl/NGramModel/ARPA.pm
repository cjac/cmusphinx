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
package Text::CMU::NGramModel::NGram;

sub words {
    my $self = shift;
    return @{$self->[1]};
}

sub prob {
    my $self = shift;
    return $self->[0];
}

sub bowt {
    my $self = shift;
    return $self->[2];
}

package Text::CMU::NGramModel::ARPA;
use IO::File;

sub new {
    my $this = shift;
    my $class = ref $this || $this;
    my $self = bless {}, $class;
    $self->initialize(@_);
    return $self;
}

sub initialize {
    my ($self, $file, $maxn) = @_;

    $self->load($file, $maxn) if defined($file);
}

sub load {
    my ($self, $file, $maxn) = @_;
    my $fh = ref($file) ? $file : IO::File->new($file, "<:utf8");
    die "Failed to open $file: $!" unless defined $fh;

    my $ngrams = $self->{ngrams} = [];
    $self->{header} = "";
    local $_;
    while (<$fh>) {
	last if /^\\data\\/;
	$self->{header} .= $_;
    }

    my @count;
    while (<$fh>) {
	chomp;
	last if /^$/;
	/^ngram (\d+)=(\d+)/ and $count[$1]=$2;
    }
    my $n;
    while (<$fh>) {
	chomp;
	next if /^$/;
	last if /^\\end\\/;
	if (/\\(\d+)-grams:/) {
	    $n = $1;
	    last if defined($maxn) and $n > $maxn;
	    next;
	}
	my ($prob, @words) = split;
	my $bowt = pop @words;
	push @{$ngrams->[$n]}, bless([$prob, \@words, $bowt], 'Text::CMU::NGramModel::NGram');
    }
}

sub n {
    my $self = shift;
    return scalar(@{$self->{ngrams}});
}

sub ngrams {
    my ($self, $n) = @_;

    return @{$self->{ngrams}[$n]};
}

1;
__END__

=head1 NAME

Text::CMU::NGramModel::ARPA - Read ARPA-format N-gram language models

=head1 SYNOPSIS

  my $lm = Text::CMU::NGramModel::ARPA->new("foo.arpa");
  foreach my $n (1..$lm->n()) {
    foreach my $ng ($lm->ngrams($n)) {
      print "Unigram: ", $ng->words(), " prob: ", $ng->prob(),
            " bowt: ", $ng->bowt(), "\n";
    }
  }

=head1 DESCRIPTION

=head1 AUTHOR

David Huggins-Daines E<lt>dhuggins@cs.cmu.eduE<gt>

=cut
