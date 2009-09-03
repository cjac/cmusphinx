#!/usr/bin/perl -w
use strict;

die "Usage: $0 CONTROL TRANSCRIPTS...\n" unless @ARGV >= 2;

my $ctlfn = shift;
open CONTROL, "<$ctlfn" or die "Failed to open $ctlfn: $!";
my (%utts, @utts);
while (<CONTROL>) {
    chomp;
    s/\r$//;
    my @fields = split;
    my $uttid = $fields[-1];
    $utts{$uttid} = \@fields;
    push @utts, $uttid;
}

while (<>) {
    my ($sent, $uttid) = (/^(.*) *\(([^)]+)\)$/);
    $uttid =~ s,.*/,,;
    if (exists $utts{$uttid}) {
	print "@{$utts{$uttid}}\n";
    }
}
