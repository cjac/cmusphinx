#!/usr/bin/perl

#########################################################################################
#
# USAGE:  go.find_consensus.pl <latlist> <pronfile> <outdir> [<print>]
#
#########################################################################################

$latlist    = $ARGV[0];            # the file containing the lattice list (see example ../Lists/latlist_devtest)
                                   # Note: the lattices need to be zipped

$pronfile   = $ARGV[1];            # the file containing words, their most likely pronunciation and the number
                                   # of pronunciations (see example ../data/prons)
                                   # Note: 
                                   # you can use ./create_pron_file.pl to create this file from 
                                   # a pronunciation dictionary

$outdir     = $ARGV[2];            # the directory in which we output the file containing the consensus hyps,
                                   # the confusion networks (if the flag $output_networks is 1) and 
                                   # a log file showing the vals of the params used for producing these things 


$print = 0;                      
if ($#ARGV > 2){
    $print  = $ARGV[3];            # 1 if you want to see the merging steps; default 0
}

system("mkdir $outdir");
$outfile   = $outdir."/consensus.score";

############################################################################################
#
# PARAMETERS      (set them before you run the program)
#
############################################################################################


$want_networks    = 1;              # 1 if you want to output confusion networks; otherwise 0 

$scale            = 12;             # scale for the posterior        
$LMscale          = 12;             # language model weight 
$PRscale          = 12;             # pronunciation model weight
$Wdpenalty        = 0;              # word insertion penalty 

$LMoldscale       = 1;              # If the scores in the files have been already scaled....
                                    # we need these values in order to unscale them
$PRoldscale       = 1;              

                                    # There are two ways to prune links:

$Thresh           = 1200;           # 1) remove all the links for which the ratio between the total posterior 
                                    #    prob in the lattice and their posterior prob is greater than $Thresh 
                                    #    For most of my experiments I used 1000; but even much smaller values
                                    #    give the same WER result

$PThresh          = 0;              # 2) sort all the links in decreasing order of their posterior and keep only 
                                    #    $PThresh % of them

                                    # NOTE: only one of $Thresh and $PThresh must be non-zero; 
                                    #       also, the first method of pruning makes much more sense 


$input_format     = "SLF";          # "SLF" for HTK lattices and "FSM" for  lattices in AT&T fsm text format

$time_info        = 1;              # 1 if the input lattices have time information; otherwise 0 
                                    # the assumption is that the FSM lattices don't have time information
                                    # this value is used only for SLF lattices which have time information 
                                    # but we don't want to use it; 

$use_phon_info    = 1;              # 1 if the phonetic similarity is used in clustering; otherwise 0 


$constrain_intra  = 1;              # 1 if you want to constrain the intra-word clustering (namely don't merge
                                    # clusters which don't overlap in time); otherwise 0 
$constrain_inter  = 1;              # 1 if you want to constrain the inter-word clustering (namely don't merge
                                    # clusters which don't overlap in time); otherwise 0



####################################################################

# THAT'S IT! ALL THE PARAMETERS ARE SET...

####################################################################

if ($print){
  $command = "../bin/Consensus_print -i $latlist -c $outfile -R $pronfile";
}
else{
  $command = "../bin/Consensus -i $latlist -c $outfile -R $pronfile";
}

if ($want_networks){
  $netdir = $outdir."/NETWORKS";
  system("mkdir $netdir");
  $command.=" -C $netdir";
}

$command.= " -S $scale -L $LMscale -l $LMoldscale -P $PRscale -p $PRoldscale -I $Wdpenalty";
  
if ($Thresh > 0){
  $command.= " -T $Thresh";
}
else{
  if ($PThresh > 0){
    $command.= " -t $PThresh";
  }
}
  
if ($use_phon_info == 0){
  $command.= " -s";
}
if ($time_info == 0){
  $command.= " -o";
}
if ($constrain_intra == 1){
  $command.= " -b";
}
if ($constrain_inter == 1){
  $command.= " -n";
}

if ($input_format eq "FSM"){
  $command.=" -f";
}

$logfile = $outdir."/log";
$command.= " -G $logfile";

system($command);

open(flog,">>$logfile");
print flog $command,"\n";
close(flog);
close(f);
