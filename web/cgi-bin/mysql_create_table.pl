#!/usr/bin/perl

use strict;
use DBI;

require "mysql_user.pl";

my $host = "mysql4-c";
my $database = "c1904_regression";
my ($user, $pw) = getUserPwd('admin');

my $dbh = DBI->connect("dbi:mysql::$host", 
		       $user, $pw, {PrintError=>0}) 
  or die "can't connect to database: $DBI::errstr\n";

my $create_database = qq{CREATE DATABASE IF NOT EXISTS c1904_regression};

$dbh->do($create_database) or die "Error creating table: $DBI::errstr";

$dbh->disconnect();

my $dbh = DBI->connect("dbi:mysql:$database:$host", 
		       $user, $pw, {PrintError=>0}) 
  or die "can't connect to database: $DBI::errstr\n";


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

my $create_test_table = qq{CREATE TABLE IF NOT EXISTS test (
  date DATE NOT NULL DEFAULT 0,
  time TIME NOT NULL DEFAULT 0,
  machine VARCHAR(128) NOT NULL DEFAULT \'\',
  sphinx VARCHAR(32) NOT NULL DEFAULT \'\',
  test_name VARCHAR(128) NOT NULL DEFAULT \'\',
  who VARCHAR(16),
  status VARCHAR(16),
  audio_time FLOAT,
  proc_time FLOAT,
  words INT,
  insertions INT,
  deletions INT,
  substitutions INT,
  sentences INT,
  correct_sentences INT,
  heap_size FLOAT,
  load_average FLOAT,
  PRIMARY KEY  (machine, sphinx, date, time, test_name)
) TYPE=MyISAM};

createTable($dbh, $create_test_table);

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

my $create_build_table = qq{CREATE TABLE IF NOT EXISTS build (
  date DATE NOT NULL DEFAULT 0,
  time TIME NOT NULL DEFAULT 0,
  machine VARCHAR(128) NOT NULL DEFAULT \'\',
  sphinx VARCHAR(32) NOT NULL DEFAULT \'\',
  who VARCHAR(16),
  status VARCHAR(16),
  build_time INT,
  test_time INT,
  test_name VARCHAR(128) NOT NULL DEFAULT \'\',
  PRIMARY KEY  (machine, sphinx, date, time, test_name)
) TYPE=MyISAM};

createTable($dbh, $create_build_table);

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

my $create_system_table = qq{CREATE TABLE IF NOT EXISTS system (
  machine VARCHAR(128) NOT NULL DEFAULT \'\',
  num_cpus int,
  cacheSize_kB int,
  clock_MHz int,
  memory_MB int,
  architecture varchar(16),
  OS varchar(16),
  PRIMARY KEY  (machine, OS)
) TYPE=MyISAM};

createTable($dbh, $create_system_table);

# Record "environment' - describes test environment
#  1) "environment"
#  2) date
#  3) machine
#  4) JAVA_HOME
#  5) JAVA_VERSION
#  6) uptime

my $create_environment_table = qq{CREATE TABLE IF NOT EXISTS environment (
  machine VARCHAR(128) NOT NULL DEFAULT \'\',
  date DATE NOT NULL DEFAULT 0,
  sphinx VARCHAR(32) NOT NULL DEFAULT \'\',
  compiler_home varchar(255),
  compiler_version varchar(32),
  uptime varchar(128),
  PRIMARY KEY  (machine, sphinx, date)
) TYPE=MyISAM};

createTable($dbh, $create_environment_table);

# Record "metrics" - describes the source tree environment
#  1) number of files
#  2) number of source code files
#  3) number of classes
#  4) number of lines of source code
#  5) number of packages

my $create_metrics_table = qq{CREATE TABLE IF NOT EXISTS metrics (
  machine VARCHAR(128) NOT NULL DEFAULT \'\',
  sphinx VARCHAR(32) NOT NULL DEFAULT \'\',
  date DATE NOT NULL DEFAULT 0,
  time TIME NOT NULL DEFAULT 0,
  files int,
  source_files int,
  classes int,
  code_lines int,
  packages int,
  PRIMARY KEY  (machine, sphinx, date, time)
) TYPE=MyISAM};

createTable($dbh, $create_metrics_table);

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

my $create_live_test_table = qq{CREATE TABLE IF NOT EXISTS live_test (
  date DATE NOT NULL DEFAULT 0,
  time TIME NOT NULL DEFAULT 0,
  machine VARCHAR(128) NOT NULL DEFAULT \'\',
  sphinx VARCHAR(32) NOT NULL DEFAULT \'\',
  test_name VARCHAR(128) NOT NULL DEFAULT \'\',
  who VARCHAR(16),
  status VARCHAR(16),
  audio_time FLOAT,
  proc_time FLOAT,
  words INT,
  insertions INT,
  deletions INT,
  substitutions INT,
  sentences INT,
  correct_sentences INT,
  heap_size FLOAT,
  load_average FLOAT,
  actualUtterances INT,
  foundUtterances INT,
  gapInsertions INT,
  averageResponseTime FLOAT,
  maxResponseTime FLOAT,
  minResponseTime FLOAT,
  PRIMARY KEY  (machine, sphinx, date, time, test_name)
) TYPE=MyISAM};

createTable($dbh, $create_live_test_table);

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

my $create_rejection_test_table = qq{CREATE TABLE IF NOT EXISTS rejection_test (
  date DATE NOT NULL DEFAULT 0,
  time TIME NOT NULL DEFAULT 0,
  machine VARCHAR(128) NOT NULL DEFAULT \'\',
  sphinx VARCHAR(32) NOT NULL DEFAULT \'\',
  test_name VARCHAR(128) NOT NULL DEFAULT \'\',
  who VARCHAR(16),
  status VARCHAR(16),
  audio_time FLOAT,
  proc_time FLOAT,
  words INT,
  insertions INT,
  deletions INT,
  substitutions INT,
  sentences INT,
  correct_sentences INT,
  heap_size FLOAT,
  load_average FLOAT,
  correctOutOfGrammar INT,
  falseOutOfGrammar INT,
  correctInGrammar INT,
  falseInGrammar INT,
  PRIMARY KEY  (machine, sphinx, date, time, test_name)
) TYPE=MyISAM};

createTable($dbh, $create_rejection_test_table);

$dbh->disconnect();

exit 0;

sub createTable {
  my $dbh = shift;
  my $create = shift;
  my $table = $create;
  $table =~ s/\n//g;
  $table =~ s/\s+\(.*$//;
  $table =~ s/^.*\s//;
# If you want to start from scratch, uncomment the following line
#  $dbh->do("drop table $table") or warn "Couldn't drop table: $DBI::errstr";
  $dbh->do($create) or die "Error creating table: $DBI::errstr";
}
