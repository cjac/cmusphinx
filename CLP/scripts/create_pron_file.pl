#!/usr/bin/perl

# used to create a file containing the most likely pronunciation and 
# the number of pronunciations for each word in the dictionary

$input_file = $ARGV[0];  # look at ../data/ws97dictv1+lsegvoc.22k.jrlx 
# input FORMAT: <word> <pron>


$output_file = $ARGV[1]; # look at ../data/prons
# output FORMAT: <word>=<best_pron>;<#prons>

# If the file has no probability distribution on pronunciations, the 
# assumption is that the first listed pronunciation is the most likely one
# If you have a file with probabilities, you have to modify this script
# But NOTE: it seems that the phonetic similarity doesn't play a very important 
# role in the clustering procedure, so I don't think that it is really worth doing it


open(fin,$input_file) || die "I can't open the input file $input_file";

while(<fin>){
    chomp;
    @a=split(/\s+/);
    $b="";
    if ($a[0] =~ /\'/){
	$b = $a[0];
	$b=~ s/\\\'/\'/;
    }
    if (/sp/){
	$sir="";
        for ($i=1; $i<=$#a-1; $i++){
            $sir.=$a[$i]." ";
        }
        chop($sir);
        if ($m{$a[0]} eq ""){
            $m{$a[0]} = $sir;
        }
	if ($b ne "" && $m{$b} eq ""){
	    $m{$b} = $sir;
	}
    }
    $no{$a[0]}++;
    if ($b ne "") {$no{$b}++;};
}
close(fin);

open(fout,">$output_file") || die "I can't open the output file $output_file";

for $k (sort keys %no){
    print fout $k,"=",$m{$k},";",$no{$k},"\n";
}
close(fout);

