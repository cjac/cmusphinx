#!/usr/bin/perl -w

open SYM, "<wsj_100k.dic.osym" or die "$!";

while (<SYM>) {
    ($w,$i) = split;
    $sym{$w} = $i;
}

while (<>) {
    chomp;
    my @words = split;
    foreach (@words) {
	$_ = "<UNK>" unless exists $sym{$_};
    }
    print "@words\n";
}
