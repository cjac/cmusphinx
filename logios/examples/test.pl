#!/usr/bin/perl

use File::Spec;
use File::Path;
use File::Find;
use File::stat;
use Getopt::Long;

use strict;

my $LOGIOS_ROOT = File::Spec->rel2abs(File::Spec->updir);
my $RESOURCES = File::Spec->catdir($LOGIOS_ROOT, 'examples', 'Resources');
my $PROJECT = 'MeetingLineDomain';
my $INSTANCE = 'MeetingLine';

my $target = shift;
&clean; #clean by default
exit if $target eq 'clean';
&build;
&build_make_pronunciation;
my $testnum = 0;
my $testpass = 0;
my $testfail = 0;
&test;
&test_make_pronunciation;
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
  print STDERR "Exercising Logios.pm$/";

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

sub build_make_pronunciation {
  print STDERR "Executing make_pronunciation.pl$/";

  chdir('Resources');
  my $cmd = "$^X \"".File::Spec->catfile($LOGIOS_ROOT, 'Tools', 'MakeDict',
                                         'make_pronunciation.pl').'"'
                    ." -tools \"".File::Spec->catdir($LOGIOS_ROOT, 'Tools').'"'
                    ." -dictdir \"".File::Spec->catdir($RESOURCES, 'DecoderConfig', 'Dictionary').'"'
                    ." -words $INSTANCE.token"
                    ." -dict $INSTANCE-make_pronunciation.dic";
  print "$cmd$/";
  system($cmd);
  chdir(File::Spec->updir);
}

sub test_make_pronunciation {
  print STDERR "Testing make_pronunciation$/";

  my $dicfn = File::Spec->catfile($RESOURCES, 'DecoderConfig', 'Dictionary',
                                  "$INSTANCE-make_pronunciation.dic");
  return if !&test_results((-e $dicfn && stat($dicfn)->size), "$dicfn is missing or empty");
  &test_duquesne($dicfn);
}

sub test {
  print STDERR "Testing targets$/";
  #check to see if all the files exist
  for my $target (map {File::Spec->catfile($RESOURCES, $_)}
      (File::Spec->catfile('Grammar', "$INSTANCE.net"),
       File::Spec->catfile('Grammar', 'forms'),
       File::Spec->catfile('DecoderConfig', 'Dictionary', "$INSTANCE.dict"),
       File::Spec->catfile('DecoderConfig', 'LanguageModel', "$INSTANCE.arpa"),
       File::Spec->catfile('DecoderConfig', 'LanguageModel', "$INSTANCE.ctl"),
       File::Spec->catfile('DecoderConfig', 'LanguageModel', "$INSTANCE.probdef"))) {
    &test_results((-e $target && stat($target)->size),
                  (File::Spec->splitpath($target))[2], 
                  'missing or empty');
  }
  &test_duquesne(File::Spec->catfile($RESOURCES, 'DecoderConfig', 'Dictionary', 
                                     "$INSTANCE.dict"));
}

sub test_duquesne {
  open(DICT, shift);
  my @duq_entries = grep(/^DUQUESNE:/, <DICT>);
  if(&test_results((scalar @duq_entries == 1), 
                   "DUQUESNE entry doesn't exist in dictionary")) {
    chop(my $duq_entry = $duq_entries[0]);
    my $duq_pron = substr($duq_entry, index($duq_entry, "\t")+1);
    &test_results(($duq_pron eq 'D UW K EY N'), 
                  "DUQUESNE is '$duq_pron', should be 'D UW K EY N'");
  }
}

sub test_results {
  my ($status, @reasons) = @_;

  
  if($status) {
    ++$testpass;
    print sprintf("Test %2d: OK$/", ++$testnum);
  } else {
    ++$testfail;
    print sprintf("Test %2d: FAILED: ", ++$testnum), join(': ', @reasons), $/;
  }

  return $status;
}
