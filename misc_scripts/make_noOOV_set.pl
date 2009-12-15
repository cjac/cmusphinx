#!/usr/bin/env perl
# from a .refids file produce a set of useful files for further processing
# [20091026] (air)
#

use Getopt::Long;
$LMfil="";
$BASE="";
$REFIDS="";

my $result = GetOptions(
    "lm=s", \$LMfil,
    "basename=s", \$BASE,
    "refids=s", \$REFIDS,
    );

if ( ($LMfil eq "") or ($BASE eq "") or ($REFIDS eq "") ) {
    die "usage: make_noOOV_set -lm <lmfile> -refids <.refids> -basename <output_basename>\n";
}

open(LM,"$LMfil") or die "can't open $LMfil\n";
$count = 0;
while (<LM>) { if ( /^\\data\\/ ) { last; } }  # advance to data section
while( <LM>) { if ( /^\\1-grams:/ ) { last; } }  # advance to 1-gram section
while (<LM>) {
    ($likh,$word,$bo) = split /\s+/,$_;
    if (length $likh eq 0) { last; }
    $words{$word}++;
    $count++;
}
print STDERR "$count words found in language model\n";

open(REFID,"$REFIDS") or "can't open $REFIDS.sent\n"; 
$refcnt = 0;
while (<REFID>) {
    s/[\r\n]+$//;
    ($utt,$id) = ( $_ =~ /(.+?)\((.+?)\)/);
    $utts{$id} = $utt;
    $refcnt++;
}
close(REFIDS);

# produce a .corpus file, suitable (in our case) for use with evallm)
# produce corresponding .ref and .ctl files
$oovutt = $okutt = 0;
open(CORPUS,">$BASE.sent") or die "can't open $BASE.sent\n"; 
open(CTL,">$BASE.ctl") or die "can't open $BASE.ctl\n";
open(REF,">$BASE.ref") or die "can't open $BASE.ref\n";
foreach $id ( sort keys %utts ) {
    @wds = split /\s+/,$utts{$id};
    $oov = 0;
    foreach $w (@wds) {
	if ( not defined $words{$w} ) {
	    print STDERR "$id\t$w is an OOV\n";
	    $oov = 1;
	}
    }
    if ( $oov gt 0 ) { $oovutt++; next; }
    print CORPUS "<s> $utts{$id} </s>\n";
    print CTL "$id\n";
    print REF "$utts{$id}\n";
    $okutt++;
}
close(CORPUS);
close(CTL);
close(REF);

print STDERR "refcnt=$refcnt\noovutt=$oovutt\nokutt=$okutt\n";

#
