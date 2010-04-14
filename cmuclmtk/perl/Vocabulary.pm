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
package Text::CMU::Vocabulary;
use IO::File;

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
    $self->{words} = {};
    $self->{sources} = [];
    if (defined $opts{wfreqfile}) {
	$self->load($opts{wfreqfile});
	delete $opts{wfreqfile};
    }
    if (defined $opts{vocabfile}) {
	$self->load_words($opts{vocabfile});
	delete $opts{vocabfile};
    }
    if (defined $opts{transcript}) {
	$self->add_transcript($opts{transcript});
	delete $opts{transcript};
    }
    if (defined $opts{transcripts}) {
	$self->add_transcript($_) foreach @{$opts{transcripts}};
	delete $opts{transcripts};
    }
    if (defined $opts{words}) {
	$self->add_words(@{$opts{words}});
	delete $opts{words};
    }
}

sub _add_contents {
    my ($self, @contents) = @_;

    foreach my $object (@contents) {
	if (ref $object) {
	    # This could be a <Transcripts> section with transcripts
	    if ($object->can('get_files')) {
		$self->add_transcript($_) foreach $object->get_files();
		# Keep a reference to this object so its temporary
		# directory won't die.
		push @{$self->{sources}}, $object;
	    }
	    # Or it could be another <Vocabulary>
	    elsif ($object->isa('Text::CMU::Vocabulary')) {
		$self->merge($object);
	    }
	}
	else {
	    # Otherwise it is just a list of words.
	    my @lines = split /\015?\012/, $object;
	    foreach (@lines) {
		s/^\s+//;
		s/\s+$//;
		next if /^$/;
		$self->add_word($_);
	    }
	}
    }
}

sub add_words {
    my ($self, @words) = @_;
    $self->{words}{$_}++ foreach @words;
}

*add_word = \&add_words;

sub add_closure_word {
    my ($self, $word) = @_;
    $self->{words}{$word} = 0x7fffffff; # suitably huge number, we hope
}

sub count {
    my ($self, $word) = @_;
    return $self->{words}{$word};
}

sub words {
    my ($self, %opts) = @_;

    my @outwords;
    my $words = $self->{words};
    if ($opts{-gt} ||= $self->{opts}{cutoff}) {
	while (my ($word, $count) = each %$words) {
	    push @outwords, $word if $count > $opts{-gt};
	}
    }
    elsif ($opts{-top} ||= $self->{opts}{topn}) {
	@outwords = sort {$words->{$b} <=> $words->{$a}} keys %$words;
	splice @outwords, $opts{-top};
    }
    else {
	@outwords = keys %$words;
    }
    return sort @outwords;
}

sub add_transcript {
    my ($self, $file) = @_;

    my $fh = ref($file) ? $file : IO::File->new($file, "<:utf8");
    die "Failed to open $file: $!" unless defined $fh;
    push @{$self->{sources}}, $file;
    local $_;
    while (<$fh>) {
	# Remove utterance IDs
	s,\([^\)\(]+\)$,,;
	# Handle class tags if present
	while (m,<([^sS>]|[^>][^>]+)>\s*([^<]+?)\s*</\1>,g) {
	    next if $1 eq 'unmarked';
	    $self->{class}{$1}{$2}++;
	}
	s,<unmarked>\s*([^<]+?)\s*</unmarked>,$1,g;
	s,<([^sS>]|[^>][^>]+)>\s*([^<]+?)\s*</\1>,[$1],g;
	my @tokens = split;
	$self->add_words(@tokens);
    }
}

sub merge {
    my ($self, $other) = @_;

    # We have to respect the other one's cutoff or topn
    if (defined($other->{opts}{cutoff})) {
	while (my ($word, $count) = each %{$other->{words}}) {
	    $self->{words}{$word} += $count
		if $count > $other->{opts}{cutoff};
	}
    }
    elsif (defined($other->{opts}{topn})) {
	my @outwords =
	    sort {$other->{words}{$b} <=> $other->{words}{$a}}
		keys %{$other->{words}};
	splice @outwords, $other->{opts}{topn};
	foreach my $word (@outwords) {
	    $self->{words}{$word} += $other->{words}{$word};
	}
    }
    else {
	while (my ($word, $count) = each %{$other->{words}}) {
	    $self->{words}{$word} += $count;
	}
    }
}

sub load {
    my ($self, $file) = @_;

    my $fh = ref($file) ? $file : IO::File->new($file, "<:utf8");
    die "Failed to open $file: $!" unless defined $fh;
    push @{$self->{sources}}, $file;
    my $words = $self->{words};
    %$words = ();
    local $_;
    while (<$fh>) {
	s/##.*$//;
	next if /^$/;
	my ($word, $count) = split;
	$words->{$word} = $count;
    }
}

sub save {
    my ($self, $file) = @_;

    my $fh = ref($file) ? $file : IO::File->new($file, ">:utf8");
    die "Failed to open $file: $!" unless defined $fh;
    my $words = $self->{words};
    my $date = localtime;
    $fh->print("## Word Frequencies created by LMTraining modules on $date\n");
    $fh->print("## Sources:\n");
    $fh->print("## $_\n") foreach @{$self->{sources}};
    $fh->print("##\n");
    $fh->print("## Includes ", scalar(keys %$words), " words\n");
    while (my ($word, $count) = each %$words) {
	$fh->print("$word\t$count\n");
    }
}

sub load_words {
    my ($self, $file) = @_;

    my $fh = ref($file) ? $file : IO::File->new($file, "<:utf8");
    die "Failed to open $file: $!" unless defined $fh;
    push @{$self->{sources}}, $file;
    my $words = $self->{words};
    %$words = ();
    local $_;
    while (<$fh>) {
	s/##.*$//;
	chomp;
	next if /^$/;
	$words->{$_}++;
    }
}

sub save_words {
    my ($self, $file, @opts) = @_;
    my $fh = ref($file) ? $file : IO::File->new($file, ">:utf8");
    die "Failed to open $file: $!" unless defined $fh;
    my @words = $self->words(@opts);
    my $date = localtime;
    $fh->print("## Vocabulary created by LMTraining modules on $date\n");
    $fh->print("## Sources:\n");
    $fh->print("## $_\n") foreach @{$self->{sources}};
    $fh->print("##\n");
    $fh->print("## Includes ", scalar(@words), " words\n");
    $fh->print("$_\n") foreach @words;
}

1;

__END__

=head1 NAME

Text::CMU::Vocabulary - Object for creating and manipulating vocabularies

=head1 SYNOPSIS

 use Text::CMU::Vocabulary;
 my $v = Text::CMU::Vocabulary->new(%options);
 $v->add_words(qw(FOO BAR BAZ));
 $v->add_transcript("something.trs");
 my @words = $v->words(%options);
 $v->load("vocab.txt");
 $v->save("vocab.txt"); # Words with frequencies
 $v->save_words("vocab.txt", %options); # Sorted word list, CMU SLM-style

=head1 DESCRIPTION

=head1 OPTIONS

=head1 SEE ALSO

=head1 AUTHOR

David Huggins-Daines E<lt>dhuggins@cs.cmu.eduE<gt>

=cut
