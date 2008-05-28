#!/usr/bin/perl

while(<STDIN>) {
	chop;
	system("cat $_");
	system("echo ';\n\n'");
}
