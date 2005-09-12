#!/usr/local/bin/perl

# Import some important packages
use Env qw(PATH HOME);
use strict;
use File::Path;
use Cwd;

die "Usage: $0 \\\n\t<test name (e.g. dailytestMON)> \\\n\t<test type (accuracy|speed)>\n" if ($#ARGV < 1);

my $test_name = shift;
my $test_type = shift;

# Get a unique label, the current time
my $thisTime = time();

# Define the regression directory in a default location, using the unique label
my $top_dir = "$HOME/project/SourceForge/regression/sphinx3$thisTime";

# Create the directory for the current regression test
mkpath($top_dir, 0, 0755);

#Keep track of the current directory
my $current_dir = cwd;

# Move to the directory where we'll run the regression test
chdir $top_dir or warn "Failed to cd $top_dir\n";

# Retrieve the code. 'scvs' is user dependent. An example is located in this
# directory, but the script should be located in your path
my $result = "";
if (system("scvs co sphinx3")) {
  chdir "sphinx3/src/tests/regression";

  # Run it
  $result = `perl regression_launch.pl $test_name $test_type`;
warn "$result\n";
  $result =~ s/\..*$//;
}

# Move back to where we started
chdir $current_dir;

my $remove_script = "remove.sh";
if (open (REMOVE, ">$remove_script")) {
  print REMOVE "#\n";
  print REMOVE "cd $current_dir\n";
  print REMOVE "perl regression_remove.pl $top_dir\n";
  close(REMOVE);
  if ($result eq "") {
    system("sh $remove_script");
  } else {
    system("qsub -W afterany:$result $remove_script");
  }
}
