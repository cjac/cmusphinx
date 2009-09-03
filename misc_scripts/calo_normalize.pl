#!/usr/local/bin/perl58 -w
# -*- cperl -*-
use strict;
use File::Spec::Functions;
use Getopt::Long;
use XML::Parser;

Getopt::Long::Configure('bundling');
my %optarg =
    (
     'help|h|?'     => 'Show this message',
     'verbose|v'    => 'Describe what is going on',
     'outfile|o=s'  => 'Output file (TRS format) for all transcripts',
     'outstm|m=s'   => 'Ouptut file (STM format) for all transcripts',
     'outctl|c=s'   => 'Sphinx training/decoding control file for output',
     'fps|r=s'      => 'Frame rate (frames/second) for STM and control file',
     'partials|p'	=> 'Include partial words',
     'filledpauses|P'	=> 'Treat filled pauses as words rather than noise',
     'uttered|u'	=> 'Use "uttered" rather than "intended" in mispronounces',
     'noisy|E'		=> 'Include noise-only utterances (fillers/silence only)',
     'compound'		=> 'Include compound words as marked'
    );

sub usage {
    print "Usage: $0 [OPTIONS] INPUT\n";
    print "Options:\n";
    foreach my $opt (sort keys %optarg) {
	my $oname = $opt;
	$oname =~ s/=.*$//;
	printf "  --%-20s %s\n", $oname, $optarg{$opt};
    }

    exit 1;
}


my %opts;
GetOptions(\%opts, keys %optarg)
    or usage();
usage() if $opts{help};
$opts{transext} = 'lsn' unless defined($opts{transext});
print "Must give --fps with --outctl\n" and usage()
    if defined($opts{outctl}) and !defined($opts{fps});

my ($recdir, @meetings) = @ARGV;
my %meetings = map {$_ => 1} @meetings;
usage() unless defined($recdir);

my $parser = XML::Parser->new(Style => 'Subs',
			      Handlers => { Char => \&utt_text });

if (defined($opts{outfile})) {
    open OUTFILE, ">$opts{outfile}" or die "Failed to open $opts{outfile}: $!";
}
else {
    open OUTFILE, ">/dev/null";
}
if (defined($opts{outctl})) {
    open OUTCTL, ">$opts{outctl}" or die "Failed to open $opts{outctl}: $!";
}
else {
    open OUTCTL, ">/dev/null";
}

my %fillermap = split " ", <<EOF;
/#h/		++BREATH++
/h#/		++BREATH++
/h3/		++BREATH++
/H#/		++BREATH++
/cg/		++COUGH++
/lg/		++LAUGH++
/ls/		++SMACK++
/noise/		++NOISE++
/feed/		++GARBAGE++
/click/		++CLICK++
/hangup/	++CLICK++
#noise#		++NOISE++
#cut-in#	++NOISE++
/crosstalk/	++GARBAGE++
/background/	++NOISE++
\*pause\*	<SIL>
/ignore/	++NOISE++
/mumble/	++GARBAGE++
/los/		++SMACK++
/clap/		++NOISE++
/sneeze/	++NOISE++
/yawn/		++NOISE++
/whoosh/	++BREATH++
/ah/		++UH++
/eh/		++UH++
/um/		++UM++
/uh/		++UH++
/Um/		++UM++
/Uh/		++UH++
/mm/		++UM++
/hm/		++UM++
/hmm/		++UM++
/mhm/		++UM++
/er/		++UH++
/aa/		++UH++
/ix/		++UH++
/ax/		++UH++
/huh/		++HUH++
/heh/		++HUH++
/hah/		++HUH++
/oh/		OH
/bah/		BAH
/ooh/		OOH
/e/		E-
/u/		U-
/d/		D-
/a/		A-
/th/		TH-
/w/		W-
/l/		L-
/i/		I-
/ih/		I-
/n/		N-
/o/		O-
/r/		R-
/m/		M-
/t/		T-
/sh/		SH-
/g/		G-
/k/		K-
/p/		P-
/v/		V-
/y/		Y-
/f/		F-
/s/		S-
/b/		B-
/aw/		AW-
/hh/		H-
/ha/		HA-
/ng/		NG-
/wh/		WH-
/ee/		E-
/ba/		BA-
/ae/		AE-
/oy/		OY-
/ow/		OW-
/dd/		D-
/lh/		L-
/ch/		CH-
/uw/		OO-
/bd/		B-
/z/		Z-
/zh/		ZH-
/st/		ST-
/tt/		T-
/cr/		CR-
/kd/		K-
/ey/		EY-
/h/		H-
/so/		SO-
/ai/		EY-
/oo/		OO-
/fr/		FR-
/du/		DO-
/co/		CO-
/td/		T-
/dude/		DUDE
/yah's/		YEAHS
/ah's/		AHS
/oh's/		OHS
EOF

use vars qw(@STMS);
my (@all_stms);
foreach my $transfile (@ARGV) {
    print STDERR "Parsing transcription from $transfile\n" if $opts{verbose};
    @STMS=();
    $parser->parsefile($transfile);
    push @all_stms, @STMS;
}

if ($opts{outstm}) {
    open OUTSTM, ">$opts{outstm}" or die "Failed to open $opts{outstm}: $!";
    write_STMS(\*OUTSTM, \@all_stms);
    close OUTSTM;
}

close OUTCTL;
close OUTFILE;
exit 0;

sub write_STMS {
    my ($file, $stms) = @_;

    # Sort the STM lines (as required by the format)
    my @stms = sort {
	$$a[0] cmp $$b[0] or
	    $$a[1] cmp $$b[1] or
		$$a[3] <=> $$b[3]
	    } @$stms;
    foreach (@stms) {
	print $file "@$_\n";
    }
}

sub normalize_utt {
    my ($file, $start, $end, $text) = @_;
    my $uttid = "${file}_${start}-${end}";

    # This appears somewhere
    if ($text =~ /#+\s+stopped checking\s+#+/) {
	return;
    }

    # Separate fillers from words
    foreach my $f (keys %fillermap) {
	my $p = quotemeta($f);
	$text =~ s/$p/ $f /g;
    }

    # Fix broken fillers
    $text =~ s{/((?:[^/\s]+\s+)+)([^/\s]+)/}
	{ my ($f, $l) = ($1, $2);
	  $f =~ s:\s+:/ /:g;
	  "/$f$l/" }ge;

    # Remove #markers#
    $text =~ s/#[^# ]+#//g;

    # Include falsestarts
    # ARRRGHGGG!  Sometimes these have [] inside them to indicate
    # the intent of a partial word.  Transform that into [foo(bar)].
    $text =~ s/<\s*([^>]+)\s*\[([^\]]+)\]\s*>/ [$1 ($2)] /g;
    $text =~ tr/<>/  /;

    # Even if the transcriber was unsure, we still need that text!
    $text =~ s/\(\(/ /g;
    $text =~ s/\)\)/ /g;

    # Remove punctuatoin
    $text =~ tr/.,:;?!"//d;

    # Transcribers sometimes type {} instead of [].  Thankfully none
    # of our markup uses curly braces.
    $text =~ tr/{}/[]/;

    # Transcribers sometimes type (foo[bar]) instead of [foo(bar)].
    $text =~ s/\(([^[]+)\[([^]]+)\]\)/[$1($2)]/g;

    # Split apart mispronounced segments so we can use regexes on them
    $text =~ s/(\[[^]]+\])/my $foo=$1; $foo =~ s,\)\s+(?!]),)] [,g; $foo/ge;

    # Deal with mispronunciations/partial words
    if ($opts{uttered}) {
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

    # Normalize compound words
    $text =~ s/~([A-Za-z]+)/ join '_', split '', $1 /ge;
    $text =~ s/([A-Za-z])\.(?=[A-Za-z])/$1_/g;
    $text =~ s/_\b//g;

    # Split compound words unless otherwize requested
    unless ($opts{compound}) {
	$text =~ s/[_-](\S)/ $1/g;
    }

    # Now do filler mapping
    my @words = split ' ', $text;
    my @outwords;
    foreach my $word (@words) {
	my $nodash = $word;
	$nodash =~ s/-$//;
	$word = $nodash if exists $fillermap{$nodash};
	$word = lc$word if exists $fillermap{lc$word};

	if (exists $fillermap{$word}) {
	    my $outword = $fillermap{$word};

	    if ($outword =~ /-$/) {
		push @outwords, uc$outword if $opts{partials};
	    }
	    elsif ($outword !~ tr/-+//) {
		push @outwords, uc$outword if $opts{filledpauses};
	    }
	    else {
		push @outwords, $outword unless $opts{lm} or $opts{stm};
	    }
	}
	elsif ($word =~ /-$/) {
	    push @outwords, uc$word if $opts{partials};
	}
	elsif ($word =~ m,^/.*/$, or $word =~ /^#.*#$/) {
	    warn "Unexpected filler word $word in $file";
	    push @outwords, "++GARBAGE++" unless $opts{lm} or $opts{stm};
	}
	elsif ($word eq '-') {
	    # Special case ... skip these
	}
	else {
	    push @outwords, uc $word;
	}
    }

    # Skip empty utterances
    return unless @outwords;

    # Skip noise-only utterances unless requested not to
    unless ($opts{noisy}) {
	my $word;
	foreach my $w (@outwords) {
	    if ($w !~ /^\+\+.*\+\+$/) {
		$word = $w;
		last;
	    }
	}
	return unless defined $word;
    }

    if (defined($opts{outctl})) {
	printf OUTCTL "%s %d %d %s\n",
	    $file, $start*$opts{fps}, $end*$opts{fps}, $uttid;
    }
    if ($opts{outfile}) {
	print OUTFILE "<s> @outwords </s> ($uttid)\n";
    }
    if ($opts{outstm}) {
	my ($channel, $speaker);
	if ($file =~ /^(\D+)_([-\d_]+)/) {
	    $speaker = $1;
	    $channel = $2;
	}
	else {
	    ($channel, $speaker) = ($file =~ /^([\d_]+)(?:_(\D*))?$/);
	    $speaker = "unknown" unless defined $speaker;
	}
	$speaker =~ tr/ /_/;
	my @stmwords = @outwords;
	foreach (@stmwords) {
	    if (/^\+\+([^+]+)\+\+$/) {
		$_ = "{ $1 / @ }";
	    }
	}
	push @STMS, [$file, $channel, $speaker, $start, $end, @stmwords];
    }
}

# XML parser business follows...
# Variables shared between these three XML handlers
{
    my ($file, $sync, $text);
    sub Trans {
	my ($expat, $tag, %attr) = @_;
	$file = $attr{audio_filename};
	$sync = 0;
	$text = undef;
    }

    sub Sync {
	my ($expat, $tag, %attr) = @_;
	if (defined($text)) {
	    normalize_utt($file, $sync, $attr{time}, $text);
	    $text = undef;
	}
	# Ignore bogus sync values
	$sync = $attr{time} unless $attr{time} < $sync;
    }

    sub utt_text {
	my ($expat, $data) = @_;
	return if $data =~ /^\s*$/;
	$text .= $data;
    }
}

__END__
