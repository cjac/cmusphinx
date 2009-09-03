#!/usr/bin/perl
use strict;
use File::Path;
use File::Spec::Functions;
use File::Basename;

my ($ctl, $outdir) = @ARGV;

open CTL, "<$ctl" or die "Failed to open $ctl: $!";
while (<CTL>) {
    chomp;
    my $d = catdir($outdir, dirname($_));
    mkpath($d);
}
