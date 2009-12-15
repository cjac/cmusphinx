#!/usr/bin/env perl
# create .ctl files by selecting utts according to various criteria
# [20091023] (air)
#
# use .merged file as input; 
# any version is ok since we're only looking at PP and coverage

use Getopt::Long;

$MERGED = "";
$OUTFILE = "";
$PP = 0;  # for a specific per-utt perplexity threshold
$COV = 0;   # for no unigrams

$result = GetOptions(
    "infile=s", \$MERGED,
    "outfile=s", \$OUTFILE,
    "pp=i", \$PP,
    "cov", \$COV,
    );

if ( ($MERGED eq "") or ($OUTFILE eq "") or (($PP == 0) && ($COV)) ) {
    die "usage: select_by_coverage_or_PP -in <.merged> -out <outfileroot> [-pp <pplimit>] [-cov]\n";
}
open (MERG,"$MERGED") or die "can't open $MERGED\n";
open (REF,">$OUTFILE.ref") or die "can't open $OUTFILE.ref\n";
open (CTL,">$OUTFILE.ctl") or die "can't open $OUTFILE.ctl\n";

# read in all data
while (<MERG>) {
    s/[\n\r]+$//;

    ($id,$pp,$ng2,$ng2,$ng1,$wer,$utt) = split /\s+/,$_,7;
    $data{$id}{PP} = $pp;
    $data{$id}{G3} = $ng3;
    $data{$id}{G2} = $ng2;
    $data{$id}{G1} = $ng1;
    $data{$id}{ER} = $wer;
    $data{$id}{UT} = $utt;

}
close(MERG);

$maxpp = $sumpp = $count = 0;
foreach $id ( sort keys %data) {

    # run through the filters
    if ( $data{$id}{PP} eq "nan" ) { next; }   # ignore utts with OOVs
    if (($PP ne 0) && ($data{$id}{PP} >= $PP) ) { next; }
    if ( $COV &&  ($data{$id}{G1} > 0.0) ) { next; }

    # clean up the utt
    @words = split /\s+/, $data{$id}{UT};
    # shift @words; pop @words;  # input file is different now
    $utt = join " ",@words;

    print CTL "$id\n";
    print REF "$utt ($id)\n";

    if ( $data{$id}{PP} > $maxpp ) { $maxpp = $data{$id}{PP}; }
    $sumpp += $data{$id}{PP};
    $count++;
}
close(REF);
close(CTL);

# print some stats

print "max PP=$maxpp\n";
printf "mean PP=%4.1f\n", ($sumpp/$count);

#
