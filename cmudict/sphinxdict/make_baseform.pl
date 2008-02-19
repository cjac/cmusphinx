#!/usr/local/bin/perl -w
# [20050309] (air)
# strip out stress marks from a cmudict
# this creates a "SphinxPhones_40" dictionary
#

if ( scalar @ARGV ne 2 ) { die "usage: make_baseform <input> <output>\n"; }

open(IN, $ARGV[0]) || die "can't open $ARGV[0] for reading!\n";
open(OUT,">$ARGV[1]") || die "can't open $ARGV[1] for writing!\n";

%dict = (); # words end up in here
%histo = ();  # some statistics on variants

get_dict(\%dict,IN);  # process the entries

# print out each entry
foreach $w (sort keys %dict) {
  $var=0;
  foreach $p (keys %{$dict{$w}}) {
    if ($var eq 0) {
      print  OUT "$w\t$p\n";
      $var++;
    }  else {
      print  OUT "$w($var)\t$p\n";
      $var++;
    }
  }
}

close(IN);
close(OUT);

#
#
# read in a dictionary
sub get_dict {
  my $dict = shift;  # data structure with dictionary entries
  my $target = shift;  # input file

  while (<$target>) {
    chomp;
    ($word,$pron) = /(.+?)\s+(.+?)$/;
    if (! defined $word) { print STDERR "$_\n"; next; }
    if ($word =~ /\)$/) { # variant
      ($root,$variant) = ($word =~ m/(.+?)\((.+?)\)/);
    } else {
      $root = $word;
      $variant = 0;
    }
    $pron = &strip_stress($pron);
    if ($dict->{$root}{$pron}) {  # remove duplicate entries
      print STDERR "duplicate entry: $root ($variant) $pron\n";
    } else {
      $dict->{$root}{$pron} = $variant; # note the variant index
    }
    $histo{$variant}++;
  }
}

# strip stress marks from phonetic symbols
sub strip_stress {
  @pron = split " ", $_[0];
  my $p;
  foreach $p (@pron) { if ( $p =~ /\d$/) { $p =~ s/(\d+)$//; } }
  return ( join(" ",@pron));
}

#
