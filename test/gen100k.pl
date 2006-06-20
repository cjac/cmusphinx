#!/usr/local/bin/perl 

use strict;
my $i;
my $j;
my $k;

srand(1);

for($j=0;$j<=5;$j++){
    for($i=0;$i<=1000000;$i++){
	$k=rand();
	printf("%s%d ","a",$i);
	if($k>0.8){
	    printf("\n");
	}
    }
}
