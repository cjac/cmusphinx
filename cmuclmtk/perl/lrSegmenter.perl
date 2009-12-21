#!/usr/bin/perl -w
#
#
# This program is a perl version of left-right mandarin segmentor
# As LDC segmenter takes a long time to build the DB files which makes the
# the training process last too long time.
#
# For ablation experiments, we do not need to create the DB files because the
# specific frequency dictionary will be used only once for each slice.
#
# The algorithm for this segmenter is to search the longest word at each point
# from both left and right directions, and choose the one with higher frequency 
# product.
#
# By Joy, joy@cs.cmu.edu
# July 4th, 2001
#
# Usage:
#
# Perl5 lrSegmenter.perl [frequencyDictionary] < mandarinFile > segmentedFiles
#
#	if no parameter is given (no frequency dictionary file specified), there should
#	be one called "Mandarin.fre.complete" as default
#
#
# The format of the dictionary file is this:
# for each line:
# "chineseWord\tfrequency\n"

use strict;
use File::Spec::Functions;
use File::Basename;
use open qw(:std :utf8);

if (@ARGV and $ARGV[0] eq "-help"){
	print STDERR "\n-----------------------------------------------------------------------";
	print STDERR "\nUsage:";
	print STDERR "\n\tperl5 lrSegmenter.perl [freqFile] < srcMandarinFile > segmentedFile";
	print STDERR "\n-----------------------------------------------------------------------\n";

	exit;
}

my $dictFileName;
unless (@ARGV) {
	print STDERR "\nUsing default frequency dictionary\n";
	$dictFileName = catfile(dirname($0), "simplified_chinese.wfreq");
}
else{
	$dictFileName = shift;
}

# Chinese word as key of the hash table, frequency is the value
my %wordFreqList=();
# Chinese character as key in the hash table, length of the longest word starting with
my %longestWordListStart=();
# this character is the value in the table

# reading the dictionary
print STDERR "Reading dictionary $dictFileName\n";
open dictFile, $dictFileName;

while (<dictFile>){
    chomp;
    my ($thisChnWord, $thisFreq) = split;

    $wordFreqList{$thisChnWord}=$thisFreq;

    my $headChar = substr($thisChnWord, 0, 1);

    #for debug
    #print STDERR "$thisChnWord \t$headChar\n";
    my $thisLen = length($thisChnWord);		#the length of the Chinese word in character

    if (!exists($longestWordListStart{$headChar})
	or $longestWordListStart{$headChar}<$thisLen){
	$longestWordListStart{$headChar}=$thisLen;
    }
}


print STDERR "Dictionary read.\n";
print STDERR "Segmenting...\n";

while(<>){
    chomp;
    my $finalResult = "";
    my $thisSent = $_;
    my $partialChnString = "";

    # Break sentence into runs of Hanzi and non-Hanzi
    foreach my $thisChar (split "", $thisSent) {
	if ($thisChar =~ /\p{Han}/) {
	    $partialChnString .= $thisChar;
	}
	else {
	    # A non-Hanzi, so segment everything before it
	    if ($partialChnString ne ""){
		my $partialSegString = segmentAString($partialChnString);
		$finalResult .= $partialSegString;
		$partialChnString = "";
	    }
	    $finalResult .=  $thisChar;
	}
    }
    # Any leftover Hanzi
    if ($partialChnString ne "") {
	my $partialSegString=segmentAString($partialChnString);
	$finalResult .= $partialSegString;
	$partialChnString="";
    }

    print "$finalResult\n";
}

# segmenting a string of Chinese characters, there should be no non-Chinese character in the string
sub segmentAString {
    my $inputString=$_[0];
    my $result="";

    #for debug
    #print STDERR "Try to segment string $inputString\n";

    my $lenOfString=length($inputString);
    my @arcTable=();

    #----------------------------------------------------------
    #step0, initialize the arcTable
    for (my $i=0;$i<$lenOfString;$i++) {
	for (my $j=0;$j<$lenOfString;$j++) {
	    if ($i==$j) {
		$arcTable[$i][$j]=1;				
	    } else {
		$arcTable[$i][$j]=-1;
	    }
	}
    }


    #-----------------------------------------------------------
    #step1: search for all possible arcs in the input string
    #		and create an array for them

    for (my $currentPos=0;$currentPos<$lenOfString;$currentPos++) { #currentPos is the index of Chinese character
	my $currentChar=substr($inputString,$currentPos,1);

	#from this position, try to find all possible words led by this character
	my $possibleLen=$longestWordListStart{$currentChar};
	$possibleLen = 0 unless defined $possibleLen;

	#for debug
	#print STDERR "\n$currentChar=$possibleLen\n";

	if (($possibleLen+$currentPos)> ($lenOfString-1)) {
	    $possibleLen=$lenOfString-$currentPos;
	}

	while ($possibleLen>=2) { #all possible words with more than 2 characters
	    my $subString=substr($inputString,$currentPos,$possibleLen);

	    #for debug
	    #print STDERR "s=$subString\n";

	    if ($wordFreqList{$subString}) {
				#for debug
				#print STDERR "$subString found\n";

		$arcTable[$currentPos][$currentPos+$possibleLen-1]=$wordFreqList{$subString};
	    }


	    $possibleLen--;
	}

    }

    #for debug
    #for($i=0;$i<$lenOfString;$i++){
    #	for($j=0;$j<$lenOfString;$j++){
    #		print "  ",$arcTable[$i][$j];				
    #	}
    #	print "\n";
    #}


    #--------------------------------------------------------------------------
    #step2: from the arc table, try to find the best path as segmentation at
    #each point use the longest possible arc
    # Try from two directions for the search: left to right and right to left
    # using the one with higher product of frequency of the arcs

    my @leftRightSegLabel=();
    my @rightLeftSegLabel=();

    #initialize the segmentation label array
    for (my $k=0;$k<$lenOfString;$k++) {
	$leftRightSegLabel[$k]=0;
	$rightLeftSegLabel[$k]=0;
    }
	
    #from left to right
    #-------------------------------
    my $leftToRightFreq=0;

    my $thisCharIndex=0;
    my $charIndexEnd=$lenOfString-1;


    while ($thisCharIndex<$lenOfString) {
	my $endCharIndex=$charIndexEnd;

	my $found=0;

	while ((!$found)&&($endCharIndex>=$thisCharIndex)) {
	    if ($arcTable[$thisCharIndex][$endCharIndex]!=-1) {
		$leftToRightFreq+=log($arcTable[$thisCharIndex][$endCharIndex]);
		$found=1;
	    } else {
		$endCharIndex--;
	    }
	}

	$leftRightSegLabel[$endCharIndex]=1;
	$thisCharIndex=$endCharIndex+1;
    }

    #for debug
    #print STDERR @leftRightSegLabel,"\n $leftToRightFreq\n";

    #from right to left
    #---------------------------------
    my $rightToLeftFreq=0;
    $thisCharIndex=$lenOfString-1;

    while ($thisCharIndex>=0) {
	my $startCharIndex=0;

	my $found=0;
	while ((!$found)&&($startCharIndex<=$thisCharIndex)) {
	    if ($arcTable[$startCharIndex][$thisCharIndex]!=-1) {
		$found=1;
		$rightToLeftFreq+=log($arcTable[$startCharIndex][$thisCharIndex]);
	    } else {
		$startCharIndex++;
	    }
	}

	$rightLeftSegLabel[$startCharIndex]=1;
	$thisCharIndex=$startCharIndex-1;
    }

    #for debug
    #print STDERR @rightLeftSegLabel,"\n $rightToLeftFreq\n";


    #---------------------------------------------------------------------------------
    # Step3: create result
    if ($leftToRightFreq>$rightToLeftFreq) { #using left to right solution, prefer right to left
	for (my $p=0;$p<$lenOfString;$p++) {
	    $result=$result.substr($inputString, $p, 1);

	    if ($leftRightSegLabel[$p]==1) {
		$result=$result." ";
	    }
	}
    } else {
	for (my $p=0;$p<$lenOfString;$p++) {
	    if ($rightLeftSegLabel[$p]==1) {
		$result=$result." ";
	    }
	    $result=$result.substr($inputString, $p, 1);
	}
    }

    $result=~s/^\s+//;
    $result=~s/\s+$//;

    #for debug
    #print "result=$result\n";

    return " $result ";
}
