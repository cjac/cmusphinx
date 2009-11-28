#!/usr/local/bin/perl5
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
# "Frequency\tchineseWord\n"


if($ARGV[0] eq "-help"){
	print STDERR "\n-----------------------------------------------------------------------";
	print STDERR "\nUsage:";
	print STDERR "\n\tperl5 lrSegmenter.perl [freqFile] < srcMandarinFile > segmentedFile";
	print STDERR "\n-----------------------------------------------------------------------\n";

	exit;
}

if($ARGV[0] eq ""){
	print STDERR "\nUsing default frequency dictionary\n";
	$dictFileName="/afs/cs/user/joy/Joy-work/lrsegmenter/Mandarin.fre.ldc";
}
else{
	$dictFileName=$ARGV[0];
}


%wordFreqList=();				#Chinese word as key of the hash table, frequency is the value
%longestWordListStart=();		#Chinese character as key in the hash table, length of the longest word starting with
								# this character is the value in the table

#reading the dictionary
print STDERR "Reading dictionary $dictFileName\n";
open dictFile, $dictFileName;

while(<dictFile>){
	s/\x0A//;
	s/\x0D//;

	s/\x20/\t/g;

	@entries=split(/\t/,$_);
	
	$thisChnWord=$entries[1];
	$thisFreq=$entries[0];

	$wordFreqList{$thisChnWord}=$thisFreq;

	if($thisChnWord=~/[\x00-\x7F]/){
		#for debug
		#print STDERR "$thisChnWord\n";
	}
	else{
		$thisChnWord=~/^([\x80-\xFF]{2})/;
		$headChar=$1;

		#for debug
		#print STDERR "$thisChnWord \t$headChar\n";


		$thisLen=length($thisChnWord)/2;		#the length of the Chinese word in character

		if($longestWordListStart{$headChar}<$thisLen){
			$longestWordListStart{$headChar}=$thisLen;
		}

	}
}


print STDERR "Dictionary read.\n";


print STDERR "Segmenting...\n";
while(<STDIN>){
	s/\x0A//g;
	s/\x0D//g;
	
	$finalResult="";

	$thisSent=$_;
	$sentLen=length($_);

	$partialChnString="";
	
	$index=0;

	while($index<$sentLen){
		$thisChar=substr($thisSent, $index,1);
		
		if($thisChar ge "\x80")	{		#this is half of a Chinese character
			$thisChar=substr($thisSent,$index,2);
			$index+=2;

			$partialChnString=$partialChnString.$thisChar;

		}
		else{
			$index++;

			if($partialChnString ne ""){
				$partialSegString=segmentAString($partialChnString);
				$finalResult=$finalResult.$partialSegString;
				
				$partialChnString="";
				$partialSegString="";
			}

			$finalResult=$finalResult.$thisChar;

		}
	}

	#in case of pure Chinese characters
	if($partialChnString ne ""){
		$partialSegString=segmentAString($partialChnString);
		$finalResult=$finalResult.$partialSegString;
		
		$partialChnString="";
		$partialSegString="";
	}

	$finalResult=~s/^\x20+//;
	$finalResult=~s/\x20+\Z//;
	$finalResult=~s/\x20+/\x20/g;
	print "$finalResult\n";

}


sub segmentAString{		#segmenting a string of Chinese characters, there should be no non-Chinese character in the string
	$inputString=$_[0];
	$result="";


	#for debug
	#print STDERR "Try to segment string $inputString\n";

	$lenOfString=length($inputString)/2;


	@arcTable=();
	@arcTable=();

	#----------------------------------------------------------
	#step0, initialize the arcTable
	for($i=0;$i<$lenOfString;$i++){
		for($j=0;$j<$lenOfString;$j++){
			if($i==$j){
				$arcTable[$i][$j]=1;				
			}
			else{
				$arcTable[$i][$j]=-1;
			}
		}
	}


	#-----------------------------------------------------------
	#step1: search for all possible arcs in the input string
	#		and create an array for them

	for($currentPos=0;$currentPos<$lenOfString;$currentPos++){			#currentPos is the index of Chinese character
		$currentChar=substr($inputString,$currentPos*2,2);



		#from this position, try to find all possible words led by this character
		$possibleLen=$longestWordListStart{$currentChar};

		#for debug
		#print STDERR "\n$currentChar=$possibleLen\n";

		if(($possibleLen+$currentPos)> ($lenOfString-1)){
			$possibleLen=$lenOfString-$currentPos;
		}

		while($possibleLen>=2){		#all possible words with more than 2 characters
			$subString=substr($inputString,$currentPos*2,$possibleLen*2);

			#for debug
			#print STDERR "s=$subString\n";

			if($wordFreqList{$subString}){
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

	@leftRightSegLabel=();
	@rightLeftSegLabel=();

	#initialize the segmentation label array
	for($k=0;$k<$lenOfString;$k++){
		$leftRightSegLabel[$k]=0;
		$rightLeftSegLabel[$k]=0;
	}
	
	#from left to right
	#-------------------------------
	$leftToRightFreq=0;

	$thisCharIndex=0;
	$charIndexEnd=$lenOfString-1;


	while($thisCharIndex<$lenOfString){
		$endCharIndex=$charIndexEnd;

		$found=0;

		while((!$found)&&($endCharIndex>=$thisCharIndex)){
			if($arcTable[$thisCharIndex][$endCharIndex]!=-1){
				$leftToRightFreq+=log($arcTable[$thisCharIndex][$endCharIndex]);
				$found=1;
			}
			else{
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
	$rightToLeftFreq=0;
	$thisCharIndex=$lenOfString-1;

	while($thisCharIndex>=0){
		$startCharIndex=0;

		$found=0;
		while((!$found)&&($startCharIndex<=$thisCharIndex)){
			if($arcTable[$startCharIndex][$thisCharIndex]!=-1){
				$found=1;
				$rightToLeftFreq+=log($arcTable[$startCharIndex][$thisCharIndex]);
			}
			else{
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
	if($leftToRightFreq>$rightToLeftFreq){			#using left to right solution, prefer right to left
		for($p=0;$p<$lenOfString;$p++){
			$result=$result.substr($inputString, $p*2, 2);

			if($leftRightSegLabel[$p]==1){
				$result=$result." ";
			}
		}
	}
	else{
		for($p=0;$p<$lenOfString;$p++){
			if($rightLeftSegLabel[$p]==1){
				$result=$result." ";
			}
			$result=$result.substr($inputString, $p*2, 2);
		}
	}

	$result=~s/^\x20+//;
	$result=~s/\x20+\Z//;

	#for debug
	#print "result=$result\n";

	return " $result ";

}