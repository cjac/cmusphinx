#!/usr/local/bin/perl

# Import some important packages
use strict;
use File::Path;

die "Usage: $0 <directory to remove>\n" if ($#ARGV < 0);

my $topDir = shift;

# Remove the temporary regression test
rmtree($topDir, 0, 1);
