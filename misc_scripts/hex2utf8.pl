#!/usr/bin/perl

use Encode;

while (<>) {
  my @inwords = split;
  my @outwords;
  foreach my $w (@inwords) {
    # Leave things that obviously aren't hex encodings of GB alone
    if (length($w) % 4 or $w =~ tr/0-9a-f//c) {
      push @outwords, $w;
    }
    else {
      push @outwords, encode_utf8(decode('gb2312', pack("H*", $w)));
    }
  }
  print "@outwords\n";
}
