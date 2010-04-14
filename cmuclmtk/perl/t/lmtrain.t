#!/usr/bin/perl -w
use strict;
use Text::CMU::NGramFactory;
use Test::Simple tests => 6;

ok(my $factory = Text::CMU::NGramFactory->new(bindir => "../src/programs"));
ok(my $ng = $factory->train("t/test.xml"));
ok($ng->save("t/cmu2.test.arpa"));
ok(my $results = $ng->evaluate_sentence("THIS IS A TEST"));
ok(abs($results->perplexity() - 51.11) < 0.1);
ok($results->oov() eq '25.00%');

1;
