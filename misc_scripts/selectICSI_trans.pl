#!/usr/local/bin/perl

use Getopt::Long;

if(!GetOptions(
               '-removenoise=s'   => \$removenoise,
               '-removemixture=s'   => \$removemixture,
               '-removeoverlap=s'   => \$removeoverlap,
               '-incontrolfile=s' => \$incontrolfile,
               '-intranswosil=s'   => \$intranswosil,
               '-outcontrolfile=s' => \$outcontrolfile,
               '-outtranswsil=s'  => \$outtranswsil,
               '-outtranswosil=s'   => \$outtranswosil,
               '-outtransremoved=s'   => \$outtransremoved,
                ))
{
        printf("         $0 		 <options>: script to post-process the transcript and control files of a particular meeting: remove overlapped utterances, remove utterances mixed with noise, remove noise only utterances\n");      
        printf("         -removemixture      (y/n) : remove utterances mixed with noise\n");
        printf("         -removenoise        (y/n) : remove noise only utterances\n");
        printf("         -removeoverlap      (y/n) : remove utterances overlapped with other utterances in the same meeting, different channels\n");
        printf("         -incontrolfile   <string> : input control file\n");
        printf("         -intranswosil    <string> : input transcript file without silence\n");
        printf("         -outcontrolfile  <string> : output control file\n");
        printf("         -outtranswsil    <string> : output transcript file with silence\n");
        printf("         -outtranswosil   <string> : output transcript file without silence\n");
        printf("         -outtransremoved <string> : output transcripts that were removed\n");
}

# open the input control file
open(incontrollist, $incontrolfile);

# open the input trans without silence file 
open(intranswosil, $intranswosil);

# open the output control file
open(outcontrol, ">$outcontrolfile");

# open the output trans with silence file 
open(outtranswsil, ">$outtranswsil");

# open the output trans without silence file 
open(outtranswosil, ">$outtranswosil");

# open the output trans removed 
open(outtransremoved, ">$outtransremoved");


##############################################################################################
# Let's read all the utterances and get their timing information and controlfile information #
##############################################################################################

$counter = 0; # Will contain the number of utterances in the meeting

while (defined($line = <intranswosil>))
{
	chomp($line);

        $line =~ /(.*)  \((.*)\)/;
        $transcript[$counter] = $1;
        $filename[$counter] = $2;

	## Get the starttime and endtime of each utterance
	$filename[$counter] =~ /dvd.\_.*\_(.*)\_.*\_.*\_(.*)\_(.*)/;
	$chanid[$counter] = $1;
	$starttime[$counter] = $2;
	$endtime[$counter] = $3;

	#print " $chanid[$counter] : $starttime[$counter] : $endtime[$counter] : $filename[$counter] \n";
	
	$controlline[$counter] = <incontrollist>;
	chomp($controlline[$counter]);	


	$counter++; # increment the counter
}

#print "counter : $counter\n";

###########################################################
# initialize all the transcripts assuming we will use all #
########################################################### 

for ($i = 0; $i < $counter; $i ++)
{
	$transcheck [$i] = 1;
}

###########################################################
# detect if there is overlapped speech in each transcript #
###########################################################


for ($i = 0; $i < $counter; $i ++)
{
	# in case the transcript was not singled out from previous iterations
	
	for($j = $i + 1; $j < $counter; $j ++) 
	{
		if( ( ($starttime[$j] <= $starttime[$i]) && ($starttime[$i] + 0.5 < $endtime[$j]) ) || ( ($starttime[$j] + 0.5 < $endtime[$i]) && ($endtime[$i] <= $endtime[$j]) ) || ( ($starttime[$i] <= $starttime[$j]) && ($starttime[$j] + 0.5 < $endtime[$i]) ) || ( ($starttime[$i] + 0.5 < $endtime[$j]) && ($endtime[$j] <= $endtime[$i]) ))
		{
			#printf("i: $starttime[$i] $endtime[$i] \t j:$starttime[$j] $endtime[$j]\n");
			$transcheck [$i] = 0;
			$transcheck [$j] = 0;
		}
	}

}

#$dropcounter = 0;
#for ($i = 0; $i < $counter; $i ++)
#{
#	if ($transcheck [$i] == 0 )	
#	{
#		$dropcounter ++;
#	}
#}
#printf "dropcounter : $dropcounter\n";


########################################################
####### Write the trans and control files ##############
########################################################

for($uttcount = 0; $uttcount < $counter; $uttcount++)
{

	@transcriptparts = split(/ /,$transcript[$uttcount]);
        $NumberOfWords = grep(/.*/,@transcriptparts);

	# to hold the label of the utterance
        $frametrans = "";

	$check2write = 1;

	# check for the overlap condition 
	if(  ($removeoverlap =~ /y/) && ($transcheck[$uttcount] == 0) ) 
	{
		print outtransremoved "$transcript[$uttcount] ($filename[$uttcount])\n";
		next;
	}	
	else
	{

		# print the close non-overlap channels trans	
		for($i = 0; $i < $NumberOfWords; $i++)
		{
			# print N 	
			if( $transcriptparts[$i] =~ /\+\+(.*)\+\+/)
                        {
		 		$frametrans = "$frametrans N";
			}
			# print the silence
			elsif ($transcriptparts[$i] =~ /sil/)
			{
			 	$frametrans = "$frametrans /sil/";	
			}
			# print the owner speech
			else
			{
		 		$frametrans = "$frametrans O";
			}
		}


		if ($frametrans =~ /O/)
		{
	        	# Mixture of O and N
	        	if($frametrans =~ /N/)
        		{
				if($removemixture =~ /y/)
				{
					$check2write = 0;
					print outtransremoved "$transcript[$uttcount] ($filename[$uttcount])\n";
				}	
                	}
		}               

		# Only N
		elsif ($frametrans =~ /N/)
		{
				if($removenoise =~ /y/)
                                {
                                        $check2write = 0;
					print outtransremoved "$transcript[$uttcount] ($filename[$uttcount])\n";
                                }
        	} 

		if ($check2write == 1)
		{
			print outtranswsil "<s> $transcript[$uttcount] <\/s> ($filename[$uttcount])\n";
			print outtranswosil "$transcript[$uttcount] ($filename[$uttcount])\n";
			print outcontrol "$controlline[$uttcount]\n";
		}
	}

}

#close the opened files
close(chanlist);
close(incontrollist);
close(intranswosil);
close(outtranswosil);
close(outtranswsil);
close(outcontrol);
close(outtransremoved);


