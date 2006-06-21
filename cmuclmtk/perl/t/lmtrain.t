#!/usr/bin/perl -w
use strict;
use NGramFactory;
use Test::Simple tests => 6;

ok(my $factory = NGramFactory->new(bindir => "../src"));
ok(my $ng = $factory->train("t/test.xml"));
ok($ng->save("t/cmu2.test.arpa"));
ok(my $results = $ng->evaluate_sentence("THIS IS A TEST"));
ok(abs($results->perplexity() - 52.72) < 0.1);
ok($results->oov() eq '25.00%');

1;
