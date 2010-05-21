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

package Text::CMU::Interpolation;

sub new {
    my $this = shift;
    my $class = ref $this || $this;
    my $self = bless {}, $class;
    $self->initialize(@_);
    return $self;
}

sub initialize {
}

sub _add_contents {
    my ($self, @contents) = @_;

    foreach my $object (@contents) {
	my $class = ref $object;
	if ($class) {
	    if ($object->can('get_files')) {
		$self->add_files($object->get_files());
		# Keep a reference to this object so its temporary
		# directory won't die.
		push @{$self->{sources}}, $object;
	    }
	    elsif ($object->isa('Text::CMU::NGramModel')) {
		# Add it to inputs
		push @{$self->{inputs}}, $object;
	    }
	}
	else {
	    # Ignore it, I guess?
	}
    }
}

sub add_files {
    my ($self, @files) = @_;
    push @{$self->{heldout}}, @files;
}

sub estimate {
    my ($self, $lmout) = @_;

    my $heldout = $self->{heldout};
    my $opts = $self->{opts};
    my $inputs = $self->{inputs};

    # Estimate all input LMs then evaluate them with the held-out set
    my %pplx;
    foreach my $lm (@$inputs) {
	$lm->estimate();
	my $result = $lm->evaluate($heldout);
	$pplx{$lm} = $result->perplexity();
	$lmout->log_message("Perplexity of ", $lm->name(),
			    ": $pplx{$lm}");
    }

    # Now merge them one by one
    my @lms;
    if ($opts->{reverse}) {
	@lms = sort {$pplx{$b} <=> $pplx{$a}} @$inputs;
    } else {
	@lms = sort {$pplx{$a} <=> $pplx{$b}} @$inputs;
    }
    my $lm1 = shift @lms;
    my $last_pplx = $pplx{$lm1};
    $lmout->log_message("Perplexity of ", $lm1->name(), ": $last_pplx");
    foreach my $lm2 (@lms) {
	# Create a new interpolated LM
	my $pplx = $lmout->interpolate($heldout, $lm1, $lm2);
	$lmout->log_message("Perplexity after adding ", $lm2->name(), ": $pplx");
	if (defined($last_pplx)
	    and not($opts->{all} or $opts->{reverse})
	    and $last_pplx < $pplx) {
	    $lmout->log_message("Perplexity has increased ($pplx > $last_pplx), ",
				"will not continue interpolation");
	    last;
	}

	# Use it as the basis for further interpolation
	$lm1 = $lmout;
	$last_pplx = $pplx;
    }
    return $last_pplx;
}

package Text::CMU::NGramModel::Evaluation;

sub perplexity {
    my $self = shift;
    return $$self[0];
}

sub oov {
    my $self = shift;
    return $$self[1];
}

sub ngram_hits {
    my ($self, $n) = @_;
    return $$self[2+$n];
}

package Text::CMU::NGramModel;
use IPC::Open3;
use File::Spec::Functions qw(catfile devnull file_name_is_absolute);
use File::Basename;
use File::Temp;
use File::Path;
use File::Copy;
use Text::CMU::Smoothing;
use Text::CMU::InputFilter;

sub new {
    my $this = shift;
    my $class = ref $this || $this;
    my $self = bless {}, $class;
    $self->initialize(@_);
    return $self;
}

use vars qw(%Defaults);
%Defaults = (
	     context => [qw(<s>)],
	     smoothing => 'Text::CMU::Smoothing::GoodTuring',
	     n => 3
	    );
sub initialize {
    my ($self, %opts) = @_;

    %opts = (%Defaults, %opts);
    # For now we just shell out to the SLM toolkit internally.  So we
    # will maintain a temporary directory for this object where we
    # will store all scratch files created by the toolkit.
    if (defined($opts{tempdir})) {
	$self->{tempdir} = $opts{tempdir};
	$opts{nodelete} = 1;
    }
    else {
	$self->{tempdir} = File::Temp::tempdir(CLEANUP => 1);
    }
    if (defined($opts{logfile})) {
	$self->{logfh} = IO::File->new($opts{logfile}, ">>:utf8");
    }
    elsif ($opts{verbose}) {
	$self->{logfh} = \*STDERR;
    }
    if (defined($opts{arpafile})) {
	$self->load($opts{arpafile});
    }
    if (defined($opts{contextfile})) {
	my $cfh = IO::File->new($opts{contextfile}, "<:utf8")
	    or die "Failed to open $opts{contextfile}: $!";
	local $_;
	$opts{context} = [];
	while (<$cfh>) {
	    chomp;
	    push @{$opts{context}}, $_;
	}
    }
    $opts{cutoffs} = [split ",", $opts{cutoffs}]
	if defined $opts{cutoffs};
    if (defined $opts{disc_ranges}) {
	$opts{disc_ranges} = [split ",", $opts{disc_ranges}];
    }
    else {
	$opts{disc_ranges} = [0];
	push @{$opts{disc_ranges}}, 7 for (2..$opts{n});
    }
    $self->{inputfilter} = $opts{inputfilter}->new(%opts)
	if defined($opts{inputfilter});
    $self->{smoothing} = $opts{smoothing}->new(%opts)
	if defined($opts{smoothing});
    $self->{opts} = \%opts;
    return $self;
}

sub _add_contents {
    my ($self, @contents) = @_;

    foreach my $object (@contents) {
	if (ref $object) {
	    # BEWARE: This little section is not particularly clean
	    # or OO-kosher.

	    # This could be a <Transcripts> section with transcripts, or
	    # something else that knows how to give us a list of files
	    # (an InputFilter will also work).
	    if ($object->can('get_files')) {
		$self->add_transcript($_) foreach $object->get_files();
		# Keep a reference to this object so its temporary
		# directory won't die.
		push @{$self->{sources}}, $object;
	    }
	    # Or it could be a <Vocabulary>
	    elsif ($object->isa('Text::CMU::Vocabulary')) {
		$self->{opts}{vocabulary} = $object;
	    }
	    # Or it could be a <Text::CMU::Smoothing>
	    elsif ($object->isa('Text::CMU::Smoothing')) {
		$self->{smoothing} = $object;
	    }
	    # Or it could be a <Interpolation>
	    elsif ($object->isa('Text::CMU::Interpolation')) {
		$self->{interpolation} = $object;
	    }
	    # Otherwise, ignore it (it could be another NGramModel)
	}
	else {
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

sub log_message {
    my $self = shift;
    my ($package, $filename, $line, $sub) = caller(1);
    $self->{logfh}->print("$filename:$line:$sub: ", @_, "\n")
	if defined($self->{logfh});
}

sub tempdir {
    my $self = shift;
    return $self->{tempdir};
}

sub tempfile {
    my ($self, $file) = @_;
    # Cache it, and also allow it to be overriden (useful for .arpa
    # files in evaluation)
    return $self->{files}{$file} if defined $self->{files}{$file};
    return ($self->{files}{$file} = catfile($self->{tempdir}, $file));
}

sub set_option {
    my ($self, $opt, $arg) = @_;
    $self->{opts}{$opt} = $arg;
}

sub get_option {
    my ($self, $opt) = @_;
    $self->{opts}{$opt};
}

sub DESTROY {
    my $self = shift;
    # Remove the temporary directory when this object is destroyed
    rmtree($self->{tempdir}) unless $self->{opts}{nodelete};
}

sub add_transcript {
    my ($self, $transcript) = @_;

    # Do some input filtering if need be
    if (defined($self->{inputfilter})) {
	my $tempfile = $self->tempfile(sprintf "%03d_%s",
				       ++$self->{transcount},
				       basename($transcript));
	$self->log_message("Filtering $transcript to $tempfile");
	$self->{inputfilter}->normalize_transcript($transcript, $tempfile);
	$transcript = $tempfile;
    }

    # Defer doing anything with it until estimate() gets called
    push @{$self->{transcripts}}, $transcript;
}

sub text2idngram {
    my ($self, $input, $output, $args) = @_;
    $self->{class} = {};
    die "text2idngram requires input" unless defined($input) and @$input;
    $self->run('text2idngram', $input, $output, undef, $args);
}

sub idngram2lm {
    my ($self, $args) = @_;
    $self->run('idngram2lm', undef, undef, undef, $args);
}

sub run {
    my ($self, $bin, $input, $output, $error, $args) = @_;

    # Normalize arguments and such
    $input = [] unless defined $input;
    $output = devnull() unless defined($output) or $self->{opts}{verbose};
    $error = devnull() unless defined($error) or $self->{opts}{verbose};

    $bin = catfile($self->{opts}{bindir}, $bin)
	if defined($self->{opts}{bindir}) and !file_name_is_absolute($bin);

    $args = [] unless defined $args;
    $args = [%$args] if ref $args eq 'HASH';
    $input = ref($input) ? $input : [$input];
    my $instring;
    if (@$input > 10) {
	$instring = "$$input[0] ... ("
	    . scalar(@$input) . " files) ... $$input[-1]";
    }
    else {
	$instring = "@$input";
    }
    $self->log_message("Running $bin ",
		       @$input ? "<[$instring] ":" ",
		       defined($output) ? ">$output ":" ",
		       defined($error) ? "2>$error ":" ",
		       " @$args");

    # DO IT
    local (*READER, *WRITER, $_);
    pipe READER, WRITER;
    my $pid;
    if ($pid = fork()) {
	close READER;
	binmode WRITER, ':utf8';
	my $i = 0;
	foreach my $in (@$input) {
	    my $infh = IO::File->new($in, "<:utf8")
		or die "Failed to open $in: $!";
	    while (<$infh>) {
		# Remove utterance ID if present
		s/\([^\)\(]+\)$//;
		# Handle class tags if present
		while (m,<([^sS>]|[^>][^>]+)>\s*([^<]+?)\s*</\1>,g) {
		    next if $1 eq 'unmarked';
		    $self->{class}{$1}{$2}++;
		}
		s,<unmarked>\s*([^<]+?)\s*</unmarked>,$1,g;
		s,<([^sS>]|[^>][^>]+)>\s*([^<]+?)\s*</\1>,[$1],g;
		# Remove utterance IDs
		s,\([^\)\(]+\)$,,;
		print WRITER $_
		    or die "Failed to write to child: $!";
	    }
	}
	close WRITER;
    }
    elsif (defined($pid)) {
	close WRITER;
	binmode READER, ':utf8';
	open STDIN, "<&READER" or die "Failed to dup STDIN: $!";
	if (defined($output)) {
	    open STDOUT, ">$output"
		or die "Failed to open $output: $!";
	}
	if (defined($error)) {
	    open STDERR, ">$error"
		or die "Failed to open $error: $!";
	}
	exec $bin, @$args or die "Failed to exec: $!";
    }
    else {
	die "Failed to fork: $!";
    }
    waitpid $pid, 0;
    return $? == 0;
}

sub estimate {
    my $self = shift;

    # Delegate to Interpolation if one exists
    if (defined($self->{interpolation})) {
	return $self->{interpolation}->estimate($self);
    }

    # Construct a vocabulary if none was provided
    my $vocab = $self->{opts}{vocabulary};
    unless (defined($vocab)) {
	# Pass along any options that might be useful
	$self->log_message("Building vocabulary");
	$vocab = Text::CMU::Vocabulary->new(transcripts => $self->{transcripts},
					    %{$self->{opts}});
    }
    # Add context cues to the vocabulary, or the world will explode
    foreach my $ctx (@{$self->{opts}{context}}) {
	$self->log_message("Adding context word $ctx to vocabulary");
	$vocab->add_closure_word($ctx);
    }

    # Dump vocabulary according to options
    my $vocabfile = $self->tempfile("vocab");
    $self->log_message("Saving vocabulary to $vocabfile");
    $vocab->save_words($vocabfile);

    # Run text2idngram
    my $idngramfile = $self->tempfile("idngram");
    $self->log_message("Counting N-grams from all transcripts to $idngramfile");
    $self->text2idngram($self->{transcripts},
			undef,
			[-vocab => $vocabfile,
			 -idngram => $idngramfile,
			 -temp => $self->tempdir(),
			 -n => $self->{opts}{n}])
	or die "text2idngram failed with status $?";

    # Run idngram2lm
    my $arpalmfile = $self->tempfile("arpabo");
    my $contextfile = $self->tempfile("context");
    $self->save_context($contextfile);
    $self->log_message("Constructing ARPA language model in $arpalmfile");
    $self->idngram2lm([-idngram => $idngramfile,
		       -vocab => $vocabfile,
		       -arpa => $arpalmfile,
		       -context => $contextfile,
		       -vocab_type => $self->{opts}{open} ? 1 : 0,
		       -n => $self->{opts}{n},
		       '-four_byte_counts',
		       '-calc_mem',
		       $self->{smoothing}->args()])
	or die "idngram2lm failed with status $?";

    # Save probdef and lmctl if requested
    $self->save_probdef($self->tempfile("probdef"));
    $self->save_lmctl($self->tempfile("lmctl"));

    return 1;
}

sub evaluate {
    my ($self, $testset, $outprobs, $include_unks) = @_;

    # Do stuff with evallm
    my $lmfile = $self->tempfile("arpabo");
    my $contextfile = $self->tempfile("context");

    local $_;
    my $testfile;
    $testset = [$testset] unless ref $testset;
    $testfile = $self->{opts}{outtrans};
    $testfile = $self->tempfile("testset")
	unless defined $testfile;
    my $testfh = IO::File->new($testfile, ">:utf8")
	or die "Failed to open temporary file $testfile: $!";
    if (ref $testset eq 'ARRAY') {
	# An array ref of files to evaluate
	# Do some input filtering if need be
	if (defined($self->{inputfilter})) {
	    foreach my $transcript (@$testset) {
		my $tempfile = $self->tempfile(sprintf "%03d_%s",
					       ++$self->{transcount},
					       basename($transcript));
		$self->log_message("Filtering $transcript to $tempfile");
		$self->{inputfilter}->normalize_transcript($transcript, $tempfile);
		$transcript = $tempfile;
	    }
	}

	# Concatenate all input files
	foreach my $in (@$testset) {
	    my $infh = IO::File->new($in)
		or die "Failed to open $in: $!";
	    while (<$infh>) {
		# Handle class tags if present
		s,<unmarked>\s*([^<]+?)\s*</unmarked>,$1,g;
		s,<([^sS>]|[^>][^>]+)>\s*([^<]+?)\s*</\1>,[$1],g;
		# Remove utterance IDs
		s,\([^\)\(]+\)$,,;
		print $testfh $_;
	    }
	}
    }
    else {
	# Assume that it is a filehandle
	while (<$testset>) {
	    # Handle class tags if present
	    s,<unmarked>\s*([^<]+?)\s*</unmarked>,$1,g;
	    s,<([^sS>]|[^>][^>]+)>\s*([^<]+?)\s*</\1>,[$1],g;
	    # Remove utterance IDs
	    s,\([^\)\(]+\)$,,;
	    print $testfh $_;
	}
    }

    $self->save_context($contextfile);
    my ($rfh, $wfh);
    my $evallm = defined($self->{opts}{bindir})
      ? catfile($self->{opts}{bindir}, 'evallm')
 	: 'evallm';
    $self->log_message("Running $evallm -arpa $lmfile -context $contextfile");
    my $pid = open3($wfh, $rfh, undef, $evallm, -arpa => $lmfile, -context => $contextfile)
	or die "Failed to open2: $! $?";
    my $cmd = "perplexity -text $testfile";
    if (defined($outprobs)) {
	$cmd .= " -probs $outprobs";
    }
    if ($include_unks) {
	$cmd .= " -include_unks";
    }
    $self->log_message("evallm: $cmd");
    print $wfh $cmd, "\n";
    close $wfh;
    local $_;
    my ($pplx, $oov, @hit);
    while (<$rfh>) {
	print if $self->{opts}{verbose};
	/Perplexity = ([^,]+)/ and $pplx = $1;
	/\d+ OOVs \(([^\)]+)\)/ and $oov = $1;
	/(\d+)-grams hit = \S+\s+\(([^\)]+)\)/ and $hit[$1] = $2;
    }
    close $rfh;
    return bless [$pplx, $oov, @hit], 'Text::CMU::NGramModel::Evaluation';
}

sub evaluate_sentence {
    my ($self, $sentence) = @_;

    # Write sentence to a temporary file
    my $sfile = $self->tempfile("sentence");
    my $fh = IO::File->new($sfile, ">:utf8")
	or die "Failed to open $sfile: $!";
    chomp($sentence);
    $fh->print($sentence, "\n");
    $fh->close();

    return $self->evaluate($sfile);
}

sub interpolate {
    my ($self, $heldout, $lm1, $lm2) = @_;

    my $lm1probs = $self->tempfile("lm1.fprobs");
    my $lm2probs = $self->tempfile("lm2.fprobs");

    # Evaluate heldout with both LMs
    $lm1->evaluate($heldout, $lm1probs, 1); # include unks
    $lm2->evaluate($heldout, $lm2probs, 1); # include unks

    # Run interpolate to find weights
    my $lambdas = $self->tempfile("lambdas");
    my $logfile = $self->tempfile("interpolate.log");
    $self->run('lm_interpolate', undef, undef, $logfile,
	       [ '+', $lm1probs,
		 '+', $lm2probs,
		 -out_lambdas => $lambdas ]);

    # Look through the logfile to get the final perplexity
    my $fh = IO::File->new($logfile, "<:utf8") or die "Failed to open $logfile: $!";
    local $_;
    my $pplx;
    while (<$fh>) {
	/TOTAL PP = (\S+)/ and $pplx = $1;
    }

    # Run lm_combine to generate new LM for $self
    $self->run('lm_combine', undef, undef, undef,
	       [ -lm1 => $lm1->tempfile("arpabo"),
		 -lm2 => $lm2->tempfile("arpabo"),
		 -lm => $self->tempfile("arpabo"),
		 -weight => $lambdas ]);

    return $pplx;
}

sub name {
    my $self = shift;
    return exists($self->{opts}{name})
	? $self->{opts}{name} : $self->tempfile("arpabo");
}

sub load {
    my ($self, $file) = @_;
    $self->{files}{arpabo} = $file;
}

sub save {
    my ($self, $file) = @_;

    # Copy the generated LM to the destination file
    copy($self->tempfile("arpabo"), $file)
	or die "Failed to copy LM to $file: $!";
}

sub save_transcripts {
    my ($self, $file) = @_;

    # Copy the (filtered) transcripts to the destination file
    my $outfh = ref($file) ? $file : IO::File->new($file, ">:utf8");
    die "Failed to open $file: $!" unless defined $file;
    local $_;
    foreach my $in (@{$self->{transcripts}}) {
	my $infh = IO::File->new($in, "<:utf8")
	    or die "Failed to open $in: $!";
	while (<$infh>) {
	    print $outfh $_
		or die "Failed to write to child: $!";
	}
    }
}

sub save_context {
    my ($self, $contextfile) = @_;
    my $fh = ref($contextfile) ? $contextfile : IO::File->new($contextfile, ">:utf8");
    die "Failed to open $contextfile: $!" unless defined($fh);
    if (defined($self->{opts}{context})) {
	$fh->print("$_\n") foreach @{$self->{opts}{context}};
    }
    $fh->close();
}

sub save_probdef {
    my ($self, $file) = @_;

    my $outfh = ref($file) ? $file : IO::File->new($file, ">:utf8");
    die "Failed to open $file: $!" unless defined $file;
    my $classes = $self->{class};

    foreach my $class (keys %$classes) {
	$outfh->print("LMCLASS [$class]\n");
	my $words = $classes->{$class};
	my $norm = 0;
	foreach my $count (values %$words) {
	    $norm += $count;
	}
	while (my ($word, $count) = each %$words)  {
	    # FIXME: Smoothing might be nice here
	    my $prob = $count / $norm;
	    $outfh->print("$word:$class\t\t$prob\n");
	}
	$outfh->print("END [$class]\n\n");
    }
}

sub save_lmctl {
    my ($self, $file, $lmfile, $probdeffile) = @_;

    my $outfh = ref($file) ? $file : IO::File->new($file, ">:utf8");
    die "Failed to open $file: $!" unless defined $file;

    $probdeffile = $self->tempfile("probdef")
	unless defined $probdeffile;
    $lmfile = $self->tempfile("arpabo")
	unless defined $lmfile;
    $outfh->print("{ $probdeffile }\n");
    my @classes = map "[$_]", keys %{$self->{class}};
    $outfh->print("$lmfile default { @classes }\n");
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
