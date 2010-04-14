#!/usr/bin/perl -w
use strict;
use Test::Simple tests => 3;
use Text::CMU::InputFilter::CMU;

ok(my $cmu = Text::CMU::InputFilter::CMU->new(fillermap => 't/cmu.test.fillermap',
				   partials => 1,
				   upper => 1,
				   feed => 1,
				   falsestarts => 1,
				   crosstalk => 1,
				   uttered => 1,
				   split => 1,
				   filledpauses => 1));
ok($cmu->normalize_transcript('t/cmu.test.trs', 't/cmu.test.lsn.test'));
ok(system('diff', '-q', 't/cmu.test.lsn', 't/cmu.test.lsn.test') == 0);

1;
