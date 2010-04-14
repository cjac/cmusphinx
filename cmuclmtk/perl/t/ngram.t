#!/usr/bin/perl -w
use strict;
use Text::CMU::NGramModel;
use Text::CMU::Vocabulary;
use Text::CMU::Smoothing;
use IO::File;
use Test::Simple tests => 14;

my $vocab = Text::CMU::Vocabulary->new(transcript => "t/cmu.test.lsn");
ok(my $ng = Text::CMU::NGramModel->new(vocabulary => $vocab, n => 3,
				       open => 1, bindir => "../src/programs"));
ok($ng->add_transcript("t/cmu.test.lsn"));
ok($ng->estimate());
ok($ng->save("t/cmu.test.arpa"));
ok(my $results = $ng->evaluate_sentence("THIS IS A TEST"));
ok(abs($results->perplexity() - 81.84) < 0.1);
ok($results->oov() eq '25.00%');
ok($results = $ng->evaluate("t/cmu.test.lsn"));
ok(abs($results->perplexity() - 15.99) < 0.1); # Ridiculously low as we'd expect
ok($results->oov() eq '0.00%');
ok($results = $ng->evaluate(["t/cmu.test.lsn"]));
ok(abs($results->perplexity() - 15.99) < 0.1); # Ridiculously low as we'd expect
my $fh = IO::File->new("t/cmu.test.lsn", "<:utf8");
ok($results = $ng->evaluate($fh));
ok(abs($results->perplexity() - 15.99) < 0.1); # Ridiculously low as we'd expect
