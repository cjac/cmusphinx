#!/usr/local/bin/perl

use strict;
use Cwd qw(abs_path cwd);

die "Usage: $0 <test name (e.g. dailytestMON)> <test type (accuracy|speed)>\n" if ($#ARGV < 1);

my $test_name = shift;
my $test_type = shift;

my $regression_dir = cwd;

my $dependence = "";

my $local_regression_log = "$regression_dir/local_regression.log";

open (REGRESSION, ">$local_regression_log") or exit -1;

my $s3_root = abs_path("$regression_dir/../../..");

my $qsub;

# Compile
my $compilation = system("cd $s3_root; ./autogen.sh && ./autogen.sh && make");

if ($compilation) {
  # Error in compilation
  print REGRESSION "compile|failed";
} else {
  print REGRESSION "compile|OK";

  # For each line in the test file, run it
  if ($test_type =~ m/^speed$/i) {
    $qsub = "qsub -q s4";
  } elsif ($test_type =~ m/^accuracy$/i) {
    $qsub = "qsub";
  } else {
    $qsub = "";
    warn "No test type defined. Assuming local test";
  }
  if (open (CTL, "<$test_name")) {
    while (<CTL>) {
      my ($test_dir, $test_target) = split /\s+/;
      chdir "$s3_root/$test_dir";
      my $current_time = time();
      my $temp_script = "tmp_script$current_time";
      open (TEMP, ">$temp_script") 
	or (
	    warn "Could not create temp file at $test_dir\n" and
	    next
	   );
      print TEMP "#\n";
      print TEMP "cd $s3_root/$test_dir\n";
      print TEMP "make $test_target\n";
      close(TEMP);
      my $job = `$qsub $temp_script`;
      if ($qsub ne "") {
	$job =~ s/\..*//;
	$dependence .= ":" . "$job";
      }
      unlink $temp_script;
    }
    close(CTL);
  }
}

close(REGRESSION);
my $job_id = "";
if ($dependence eq "") {
  system("perl regression_report.pl $local_regression_log $test_name");
} else {
  my $regression_script = "$regression_dir/regression.pl";
  open (REPORT, ">$regression_script") or exit (-1);
  print REGRESSION "#\n";
  print REGRESSION "cd $regression_dir\n";
  print REGRESSION "perl regression_report.pl $local_regression_log $test_name";
  close(REGRESSION);
  $job_id = `$qsub -W:afterany$dependence $regression_script`;
}

print "$job_id";

exit (0);

