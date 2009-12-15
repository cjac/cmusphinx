#!/usr/local/bin/perl
# for purposes of doing stats on utterances types
# reduce an .align file to have single lines per utterance
# [20091012] (air)

if ( scalar @ARGV ne 2 ) { die "usage: split_align_file <in.align> <out.wer>\n"; }

open(IN,$ARGV[0]) or die "can't open $ARGV[0]\n";
open(OUT,">$ARGV[1]") or die "can't open $ARGV[1]\n";

$_=<IN>;  # ignore first line
print $_;

# file comes in four lines per utt
while (<IN>) {
    if ( $_ =~ /TOTAL Words/ ) { last; }

               s/[\r\n]+$//; $lin_ref = $_;
    $_ = <IN>; s/[\r\n]+$//; $lin_hyp = $_;
    $_ = <IN>; s/[\r\n]+$//; $lin_wer = $_;
    $_ = <IN>; s/[\r\n]+$//; $lin_ids = $_;

    # make the uttid and the error easy to get
    $lin_hyp =~ /\((.+?)\)/; $id = $1;
    $lin_wer =~ /Error = (.+?)%/; $wer = $1;

    # "serialize" the utterance
    print OUT "$id\t$wer\t$lin_ref\t$lin_hyp\t$lin_wer\t$lin_ids\n";

}
close(IN);
close(OUT);

#
