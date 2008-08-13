#!/usr/bin/perl

use File::Spec;
use File::Path;
use File::Find;
use File::stat;
use Getopt::Long;

use strict;

my $PERL = 'perl';
my $LOGIOS_ROOT = File::Spec->rel2abs(File::Spec->updir);
my $RESOURCES = File::Spec->catdir($LOGIOS_ROOT, 'examples', 'Resources');
my $PROJECT = 'MeetingLineDomain';
my $INSTANCE = 'MeetingLine';

my $target = shift;
&clean; #clean by default
exit if $target eq 'clean';
&build;
&test;
print "Press ENTER to exit.$/"; <STDIN>;
exit;

sub wanted_cleaned {
  my $filename = $_;
  $File::Find::prune = 1 if $filename eq '.svn';
  return if ! -f $filename;

  foreach ('MeetingLineDomain.forms', 'MeetingLineDomain.gra', 'usernames.class') {
    return if $_ eq $filename;
  }

  unlink $filename;
}

sub clean {
  #clean everything that's _NOT_ on the whitelist
  print STDERR "Cleaning up Resources$/";
  find(\&wanted_cleaned, $RESOURCES);
}

sub build {
  print STDERR "Executing MakeLanguage$/";

  require File::Spec->catfile($LOGIOS_ROOT, 'scripts', 'Logios.pm');
  my $logios = Logios->new('OLYMODE' => 1,
                           'LOGIOS' => $LOGIOS_ROOT,
                           'PROJECT' => $PROJECT,
                           'RESOURCES' => $RESOURCES,
                           'INSTANCE' => $INSTANCE);
  $logios->compile_grammar;
  $logios->makelm;
  $logios->makedict;
}

sub test {
  print STDERR "Testing results$/";

  for my $target (map {File::Spec->catfile($RESOURCES, $_)}
      (File::Spec->catfile('Grammar', "$INSTANCE.net"),
       File::Spec->catfile('Grammar', 'forms'),
       File::Spec->catfile('DecoderConfig', 'Dictionary', "$INSTANCE.dict"),
       File::Spec->catfile('DecoderConfig', 'LanguageModel', "$INSTANCE.arpa"),
       File::Spec->catfile('DecoderConfig', 'LanguageModel', "$INSTANCE.ctl"),
       File::Spec->catfile('DecoderConfig', 'LanguageModel', "$INSTANCE.probdef"))) {
    if (-e $target) {
      print "$target: size -> ", stat($target)->size, $/;
    } else {
      print "$target: MISSING!$/";
    }
  }
}
