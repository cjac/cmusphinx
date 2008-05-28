#!/usr/bin/perl

open(IN,$ARGV[0]) || die "Can't open $ARGV[0]: $!";
while(<IN>) {
	if ( /^;/ ) {
	    close(OUT);
	    next;
	}
	if ( /^\[/ ) {
	    /\[(.*)\]/;
	    $name= $1;
	    open(OUT,">$name.gra") || die "Can't open $name: $!";
	}

	print OUT "$_";
}
