#!/usr/opt/PERL5004/bin/perl -w

require "../sphinx_train.cfg";


#**************************************************************************
# this script given an mdef file and a  codebook (means/vars in S3 format)
# produces flat mixture weights in a semicontinuos setting. From the models
# produced by this script we can restart normal baum-welch training
# Flat init might not be the best choice, specially if we have access to
# segmentation files for the whole training database.
#**************************************************************************

$gender              = 'g';
$logdir              = "$CFG_CI_LOG_DIR/02.ci_schmm";
$modarchdir          = "$CFG_BASE_DIR/model_architecture";
$hmmdir              = "$CFG_BASE_DIR/model_parameters";
mkdir ($logdir,0777) unless -d $logdir;
mkdir ($modarchdir,0777) unless -d $modarchdir;
mkdir ($hmmdir,0777) unless -d $hmmdir;

#-------------------------------------------------------------------------
# Take the phone list. Put three hyphens after every phone. That is the
# required format. Then make a ci model definition file from it using the
# following program.
#-------------------------------------------------------------------------

#$rawphonefile obtained from variables.def
$phonefile           = "$modarchdir/$CFG_EXPTNAME.phonelist";
$ci_mdeffile         = "$modarchdir/$CFG_EXPTNAME.ci.mdef";

# I don't understand what is happening here RAH
system "sed 's+\$+ - - - +g' $CFG_RAWPHONEFILE > $phonefile";

#set mach = `~rsingh/51..tools/machine_type.csh`
#set MAKE_MDEF = ~rsingh/09..sphinx3code/trainer/bin.$mach/mk_model_def
$MAKE_MDEF = "$CFG_BIN_DIR/mk_model_def";
system ("$MAKE_MDEF -phonelstfn $phonefile -moddeffn $ci_mdeffile -n_state_pm $CFG_STATESPERHMM >& $logdir/${$CFG_EXPTNAME}.make_ci_mdef_fromphonelist.log");

#-------------------------------------------------------------------------
# Decide on what topology to use for the hmms: 3 state, 5 state, blah state
# or what, give it to the variable "statesperhmm" and use it to create
# the topology matrix in the topology file
#-------------------------------------------------------------------------

#$statesperhmm obtained from variables.def
$topologyfile             = "$modarchdir/$CFG_EXPTNAME.topology";

#$base_dir/training/bin/maketopology.csh $statesperhmm $skipstate >! $topologyfile
system ("/sphx_train/csh/maketopology.csh $CFG_STATESPERHMM $CFG_SKIPSTATE > $topologyfile");

#-------------------------------------------------------------------------
# make the flat models using the above topology file and the mdef file
#------------------------------------------------------------------------
$outhmm               = "$hmmdir/${CFG_EXPTNAME}.ci_semi_flatinitial";
mkdir ($outhmm,0777) unless -d $outhmm;


#set FLAT = ~rsingh/09..sphinx3code/trainer/bin.$mach/mk_flat
$FLAT = "$CFG_BIN_DIR/mk_flat";

system ("$FLAT -moddeffn $ci_mdeffile -topo $topologyfile -mixwfn  $outhmm/mixture_weights -tmatfn $outhmm/transition_matrices -nstream $CFG_NUM_STREAMS -ndensity  256  >& $logdir/${CFG_EXPTNAME}.makeflat_cischmm.log");
	
exit 0
