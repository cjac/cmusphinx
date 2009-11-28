#!/usr/bin/perl -w
use strict;
use File::Spec;

die "Usage:$0 CTL PHSEGDIR\n" unless @ARGV == 2;
my ($ctl, $phsegdir) = @ARGV;

open CTL, "<$ctl" or die "Failed to open $ctl:$!";
while (<CTL>) {
    chomp;
    my $uttid;
    if (/ /) {
	(undef, undef, undef, $uttid) = split;
    }
    else {
	$uttid = $_;
    }
    my $phseg = File::Spec->catfile($phsegdir, "$uttid.phseg");
    open PHSEG, "<", $phseg or next;
    <PHSEG>; # Skip first line
    my @ptrans;
    while (<PHSEG>) {
      my ($sf, $ef, $ascr, @phones) = split;
      last if $sf eq 'Total';
      push @ptrans, $phones[0]; # CI Phone is first
    }
    print "@ptrans ($uttid)\n";
}
