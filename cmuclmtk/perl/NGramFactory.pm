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
package Text::CMU::Transcripts;

sub new {
    my $this = shift;
    my $class = ref $this || $this;
    my $self = bless {files => []}, $class;
    $self->initialize(@_);
    return $self;
}

sub initialize {
    my ($self, %opts) = @_;

    if (defined($opts{list})) {
	local (*LIST, $_);
	open LIST, "<$opts{list}" or die "Failed to open $opts{list}: $!";
	while (<LIST>) {
	    chomp;
	    next if /^\s*$/;
	    next if /^#/;
	    s/^\s+//;
	    s/\s+$//;
	    push @{$self->{files}}, $_;
	}
    }
}

sub get_files {
    my ($self, $pattern) = @_;

    my $files = $self->{files};
    if (defined($pattern)) {
	return grep /$pattern/, @$files;
    }
    else {
	return @$files;
    }
}

sub _add_contents {
    my ($self, @contents) = @_;

    foreach my $object (@contents) {
	my $class = ref $object;
	if ($class) {
	    # It's (probably) an InputFilter.  But maybe it's another
	    # <Transcripts>.  Either way, tell it to do its magic and give
	    # us its files.
	    if ($object->can('get_files')) {
		push @{$self->{files}}, $object->get_files();
		# Keep a reference to this object so its temporary
		# directory won't die.
		push @{$self->{sources}}, $object;
	    }
	}
	else {
	    my @lines = split /\015?\012/, $object;
	    foreach (@lines) {
		s/^\s+//;
		s/\s+$//;
		next if /^$/;
		push @{$self->{files}}, $_;
	    }
	}
    }
}

package Text::CMU::NGramFactory;
use Text::CMU::LMTraining;
use File::Spec::Functions qw(catdir);
use File::Path;
use XML::Parser;

sub new {
    my $this = shift;
    my $class = ref $this || $this;
    my $self = bless {}, $class;
    $self->initialize(@_);
    return $self;
}

sub initialize {
    my ($self, %opts) = @_;
    if (exists $opts{builddir}) {
	$self->{builddir} = $opts{builddir};
	delete $opts{builddir};
    }
    $self->{opts} = \%opts;
}

sub train {
    my ($self, $config) = @_;

    my $parser = XML::Parser->new(Style => 'Tree');
    my $tree = $parser->parsefile($config);

    # Now instantiate objects from the tree
    my ($root) = $self->instantiate($tree);

    # And finish doing estimation/interpolation/whatever
    $root->estimate();

    return $root;
}

sub instantiate {
    my ($self, $tree) = @_;

    my @objects;
    my $opts = $self->{opts};
    while (my ($tag, $content) = splice @$tree, 0, 2) {
	my $object;
	if ($tag) {
	    my $attr = shift @$content;
	    if ($tag !~ /^Text::CMU::/) {
		$tag = "Text::CMU::$tag";
	    }
	    if (exists($attr->{ref})) {
		# Retrieve existing objects from the namespace
		$object = $self->{$tag}{$attr->{ref}};
		die "No such $tag object: $attr->{ref}"
		    unless defined($object);
	    }
	    else {
		# Generate a name for it to use as its temporary directory
		$attr->{name} = sprintf "g%04d", ++$self->{counter}
		    unless defined($attr->{name});
		if (defined($self->{builddir})) {
		    my $cleantag = $tag;
		    $cleantag =~ tr/-A-Za-z0-9_//cd;
		    $attr->{tempdir} = catdir($self->{builddir}, $cleantag, $attr->{name});
		    mkpath($attr->{tempdir});
		}
		# Create it anew
		$object = $tag->new(%$opts, %$attr);
		# And record it in the namespace if desired
		$self->{$tag}{$attr->{name}} = $object
		    if exists($attr->{name});
	    }
	    # If there are contents, parse them and pass them to the
	    # object we just created.
	    my @children = $self->instantiate($content);
	    $object->_add_contents(@children);
	}
	else {
	    $object = $content;
	}
	push @objects, $object;
    }

    return @objects;
}

1;
__END__

=head1 NAME

Text::CMU::NGramFactory - Build N-Gram language models from XML configuration file

=head1 SYNOPSIS

 use Text::CMU::NGramFactory;
 my $factory = Text::CMU::NGramFactory->new();
 my $ngram = Text::CMU::NGramFactory->train("config.xml");
 # Proceed as with Text::CMU::NGramModel;

=head1 DESCRIPTION

=head1 CONFIGURATION FILE

=head1 SEE ALSO

L<Text::CMU::NGramModel>, L<Text::CMU::Vocabulary>,
L<Text::CMU::Smoothing>, L<Text::CMU::InputFilter>

=head1 AUTHORS

David Huggins-Daines E<lt>dhuggins@cs.cmu.eduE<gt>

=cut
