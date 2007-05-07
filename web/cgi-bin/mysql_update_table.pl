#!/usr/bin/perl

# Copyright 2007 Carnegie Mellon University.  
# All Rights Reserved.  Use is subject to license terms.
# 
# See the file "license.terms" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL 
# WARRANTIES.

use strict;
use DBI;

require "mysql_user.pl";

system("./mysql_create_table.pl");

my ($host, $database) = getServer();
my ($user, $pw) = getUserPwd('rw');

my ($machine, $sphinx, $date, $time) = ();

my $dbh = DBI->connect("dbi:mysql:$database:$host", 
		       $user, $pw, {PrintError=>0}) 
  or die "can't connect to database: $DBI::errstr\n";

foreach my $log (@ARGV) {
  if (open(LOG, "< $log")) {
    while (<LOG>) {
      chomp();
      next if m/^#/ or m/^\s*$/;
      s/\|s(\d)\|/|sphinx$1|/g;
      s/\|s3\.3\|/|sphinx3.x|/g;
      s/\|make//;
      my @fields = split /\|/;
      if ($fields[1] =~ m/^\d+-\d+-\d+$/) {
	if ($fields[2] =~ m/^\d+$/) {
	  my $temptime = scalar localtime($fields[2]);
	  $temptime =~ s/^.*(\d{2}:\d{2}:\d{2}).*$/$1/;
	  $fields[2] = $temptime;
	}
      }
      if ($fields[0] eq "build") {
	update_build(@fields);
      } elsif ($fields[0] eq "environment") {
	update_environment(@fields);
      } elsif ($fields[0] eq "live_test") {
	update_live_test(@fields);
      } elsif ($fields[0] eq "metrics") {
	update_metrics(@fields);
      } elsif ($fields[0] eq "rejection_test") {
	update_rejection_test(@fields);
      } elsif ($fields[0] eq "system") {
	update_system(@fields);
      } elsif ($fields[0] eq "test") {
	update_test(@fields);
      }
    }
    close(LOG);
  }
}

$dbh->disconnect();


sub update_test {
  my $type = shift;
  return if ($type ne "test");

# Record "test" - marks a regression test entry. 
#
# Field definitions: 
#   1) "test"
#   2) date
#   3) time
#   4) machine
#   5) recognition system
#   6) testName 
#   7) who 
#   8) status
#   9) audioTime 
#  10) procTime 
#  11) words 
#  12) insertions 
#  13) deletions 
#  14) substitutions 
#  15) sentences 
#  16) correctSentences 
#  17) heapSize 
#  18) loadAverage 

my $update_test_table = sprintf "REPLACE test set
  date = \'%s\',
  time = \'%s\',
  machine = \'%s\',
  sphinx = \'%s\',
  test_name = \'%s\',
  who = \'%s\',
  status = \'%s\',
  audio_time = \'%s\',
  proc_time = \'%s\',
  words = \'%s\',
  insertions = \'%s\',
  deletions = \'%s\',
  substitutions = \'%s\',
  sentences = \'%s\',
  correct_sentences = \'%s\',
  heap_size = \'%s\',
  load_average = \'%s\'", @_;

  $date = $_[0];
  $time = $_[1];
  $machine = $_[2];
  $sphinx = $_[3];

  updateTable($update_test_table);
}

sub update_build {
  my $type = shift;
  return if ($type ne "build");

# Record "build" - marks a system build.
#
# Field definitions: 
#   1) "build"
#   2) date
#   3) time
#   4) machine
#   5) who 
#   6) 'make' 
#   7) status
#   8) buildTime 
#   9) testTime 
#   10) testName

# In case the system is ommitted, assume it's sphinx4
  push @_, 'sphinx4';

my $update_build_table = sprintf "REPLACE build set
  date = \'%s\',
  time = \'%s\',
  machine = \'%s\',
  who = \'%s\',
  status = \'%s\',
  build_time = \'%s\',
  test_time = \'%s\',
  test_name = \'%s\',
  sphinx = \'%s\'", @_;

  $date = $_[0];
  $time = $_[1];
  $machine = $_[2];
  $sphinx = $_[8];

  updateTable($update_build_table);
}

sub update_system {
  my $type = shift;
  return if ($type ne "system");

# Record "system" - describes a test system
#
# Field definitions:
#   1) "system"
#   2) name
#   3) numCPUS
#   4) cacheSize (in kbytes)
#   5) clock (mhz)
#   6) memory (mbytes)
#   7) architecture
#   8) OS

# use 'psrinfo' and 'prtdiag' to get this info on solaris
#

# Check /proc/cpuinfo and /proc/meminfo on linux

my $update_system_table = sprintf "REPLACE system set
  machine = \'%s\',
  num_cpus = \'%s\',
  cacheSize_kB = \'%s\',
  clock_MHz = \'%s\',
  memory_MB = \'%s\',
  architecture = \'%s\',
  OS = \'%s\'", @_;

  $machine = $_[0];

  updateTable($update_system_table);
}

sub update_environment {
  my $type = shift;
  return if ($type ne "environment");

# Record "environment' - describes test environment
#  1) "environment"
#  2) date
#  3) machine
#  4) JAVA_HOME
#  5) JAVA_VERSION
#  6) uptime

# In case the system is ommitted, assume it's sphinx4
  push @_, 'sphinx4';

my $update_environment_table = sprintf "REPLACE environment set
  date = \'%s\',
  machine = \'%s\',
  compiler_home = \'%s\',
  compiler_version = \'%s\',
  uptime = \'%s\',
  sphinx = \'%s\'", @_;

  $date = $_[0];
  $machine = $_[1];
  $sphinx = $_[5];

  updateTable($update_environment_table);
}

sub update_metrics {
  my $type = shift;
  return if ($type ne "metrics");

# Record "metrics" - describes the source tree environment
#  1) number of files
#  2) number of source code files
#  3) number of classes
#  4) number of lines of source code
#  5) number of packages

  push @_, $machine;
  push @_, $sphinx;
  push @_, $time;

my $update_metrics_table = sprintf "REPLACE metrics set
  date = \'%s\',
  files = \'%s\',
  source_files = \'%s\',
  classes = \'%s\',
  code_lines = \'%s\',
  packages = \'%s\',
  machine = \'%s\',
  sphinx = \'%s\',
  time = \'%s\'", @_;

  updateTable($update_metrics_table);
}

sub update_live_test {
  my $type = shift;
  return if ($type ne "live_test");

# Record "live_test" - marks a regression test entry. 
#
# Field definitions: 
#   1) "live_test"
#   2) date
#   3) time
#   4) machine
#   5) recognition system
#   6) testName 
#   7) who 
#   8) status
#   9) audioTime 
#  10) procTime 
#  11) words 
#  12) insertions 
#  13) deletions 
#  14) substitutions 
#  15) sentences 
#  16) correctSentences 
#  17) heapSize 
#  18) loadAverage 
#  19) actualUtterances
#  20) foundUtterances
#  21) gapInsertions
#  22) averageResponseTime
#  23) maxResponseTime
#  24) minResponseTime

my $update_live_test_table = sprintf "REPLACE live_test set
  date = \'%s\',
  time = \'%s\',
  machine = \'%s\',
  sphinx = \'%s\',
  test_name = \'%s\',
  who = \'%s\',
  status = \'%s\',
  audio_time = \'%s\',
  proc_time = \'%s\',
  words = \'%s\',
  insertions = \'%s\',
  deletions = \'%s\',
  substitutions = \'%s\',
  sentences = \'%s\',
  correct_sentences = \'%s\',
  heap_size = \'%s\',
  load_average = \'%s\',
  actualUtterances = \'%s\',
  foundUtterances = \'%s\',
  gapInsertions = \'%s\',
  averageResponseTime = \'%s\',
  maxResponseTime = \'%s\',
  minResponseTime = \'%s\'", @_;

  $date = $_[0];
  $time = $_[1];
  $machine = $_[2];
  $sphinx = $_[3];

  updateTable($update_live_test_table);
}

sub update_rejection_test {
  my $type = shift;
  return if ($type ne "rejection_test");

# Record "rejection_test" - marks a regression test entry. 
#
# Field definitions: 
#   1) "rejection_test"
#   2) date
#   3) time
#   4) machine
#   5) recognition system
#   6) testName 
#   7) who 
#   8) status
#   9) audioTime 
#  10) procTime 
#  11) words 
#  12) insertions 
#  13) deletions 
#  14) substitutions 
#  15) sentences 
#  16) correctSentences 
#  17) heapSize 
#  18) loadAverage 
#  19) ""
#  20) ""
#  21) ""
#  22) ""
#  23) ""
#  24) ""
#  25) correctOutOfGrammar
#  26) falseOutOfGrammar
#  27) correctInGrammar
#  28) falseInGrammar

my $update_rejection_test_table = sprintf "REPLACE rejection_test set
  date = \'%s\',
  time = \'%s\',
  machine = \'%s\',
  sphinx = \'%s\',
  test_name = \'%s\',
  who = \'%s\',
  status = \'%s\',
  audio_time = \'%s\',
  proc_time = \'%s\',
  words = \'%s\',
  insertions = \'%s\',
  deletions = \'%s\',
  substitutions = \'%s\',
  sentences = \'%s\',
  correct_sentences = \'%s\',
  heap_size = \'%s\',
  load_average = \'%s\',
  %s%s%s%s%s%s
  correctOutOfGrammar = \'%s\',
  falseOutOfGrammar = \'%s\',
  correctInGrammar = \'%s\',
  falseInGrammar = \'%s\'", @_;

  $date = $_[0];
  $time = $_[1];
  $machine = $_[2];
  $sphinx = $_[3];

  updateTable($update_rejection_test_table);
}

sub updateTable {
  my $update = shift;
  $dbh->do($update) or die "Error updating table with command\n$update\n$DBI::errstr";
}
