#!/usr/bin/perl

while(<STDIN>) {
	if ( !/^\[/ ) {next;}
	chop;
	s/\[//g;
	s/\]//g;
	print "$_\n";
}
