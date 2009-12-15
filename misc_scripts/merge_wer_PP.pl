#!/usr/local/bin/perl
# Align PP and WER data for subsequent processing
# [20091012] (air)

if ( scalar @ARGV ne 3 ) { die "usage: merge_wer_PP <.pp_map> <.wer> <.merged>\n"; }

open(MAP,$ARGV[0]) or die "can't open $ARGV[0]\n";
open(WER,$ARGV[1]) or die "can't open $ARGV[1]\n";
open(MRG,">$ARGV[2]") or die "can't open $ARGV[2]\n";

while (<MAP>) {
    s/[\r\n]+$//;

    ($id,$pp,$tg,$bg,$ug,$ut) = split(/\s+/,$_,6);
    $id = lc($id);
    $map{$id} = $pp;
    $cov{$id} = [$tg,$bg,$ug];
    $utt{$id} = $ut;
}
close(MAP);

while (<WER>) {
    s/[\r\n]+$//;

    # get WER per utt
    ($id,$er) = ($_ =~ /(.+?)\s+(.+?)\s/);

    $id = lc($id);
    $wer{$id} = $er;
}
close(WER);

foreach $u ( keys %wer ) {
    print MRG "$u\t$map{$u}\t",join("\t",@{$cov{$u}}),"\t$wer{$u}\t$utt{$u}\n";
}
close(MRG);

#
