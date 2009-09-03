#!/usr/bin/perl -w
use strict;

# Feed this a control file to get information on speakers, meetings
my (%meetings, %speakers);
while (<>) {
    my ($basename, $sf, $ef, $uttid) = split;
    my ($dvd, $meeting, $channel, $speaker, $qqq, $st, $et) = split /_/, $uttid;

    $meetings{$meeting}{$speaker} += ($et - $st);
    $speakers{$speaker} += ($et - $st);
}

my @meetings;
while (my ($meeting, $speakers) = each %meetings) {
    # Accumulate mean and variance of time spoken for each speaker (we
    # want to maximize number of speakers and minimize variance in
    # time spoken)
    my ($mtime, $var, $count);
    # Also accumulate proportion of time a speaker spent in this
    # meeting to total speech for that speaker (we want to avoid
    # picking a meeting which is the sole source of data for a
    # speaker)
    my (%ptimes, %times);
    my $mptime;
    while (my ($speaker, $time) = each %$speakers) {
	$mptime += $ptimes{$speaker} = $time / $speakers{$speaker};
	$mtime += $times{$speaker} = $time;
	$var += $time*$time;
	$count++;
    }
    my $mean = $mtime/$count;
    $mptime /= $count;
    $var = $var/$count - $mean*$mean;
    my $stddev = sqrt($var);

    # Maximize count, minimize stddev, maximize meeting time, minimize
    # proportion of speaker time to total time.
    my $mscore = $stddev ? ($count / $stddev * $mtime * - log $mptime) : 1.0;

    push @meetings, [$mscore, $mtime, $count, $mean, $stddev, $meeting, \%ptimes, \%times];
}

@meetings = sort {$b->[0] <=> $a->[0]} @meetings;
foreach my $meeting (@meetings) {
    my ($mscore, $mtime, $count, $mean, $stddev, $name, $ptimes, $times) = @$meeting;
    printf "Meeting %s:\n", $name;
    printf "\tTotal %.3f seconds from %d speakers:\n", $mtime, $count;
    print "\t", join("\n\t", map sprintf("%s: %.3f sec (%.2f%% of speaker total)", $_,
				      $times->{$_}, $ptimes->{$_} * 100), keys %$ptimes), "\n";
    printf "\tMean %.3f, stddev %.3f\n", $mean, $stddev;
    printf "\tMeeting score %.3f\n", $mscore;
}

print "Speakers:\n";
foreach my $speaker (sort {$speakers{$b} <=> $speakers{$a}} keys %speakers) {
    printf "\t%s, %.3f seconds\n", $speaker, $speakers{$speaker};
}
