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
package Text::CMU::FillerMap;
use Text::ParseWords;

sub new {
    my $this = shift;
    my $class = ref $this || $this;
    my $self = bless {}, $class;
    $self->initialize(@_);
    return $self;
}

sub initialize {
    my ($self, %opts) = @_;

    if (defined($opts{file})) {
	$self->read_file($opts{file});
    }
}

sub _add_contents {
    my ($self, @contents) = @_;

    foreach my $text (@contents) {
	my @lines = split /\015?\012/, $text;
	foreach (@lines) {
	    s/^\s+//;
	    s/\s+$//;
	    next if /^$/;
	    add_line($self, $_);
	}
    }
}

sub add_line {
    my ($self, $line) = @_;

    my ($k, $v) = quotewords(',', 0, $_);
    return unless defined($k);
    $v = "" unless defined($v);
    $self->{$k} = $v;
}

sub read_file {
    my ($self, $file) = @_;

    local (*LIST, $_);
    open LIST, "<$file" or die "Failed to open $file: $!";
    while (<LIST>) {
	chomp;
	s/\015$//;
	next if /^\s*$/;
	add_line($self, $_);
    }
}

package Text::CMU::ReplaceList;

sub new {
    my $this = shift;
    my $class = ref $this || $this;
    my $self = bless {}, $class;
    $self->initialize(@_);
    return $self;
}

sub initialize {
    my ($self, %opts) = @_;

    if (defined($opts{file})) {
	$self->read_file($opts{file});
    }
}

sub _add_contents {
    my ($self, @contents) = @_;

    foreach my $text (@contents) {
	my @lines = split /\015?\012/, $text;
	foreach (@lines) {
	    s/^\s+//;
	    s/\s+$//;
	    next if /^$/;
	    add_line($self, $_);
	}
    }
}

sub add_line {
    my ($self, $line) = @_;

    my ($k, $v, $count) = split /:/, $line;
    return unless defined($k);
    $v = "" unless defined($v);
    if ($k =~ s/^\+//) {
	$self->{patterns}{$k} = $v;
    } else {
	# Remove bogus backslashes
	$k =~ tr/\\//d;
	$self->{words}{$k} = $v;
    }
    $self->{counts}{$k} = $count;
}

sub read_file {
    my ($self, $file) = @_;

    local (*LIST, $_);
    open LIST, "<$file" or die "Failed to open $file: $!";
    while (<LIST>) {
	chomp;
	s/\015$//;
	next if /^#/;
	next if /^\s*$/;
	add_line($self, $_);
    }
}

package Text::CMU::WordList;
sub new {
    my $this = shift;
    my $class = ref $this || $this;
    my $self = bless {}, $class;
    $self->initialize(@_);
    return $self;
}

sub initialize {
    my ($self, %opts) = @_;

    if (defined($opts{file})) {
	$self->read_file($opts{file});
    }
}

sub _add_contents {
    my ($self, @contents) = @_;

    foreach my $text (@contents) {
	my @lines = split /\015?\012/, $text;
	foreach (@lines) {
	    s/^\s+//;
	    s/\s+$//;
	    next if /^$/;
	    add_line($self, $_);
	}
    }
}

sub add_line {
    my ($self, $word) = @_;

    $self->{$word}++;
}

sub read_file {
    my ($self, $file) = @_;

    local (*LIST, $_);
    open LIST, "<$file" or die "Failed to open $file: $!";
    while (<LIST>) {
	chomp;
	s/\015$//;
	next if /^#/;
	next if /^\s*$/;
	add_line($self, $_);
    }
}

package Text::CMU::Numbers;
use base 'Text::CMU::WordList';

package Text::CMU::FilledPauses;
use base 'Text::CMU::WordList';

package Text::CMU::InputFilter;
use File::Temp;
use File::Path;
use File::Spec::Functions qw(catfile);
use File::Basename;
use IO::File;
use Carp;

use vars qw(%FilledPauses %Numbers);
%FilledPauses = map {$_ => 1}
    qw(UH UM EH AH HUH HA BAH ER MM OOF HEE ACH EEE EW
       HMM MM-HMM HM-HMM MM-MMM
       UH-HUH HUH-UH MM-HMM-HMM UM-HMM MM-HM OH-HUH
       HMM-HMM HMM-HMM-HMM MMM-HMM HMM-MMM HMPH NUH
       OOH OOO UGH MHM MM-MM UH-OH UH-UH UM-HUH UM-HUM);
%Numbers = map {$_ => 1}
    qw(OH AND POINT ONE TWO THREE FOUR FIVE SIX SEVEN EIGHT NINE TEN
       FIRST SECOND THIRD FOURTH FIFTH SIXTH SEVENTH EIGHTH NINTH TENTH
       ELEVEN TWELVE THIRTEEN FOURTEEN FIFTEEN SIXTEEN
       ELEVENTH TWELFTH THIRTEENTH FOURTEENTH FIFTEENTH SIXTEENTH
       SEVENTEEN EIGHTEEN NINETEEN
       SEVENTEENTH EIGHTEENTH NINETEENTH
       TWENTY THIRTY FORTY FIFTY SIXTY SEVENTY EIGHTY
       TWENTIETH THIRTIETH FORTIETH FIFTIETH SIXTIETH SEVENTIETH EIGHTIETH
       NINETY HUNDRED THOUSAND MILLION BILLION TRILLION
       NINETIETH HUNDREDTH THOUSANDTH MILLIONTH BILLIONTH TRILLIONTH);

sub new {
    my $this = shift;
    my $class = ref $this || $this;
    my $self = bless {}, $class;
    $self->initialize(@_);

    # Add some files if we were given them (we can't do this in
    # initialize() because the subclass won't yet be initialized
    # there)
    my %opts = @_;
    if (defined($opts{list})) {
	local (*LIST, $_);
	open LIST, "<$opts{list}" or die "Failed to open $opts{list}: $!";
	while (<LIST>) {
	    chomp;
	    s/^\s+//;
	    s/\s+$//;
	    next if /^$/;
	    next if /^#/;
	    $self->add_files($_);
	}
    }
    return $self;
}

sub initialize {
    my ($self, %opts) = @_;

    if (defined($opts{tempdir})) {
	$self->{tempdir} = $opts{tempdir};
	$opts{nodelete} = 1;
    }
    else {
	$self->{tempdir} = File::Temp::tempdir(CLEANUP => 1);
    }
    if (defined($opts{fillermap})) {
	print STDERR "Reading $opts{fillermap}\n" if $opts{verbose};
	$self->{FillerMap} = Text::CMU::FillerMap->new(file => $opts{fillermap});
    }
    if (defined($opts{replacelist})) {
	print STDERR "Reading $opts{replacelist}\n" if $opts{verbose};
	$self->{ReplaceList} = Text::CMU::ReplaceList->new(file => $opts{replacelist});
    }
    if (defined($opts{filledpauselist})) {
	$self->{FilledPauses} = Text::CMU::WordList->new(file => $opts{filledpauselist});
    }
    else {
	$self->{FilledPauses} = \%FilledPauses;
    }
    if (defined($opts{numberlist})) {
	$self->{Numbers} = Text::CMU::WordList->new(file => $opts{numberlist});
    }
    else {
	$self->{Numbers} = \%Numbers;
    }
    $self->{opts} = \%opts;
}

sub DESTROY {
    my $self = shift;
    # Remove the temporary directory when this object is destroyed
    rmtree($self->{tempdir}) unless $self->{opts}{nodelete};
}

sub tempdir {
    my $self = shift;
    return $self->{tempdir};
}

sub _add_contents {
    my ($self, @contents) = @_;

    foreach my $object (@contents) {
	my $class = ref $object;
	if ($class) {
	    # Check to see if it can get_files, in case someone wants
	    # to stack InputFilters, or use a named <Transcripts>
	    # object or something.
	    if ($object->can('get_files')) {
		$self->add_files($object->get_files());
		# Keep a reference to this object so its temporary
		# directory won't die.
		push @{$self->{sources}}, $object;
	    }
	    else {
		# In this case it will be a ReplaceList, FillerMap,
		# FilledPauses, or Numbers
		$self->{$class} = $object;
	    }
	}
	else {
	    # Otherwise it is (part of) a list of files that I am
	    # supposed to filter and hand back to whatever
	    # instantiated me (probably a <Transcripts> section)
	    my @lines = split /\015?\012/, $object;
	    foreach (@lines) {
		s/^\s+//;
		s/\s+$//;
		next if /^$/;
		$self->add_files($_);
	    }
	}
    }
}

sub add_files {
    my ($self, @files) = @_;

    foreach (@files) {
	my $tempfile = catfile($self->{tempdir},
			       sprintf ("%03d_%s",
					++$self->{transcount},
					basename($_)));
	if ($self->{opts}{reusefiles} and -r $tempfile) {
	    print STDERR "Reusing existing $tempfile\n" if $self->{opts}{verbose};
	}
	else {
	    print STDERR "Filtering $_ to $tempfile\n" if $self->{opts}{verbose};
	    $self->normalize_transcript($_, $tempfile);
	}
	push @{$self->{files}}, $tempfile;
    }
}

sub get_files {
    my ($self, $pattern) = @_;

    my $files = defined($self->{files}) ? $self->{files} : [];
    if (defined($pattern)) {
	return grep /$pattern/, @$files;
    }
    else {
	return @$files;
    }
}

sub normalize_transcript {
    my ($self, $transfile, $outfile, $outctl) = @_;

    if (defined($outfile)) {
	$self->{outfile} = IO::File->new($outfile, ">:utf8")
	    or die "Failed to open $outfile: $!";
    }
    if (defined($outctl)) {
	$self->{outctl} = IO::File->new($outctl, ">:utf8")
	    or die "Failed to open $outctl: $!";
    }
    $self->process_transcript($transfile);
    delete $self->{outfile};
    delete $self->{outctl};
    return 1;
}

sub output_sentence {
    my ($self, $words, $uttid, $fileid, $start, $end) = @_;

    my $filledpauses = $self->{FilledPauses};
    my $numbers = $self->{Numbers};
    my $replacelist = $self->{ReplaceList};
    # Do user-defined replacements
    my @words;
    foreach my $word (@$words) {
	if (defined($replacelist->{words})) {
	    my $map = $replacelist->{words};
	    my $counts = $replacelist->{counts};
	    if (exists $map->{$word}) {
		if (--$counts->{$word} < 0) {
		    warn "Too many replacements for $word:$map->{$word}"
			if $self->{opts}{verbose};
		}
		$word = $map->{$word};
	    }
	    # Remove empty words
	    next unless length($word);
	}

	# Remove partial words unless requested not to
	next if ($word =~ /^\-/ or $word =~ /\-$/) and not $self->{opts}{partials};

	# Remove filled pauses unless requested not to
	next if $filledpauses->{$word} and not $self->{opts}{filledpauses};

	# Parts of the word (for splitting, etc)
	my @parts = ($word);

	# Special case for splitting numbers
	unless ($self->{opts}{compound_numbers}) {
	    my @t = split(/[-_]/, $word);
	    # If they are all numbers
	    unless (grep !$numbers->{$_}, @t) {
		# Then split them.
		@parts = @t;
	    }
	}

	# Split compound words if requested
	# Don't split filler words!!!
	if ($self->{opts}{split} and $word !~ /^\+\+.*\+\+$/) {
	    @parts = split(/[-_ ]/, $word);

	    # Do some monkey business to fix stray 'S after
	    # acronyms (a problem with ICSI mostly)
	    if (@parts > 1 and $parts[-1] =~ /^'/) {
		my $apos = pop @parts;
		$parts[-1] =~ s/\.$//;
		$parts[-1] .= $apos;
	    }

	    # Do further monkey business to fix-up broken partials
	    if ($word =~ /-$/ and @parts) {
		$parts[-1] .= "-";
	    }
	}

	foreach (@parts) {
	    # We now remove all ending dots (and underscores) from
	    # acronym letters, because they are never used
	    # consistently in the input.
	    s/^(.)[._]$/$1/;
	}

	push @words, @parts;
    }

    # Skip empty utterances
    return unless @words;
    # Skip noise-only utterances unless requested not to
    unless ($self->{opts}{noisy}) {
	my $word;
	foreach my $w (@words) {
	    unless ($w =~ /^<sil>$/i or $w =~ /^\+\+.*\+\+$/) {
		$word = $w;
		last;
	    }
	}
	return unless defined $word;
    }

    # Upper or lowercase if requested
    if ($self->{opts}{upper}) {
	foreach my $w (@words) {
	    $w = uc $w;
	}
    }
    if ($self->{opts}{lower}) {
	foreach my $w (@words) {
	    $w = lc $w;
	}
    }

    # Construct string from words
    my $text = "@words";
    # Do user-defined substitutions
    if (defined($replacelist->{patterns})) {
	my $counts = $replacelist->{counts};
	while (my ($pat, $rep) = each %{$replacelist->{patterns}}) {
	    while ($text =~ s/$pat/$rep/) {
		if (--$counts->{$pat} < 0) {
		    warn "Too many replacements for $pat:$rep"
			if $self->{opts}{verbose};
		}
	    }
	}
    }

    if (defined($self->{outfile})) {
	$self->{outfile}->print(defined($uttid)
				? "<s> $text </s> ($uttid)\n"
				: "<s> $text </s>\n");
    }
    if (defined($self->{outctl})) {
	$fileid = $uttid unless defined $fileid;
	if (defined($start) and defined($end)) {
	    my $sf = int($start * $self->{opts}{fps} + 0.5);
	    my $ef = int($end * $self->{opts}{fps} + 0.5);
	    $self->{outctl}->print("$fileid $sf $ef $uttid\n");
	}
	else {
	    $self->{outctl}->print("$fileid\n");
	}
    }
}

1;
__END__

=head1 NAME

Text::CMU::InputFilter - Input filter for language and acoustic model training

=head1 SYNOPSIS

=head1 METHODS

=over 4

=item B<new>

=item B<initialize>

=item B<process_transcript>

=back

=head1 OPTIONS

=over 4

=item fillermap

A file giving a mapping of filler words from the input to filler
words in the output.  Each line has an input and an output separated
by commas.

=item dict

A pronounciation dictionary to use for constructing a list of OOVs.

=item crosstalk

Whether to include cross-talk segments.

=item fillers

Whether to include filler words (i.e. for acoustic model training).

=item noisy

Whether to include noise-only utterances.

=item feed

Whether to include system messages.

=item falsestarts

Whether to include false starts.

=item uttered

Whether to use the "uttered" or "intended" word for mispronunciations.

=item split

Whether to split compound words.

=item partials

Whether to include partial words.

=item filledpauses

Whether to include filled pauses

=item sentencebreaks

The set of punctuation used to break sentences.  (NOTE: This is not
effective with all input filters)

=back

=head1 AUTHOR

David Huggins-Daines <dhuggins@cs.cmu.edu>

=cut
