#!/usr/bin/env perl
# Generate stats for PP distribution; extract a subset per a cut-off
# [20090612] (air)

if ( scalar @ARGV ne 3 ) { die "usage: sample_pp <.merged> <threshold> <outfile> > <results>\n"; }
open(IN,$ARGV[0]) or die "can't open IN\n";
open(OUT,">$ARGV[2]_$ARGV[1]") or die "can't open OUT\n";
open(CTL,">$ARGV[2]_$ARGV[1].ctl")  or die "can't open CTL\n";
open(REF,">$ARGV[2]_$ARGV[1].ref")  or die "can't open REF\n";
open(UTT,">$ARGV[2]_$ARGV[1].utt")  or die "can't open UTT\n";
$outfile = $ARGV[2];

$CUTOFF = $ARGV[1];  # above PP of CUTOFF everything goes into a single bin

# coverage filter; n-gram hit rate must be at least CUTnG% to use an utt
$CUT3G = 10.0;
$CUT2G = 0.0;
$CUT1G = 0.0;
$CUT32G = 00.0;

$BINSIZE = 10;  # width, in PP of a bin
$LIMIT = 1000; # max allowable PP for creating a controlled-PP subset
$total = 0;
$max = 0;  # for sizing histo
$picked = 0;

# go through id->PP map; bin the id's by PP
while (<IN>) {
    chomp;
    ($id,$pp,$tg,$bg,$ug,$wer,$utt) = split(/\s+/,$_,7);

    # store data for further manipulation
    if ( $pp eq "nan" ) {
	$data{$id}{PP}=$LIMIT+10;
	$data{$id}{TG}=$data{$id}{BG}=$data{$id}{UG}='nan';
    } else {
	$data{$id}{PP} = $pp;
	$data{$id}{TG} = $tg;
	$data{$id}{BG} = $bg;
	$data{$id}{UG} = $ug;
	$data{$id}{UT} = $utt;
	$data{$id}{ID} = $id;
    }
    $data{$id}{ER} = $wer;

    $ppbin = binit($pp);  # pick the bin
    if ($pp>$LIMIT)   { $ppbin = binit($LIMIT)+10; }  # over the max-PP cutoff
    if ($pp eq "nan") { $ppbin = binit($LIMIT)+20; }  # contains oov

    # in each bin, a vector of fileid<->utterance pairs
    if ( not defined $histo{$ppbin} ) {
	@{$histo{$ppbin}{ID}}[0] = $id;  # the fileid
	@{$histo{$ppbin}{UT}}[0] = $utt;  # utt, with context markers
	@{$histo{$ppbin}{PP}}[0] = $pp;   # perplexity
	@{$histo{$ppbin}{TG}}[0] = $tg;  # trigram coverage
	@{$histo{$ppbin}{BG}}[0] = $bg;  # bigram coverage
	@{$histo{$ppbin}{UG}}[0] = $ug;  # unigram coverage
	@{$histo{$ppbin}{ER}}[0] = $wer;  #  word error
	$histo{$ppbin}{COUNT} = 1;
    } else {
	push @{$histo{$ppbin}{ID}} , $id;
	push @{$histo{$ppbin}{UT}} , $utt;
	push @{$histo{$ppbin}{PP}} , $pp;   # perplexity
	push @{$histo{$ppbin}{TG}} , $tg;   # 3-gram
	push @{$histo{$ppbin}{BG}} , $bg;   # 2-gram
	push @{$histo{$ppbin}{UG}} , $ug;   # 3-gram
	push @{$histo{$ppbin}{ER}} , $wer;  #  word error
	$histo{$ppbin}{COUNT} += 1;
    }
    
    if ( $histo{$ppbin}{COUNT} > $max) { $max = $histo{$ppbin}{COUNT}; }
    $total++;
}
close(IN);

$cumul = 0;
$scale = int ((($max)/30));
print "LIMIT= $LIMIT  PP-CUTOFF= $CUTOFF BINSIZE= $BINSIZE set= $outfile\n";
print "3gram-CUTOFF= $CUT3G  2gram-CUTOFF= $CUT2G  1gram-CUTOFF= $CUT1G\n";
print "max= $max  scale= $scale  total= $total\n";
print "bin\tPPest\tm(WER)\tmb(WER)\t m(PP)  mb(PP)\tutts\tcumul\n";

$ppsum = 0; $LASTPP = 0;
@allpp = @allid = @allut = ();
$picked = 0;

# use the bins to do a histogram and counts
# also produce a fileid file that averages to the give PP value
foreach my $x (sort { $a <=> $b } keys %histo) {

    # filters
    if ( $x == ($LIMIT+10) ) { next; } # in the long tail
    if ( $x eq ($LIMIT+20) ) { next; }  # contains OOV

    print $BINSIZE*$x,"";
    $cumul += $histo{$x}{COUNT};  # count of utts in set
    $ppsum += (($x*$BINSIZE)+($BINSIZE/2)) * $histo{$x}{COUNT};  # proxy PP mass to add to PP sum

    # collect utt data to include in selected set
    @curpp = @curer = ();
    foreach $p ( @{$histo{$x}{PP}} ) { 	push @allpp , $p; push @curpp, $p; }
    foreach $p ( @{$histo{$x}{ID}} ) {  push @allid , $p; }
    foreach $p ( @{$histo{$x}{UT}} ) {  push @allut , $p; }
    foreach $p ( @{$histo{$x}{ER}} ) {	push @aller , $p; push @curer, $p; }

    $ppmean = mean(\@allpp);  # mean PP so far
    $ermean = mean(\@aller);  # mean WER so far
    if ( $ppmean <= $CUTOFF ) { print "*"; $LASTPP = $ppmean;}  # part of selection set?

    printf "\t%4.1f",($ppsum/$cumul);  # PPcum (inferred mean PP of set)
    printf "\t%6.2f\t%6.2f", $ermean,mean(\@curer);  # current mean WER and (actual) PP for selected set

    printf "\t%6.1f",$ppmean;  # current bin error
    printf "\t%6.1f",mean(\@curpp);  # current bin pp

    print "\t$histo{$x}{COUNT}\t$cumul\t","|"x($histo{$x}{COUNT}/$scale),"\n";

    # cumulatively write out the desired fileids
    if ( $ppmean <= $CUTOFF ) {
	for (my $i=0; $i<scalar @{$histo{$x}{ID}}; $i++) {
	    printf OUT "%s\t%s", @{$histo{$x}{ID}}[$i],@{$histo{$x}{PP}}[$i];
	    print OUT "\t"; printf OUT "%s",@{$histo{$x}{UT}}[$i]; print  OUT "\n";
	    
	    print CTL "@{$histo{$x}{ID}}[$i]\n";
	    print UTT "@{$histo{$x}{UT}}[$i]\n";

	    my @temp = split /\s+/, @{$histo{$x}{UT}}[$i];
	    pop @temp; shift @temp;  # get rid of cc's
	    print REF join(" ",@temp),"\n";

	    $picked++;
	}
   }
}

print $histo{$LIMIT+10}{COUNT}+$histo{$LIMIT+20}{COUNT}, " are over the limit\n"; 
printf  "at CUTOFF=%d  files: %-4d perplexity: %-5.1f\n", $CUTOFF,$picked,$LASTPP;
close(OUT);close(CTL);close(REF);close(UTT);

# compute histo's with equal counts
$COUNT = 80;
open(BINS,">$ARGV[2]_$ARGV[1].binned") or die "can't open $ARGV[2]_$ARGV[1].binned\n";
open(BINSET,">$ARGV[2]_$ARGV[1].sets") or die "can't open $ARGV[2]_$ARGV[1].sets\n";
$cnt = $ersum = $ppsum = $inner = 0;
foreach $u ( sort {$data{$a}{PP}<=>$data{$b}{PP}} keys %data ) { # sort in ascending PP
    $cnt++;
    $ppsum += $data{$u}->{PP};
    $ersum += $data{$u}->{ER};
    $inner++;

    if ( $data{$u}->{ER} > 0.0 ) {
	printf BINSET "%2d %s", $inner,$data{$u}->{ID};
	print BINSET "\t ",$data{$u}->{PP},"\t";
	# flag all utts with WER>50%
	if ( $data{$u}->{ER} > 50.0 ) { print BINSET "*"; } else { print BINSET " "; }
	printf BINSET " %3.0f", $data{$u}->{ER};
	print BINSET "\t",$data{$u}->{UT},"\n";
    }

    if ( ($cnt % $COUNT) == 0 ) {
	printf BINS "%d\t%.1f\t%.2f\t%d\n", $cnt,($ppsum/$inner),$ersum/$inner,$inner;
	$ppsum = $ersum = $inner = 0;
	print BINSET "\n  $cnt  ",'-'x40,"\n";
    }
    
}
printf BINS "%d\t%.1f\t%.2f\t%d\n", $cnt,($ppsum/$inner),$ersum/$inner,$inner;  # last bin...
close(BINS);
close(BINSET);

#############

# assign a bin number to a PP
sub binit {
    my ($val) = @_;
    return (int ($val/$BINSIZE));
}

# compute mean of a vector
sub mean {
    my ($arr) = @_;
    my $sum = 0;
    my $cnt = 0;
    foreach $x (@$arr) { $sum += $x; $cnt++; }
    return $sum / $cnt;
}


#
