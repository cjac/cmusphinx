#!/usr/bin/perl

use strict;

if ($#ARGV < 0) {
  print "Usage: $0 <log directory>\n";
  exit;
}


my $logDir = shift;

printHeader();

my @logFiles = getLogFiles($logDir);

my %machines = getMachineNames(@logFiles);

# As much as I tried to avoid globals, I dont' know how to pass and retrieve a has to a subroutine.
our %testList;
our %allResults;

startRow();
printElement("");
foreach my $machine (sort keys %machines) {
  printTitleElement($machine);
  getResult($machine, $logDir, $machines{$machine});
}
endRow();

my %fail = 0;
my %pass = 0;
foreach my $test (sort keys %testList) {
  startRow();
  printTitleElement($test);
  foreach my $machine (sort keys %machines) {
    my $result = $allResults{$machine}{$test};
    if ($result =~ m/PASSED/) {
      printElement($result);
      $pass{$machine}++;
    } else {
      printFailedElement($result);
      $fail{$machine}++;
    }
  }
  endRow();
}

startRow();
printElement("Passed");
foreach my $machine (sort keys %machines) {
  printElement($pass{$machine});
}
endRow();

startRow();
printElement("Failed");
foreach my $machine (sort keys %machines) {
  if ($fail{$machine} > 0) {
    printFailedElement($fail{$machine});
  } else {
    printElement($fail{$machine});
  }
}
endRow();

printFooter();

exit;

sub getLogFiles {
  my $logDir = shift;
  my @dirList;
# List all directory items (files, directories) that end with ".log"
  if (opendir(DIR, $logDir)) {
    @dirList = grep /.*\.log$/, readdir DIR;
    closedir(DIR);
  }
  return @dirList;
}

sub getMachineNames {
  my %machineNames = ();
# Filter out the substring starting with "."
  while (my $dirItem = shift) {
    my $logFile = $dirItem;
    $dirItem =~ s/(.*)\..+/$1/;
    $machineNames{$dirItem} = $logFile;
  }
  return %machineNames;
}

sub getResult {
  my $machine = shift;
  my $logDir = shift;
  my $logFile = shift; 
  my $log = "$logDir/$logFile";
  if (open(LOG, "$log")) {
    while (<LOG>) {
      next unless m/PASS/ or m/FAIL/;
      m/(.*) test (.*)/;
      my $testName = $1;
      my $testResult = $2;
      $testList{$testName} = 1;
      $allResults{$machine}{$testName} = $testResult;
    }
    close(LOG);
  } else {
    warn("Could not open $log\n");
  }

}

# Process a plain text index file
# sub textIndex {
# my $extension = shift;
#   my $indexRoot = shift;
#   my @list = @ARGV;
#   my @tags = ();
#   my %hashOfTags = ();
#   my $indexOfTags = 0;
#   # Read all tags into an array
#   open (FILE, "<$indexRoot/$tagFile") or 
#     die "Can't open file $indexRoot/$tagFile\n";
#   while (<FILE>) {
#     s/\r\n/\n/;
#     chomp();
#     $hashOfTags{$_} = $indexOfTags;
#     $indexOfTags++;
#     push @tags, $_;
#   }
#   close FILE;
# 
#   # Find if the filename matches any of the tags
#   foreach $filename (@list) {
#     # lowercase the string to avoid duplicates
#     $filename =~ tr/A-Z/a-z/;
# 
#     # Split the name so that we have each component of the file path,
#     # that is, top directory, lower level directories, and base filename
#     @lineContent = split /[\/\\]/, $filename;
# 
#     # keep only the file name, last component in the path
#     $baseFilename = $lineContent[$#lineContent];
# 
#     # The highest possible index will be the number of tags.
#     $minIndex = $indexOfTags;
# 
#     # Find the correct index, that is, the correct partition to assign
#     # the file to. We do that by splitting the filename into substrings,
#     # and then comparing each substring to a list of possible tags. The
#     # tag with the lowest index will be the partition we assign the file
#     # to.
#     foreach $word (split /[-\._]/, $baseFilename) {
#       if (exists $hashOfTags{$word}) {
# 	$thisIndex = $hashOfTags{$word};
# 	if ($thisIndex < $minIndex) {
# 	  $minIndex = $thisIndex;
# 	}
#       }
#     }
# 
#     $idxFile = "$indexRoot/part" . "$minIndex" . ".idx";
# 
#     # Now process the right index file
#     open (FILE, "<$idxFile") or die "Can't open file $idxFile\n";
#     # Read each line
#     while (<FILE>) {
#       # Match against file name, with given extension
#       next unless  /${filename}.*${extension}/i;
#       # If we reach this point, there's a match
#       s/\r\n/\n/;
#       chomp();
#       $line = $_;
#       # print the match
#       printElement($line);
#     }
#     close(FILE);
#   }
# }


sub printHeader {
  print "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n";
  print "<html>\n";
  print "<head>\n";
  print "  <meta http-equiv=\"content-type\"\n";
  print " content=\"text/html; charset=ISO-8859-1\">\n";
  print "  <title>Summary of regression tests in the SF.net compile farm</title>\n";
  print "<!-- Created by " . $0 . " -->\n";
  print "</head>\n";
  print "<body>\n";
  print "<h1 style=\"text-align: center;\">Summary of regression tests in the SF.net compile farm</h1>\n";
  print "<br>\n";
  print "<table border=\"1\" width=\"100%\" cellpadding=\"2\" cellspacing=\"2\">\n";
  print "  <tbody>\n";
}

sub printTitleElement {
  my $element = shift;
  print "      <td><b>" . $element . "</b></td>\n";
}

sub printElement {
  my $element = shift;
  print "      <td>" . $element . "</td>\n";
}

sub printFailedElement {
  my $element = shift;
  print "      <td bgcolor=\"ffcccc\">" . $element . "</td>\n";
}

sub startRow {
  print "    <tr>\n";
}

sub endRow {
  print "    </tr>\n";
}

sub printFooter {
  print "  </tbody>\n";
  print "</table>\n";
  print "<br>\n";
  print "<br>\n";
  print "Notes<br>\n";
  print "<ul>\n";
  print "  <li>Last updated on " . localtime() . "</li>\n";
  print "</ul>\n";
  print "</body>\n";
  print "</html>\n";
}


