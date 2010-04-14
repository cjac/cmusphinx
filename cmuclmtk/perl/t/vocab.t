#!/usr/bin/perl -w
use strict;
use Text::CMU::Vocabulary;
use Test::Simple tests => 8;

ok(my $vocab = Text::CMU::Vocabulary->new());
$vocab->add_words("HELLO");
$vocab->add_words(qw(HELLO WORLD));
ok($vocab->count("HELLO") == 2);
my @words;
ok(@words = $vocab->words(-gt => 1));
ok($words[0] eq 'HELLO');
ok(@words = $vocab->words(-top => 1));
ok($words[0] eq 'HELLO');

$vocab = Text::CMU::Vocabulary->new();
$vocab->add_transcript("t/cmu.test.lsn");
$vocab->save_words("t/cmu.test.vocab.test");
# Should be no differences except in the comments
ok(system("diff -u t/cmu.test.vocab t/cmu.test.vocab.test | grep -q '^[+-][^-+#]'"));
$vocab->load_words("t/cmu.test.vocab.test");
$vocab->save_words("t/cmu.test.vocab.test2");
# Should be no differences except in the comments
ok(system("diff -u t/cmu.test.vocab.test t/cmu.test.vocab.test2 | grep -q '^[+-][^-+#]'"));
unlink("t/cmu.test.vocab.test2");
