#!/usr/bin/env perl
# compute WER as a function of ngram coverage
# [20091020] (air)
#
# layout of .mapped: id  PP  3g  2g  1g  WER
# @{%data}                0   1   2   3   4

if ( scalar @ARGV ne 1 ) { die "usage: do_ngram_stats <.merged>\n"; }
open(IN,"$ARGV[0]") or die "can't open $ARGV[0]\n"; 

# get the data
while (<IN>) {
    @line = split /\s+/;
    @{$data{$line[0]}} =  @line[1 .. 5];
}

# compute stats for different thresholds
print "thresh\tutts\tmPP\tmWER\n";
foreach $th (
    100.0,
    90.0,
    80.0,
    70.0,
    60.0,
    50.0,
    40.0,
    30.0,
    20.0,
    10.0,
    0.0
    )
{
    printf " %.1f\t%d\t%5.1f\t%4.1f\n",
    $th,@{&calc($th)};
}



sub calc {
    $thresh = shift;

    $count = 0; $sumwer = 0.0; $sumpp = 0.0;
    foreach $d ( keys %data ) {
	if ( $data{$d}[0] eq "nan" ) { next; }

	# filter - comment these in or out to get the variations
#	if ( ($data{$d}[1]) >= $thresh ) {
#	if ( ($data{$d}[1]+$data{$d}[2]) >= $thresh ) {
	if ( ($data{$d}[3]) >= $thresh ) {
	    $sumpp += $data{$d}[0];
	    $sumwer += $data{$d}[4];
	    $count++;
	}
    }

    if ($count eq 0 ) { return [0,-99,-99] }
    return [ $count, ($sumpp/$count), ($sumwer/$count) ] ;
}


#
