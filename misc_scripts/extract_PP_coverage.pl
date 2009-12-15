#!/usr/bin/env perl
#
# Extract PP and n-gram coverage from a evallm->perplexity output
# Emit tab-delimited line
# [20091019] (air)
#

open(IN,"$ARGV[0]") or die " can't open $ARGV[0]\n";
while (<IN>) {

    if ( $_ =~ /^Perplexity =/ ) {
	@line = split /[, ]+/;
	$pp = $line[2];
	<IN>;  # skip word-count line...
	$_ = <IN>; ($tg) = ($_ =~ /\((.+?)%\)/);
	$_ = <IN>; ($bg) = ($_ =~ /\((.+?)%\)/);
	$_ = <IN>; ($ug) = ($_ =~ /\((.+?)%\)/);

	print "$pp\t$tg\t$bg\t$ug\n";
    }
    
}

#

