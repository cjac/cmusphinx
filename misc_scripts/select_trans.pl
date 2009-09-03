#!/usr/bin/perl -w
use strict;
use File::Basename;

die "Usage: $0 CONTROL TRANSCRIPTS...\n" unless @ARGV >= 2;

my $ctlfn = shift;
open CONTROL, "<$ctlfn" or die "Failed to open $ctlfn: $!";
my (%utts, @utts);
while (<CONTROL>) {
    chomp;
    s/\r$//;
    my @fields = split;
    my $uttid = basename($fields[-1]);
    $utts{$uttid} = undef;
    push @utts, $uttid;
}

while (<>) {
    my ($uttid) = (/\(([^()]+)\)$/);
    if (exists $utts{$uttid}) {
	$utts{$uttid} = $_;
    }
}

foreach my $uttid (@utts) {
    die "Utterance $uttid not found in transcripts!"
	unless defined($utts{$uttid});
    print $utts{$uttid};
}
