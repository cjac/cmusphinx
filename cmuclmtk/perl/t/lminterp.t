#!/usr/bin/perl -w
use strict;
use NGramFactory;
use Test::Simple tests => 6;

ok(my $factory = NGramFactory->new(bindir => "../src"));
ok(my $ng = $factory->train("t/test_interp.xml"));
ok($ng->save("t/interp.test.arpa"));
ok(my $results = $ng->evaluate_sentence("THIS IS A TEST"));
ok(abs($results->perplexity() - 39.88) < 0.1);
ok($results->oov() eq '0.00%');

1;
