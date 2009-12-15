#!/usr/bin/env perl
# from 3 .wer files create a composite for utts that have errors
# [20091026] (air)
#

if ( scalar @ARGV ne 4 ) { die "usage: merge_wer_PP <0inf.wer> <15dB.wer> <10dB.wer> <.out>\n"; }

open(WERA,"logdir/$ARGV[0]/$ARGV[0].wer") or die "can't open $ARGV[0]\n";
open(WERB,"logdir/$ARGV[1]/$ARGV[1].wer") or die "can't open $ARGV[1]\n";
open(WERC,"logdir/$ARGV[2]/$ARGV[2].wer") or die "can't open $ARGV[2]\n";
open(OUT,">$ARGV[3]") or die "can't open $ARGV[3]\n";

# get all the data in
while (<WERA>) {
    s/[\n\r]+$//;
    ($id,$er, $ref,$hyp, $stat) = split /\t/,$_,5;
    @{$data{$id}{WERA}} = ( $id , $er,$ref,$hyp, );
}

while (<WERB>) {
    s/[\n\r]+$//;
    ($id,$er, $ref,$hyp, $stat) = split /\t/,$_,5;
    @{$data{$id}{WERB}} = ( $id , $er,$ref,$hyp, );
}

while (<WERC>) {
    s/[\n\r]+$//;
    ($id,$er, $ref,$hyp, $stat) = split /\t/,$_,5;
    @{$data{$id}{WERC}} = ( $id , $er,$ref,$hyp, );
}

# go through and pick out items with errors in all versions of an utt
$erra = $errb = $errc = 0;
$cnt = 0;
foreach $utt ( keys %data ) {
    # if ( scalar $data{$utt} ne 3 ) { print STDERR "$utt doesn't have a full set!\n"; next; }
    # no errors at any level; skip
    if ( ($data{$utt}{WERA}[1] == 0.0) and ($data{$utt}{WERA}[1] == 0.0) and ($data{$utt}{WERA}[1] == 0.0) ) { next; }

    ($spkr,$utterance) = split /\//,$utt;
    $speaker{$spkr}{CNT}++;

    print "$utt\n";
    print "$data{$utt}{WERA}[1]"; $erra += $data{$utt}{WERA}[1];
    print "\t",$data{$utt}{WERA}[2],"\n";  # ref
    print "\t",$data{$utt}{WERA}[3],"\n";  # hyp
    print "\n";
    print "$data{$utt}{WERB}[1]"; $errb += $data{$utt}{WERB}[1];
    print "\t",$data{$utt}{WERB}[2],"\n";  # ref
    print "\t",$data{$utt}{WERB}[3],"\n";  # hyp
    print "\n";
    print "$data{$utt}{WERC}[1]"; $errc += $data{$utt}{WERC}[1];
    $speaker{$spkr}{WER} += $data{$utt}{WERC}[1];    # just for 10dB
    print "\t",$data{$utt}{WERC}[2],"\n";  # ref
    print "\t",$data{$utt}{WERC}[3],"\n";  # hyp
    print "\n";

    $cnt++;
}

print STDERR "\n$cnt utterances found\n\n";
printf STDERR "WER_0inf=%6.1f%%\n", ($erra/$cnt);
printf STDERR "WER_15dB=%6.1f%%\n", ($errb/$cnt);
printf STDERR "WER_10dB=%6.1f%%\n", ($errc/$cnt);

print STDERR "\n10dB mean WER for utts with errors\n";
foreach $s ( sort keys %speaker ) {
    printf STDERR "%s\t%d\t%5.1f%%\n", $s, $speaker{$s}{CNT},($speaker{$s}{WER}/$speaker{$s}{CNT});
}


#


