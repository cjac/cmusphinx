#!/usr/local/bin/perl5 

# RAH Force passage of config file, or look for it in the current directory
if (lc($ARGV[0]) eq '-cfg') {
    $cfg_file = $ARGV[1];
    die "-cfg specified, but unable to find file $ARGV[1]" unless (-s $cfg_file);
    require $cfg_file;
} else {
    $cfg_file = "./sphinx_train.cfg";
    die "Must specify -cfg or create default file ./sphinx_train.cfg" unless (-s  $cfg_file);
    require ("./sphinx_train.cfg");
    &ST_LogWarning ("-cfg not specified, using the default ./sphinx_train.cfg");
}


my ($AGG_SEG,$len,$stride,$segdmpdir,$dumpfile,$logfile);

&ST_Log ("\tAggSeg ");

#$AGG_SEG  = "~rsingh/09..sphinx3code/trainer/bin.alpha/agg_seg"
$AGG_SEG  = "$CFG_BIN_DIR/agg_seg";

#unlimit
#limit core 0k

#Compute VQ codebooks on no more than 1 million vectors for sanity
#That should be about 2500 files assuming about 10 seconds of speech per file
# 1000*10*100 = 1 mil

#Instead of calling wc let's open the file. (Note on WIN32, wc may not exist)
open CTL,"$CFG_LISTOFFILES";
$len =0;
while (<CTL>) {
    $len++;
}
close CTL;

$stride = 1 unless int($stride = $len/2500);

mkdir ($CFG_VQ_LOG_DIR,0777) unless -d $CFG_VQ_LOG_DIR;

$segdmpdir = "$CFG_BASE_DIR/bwaccumdir/${CFG_EXPTNAME}_buff_1";
mkdir ($segdmpdir,0777) unless -d $segdmpdir;

$dumpfile = "$segdmpdir/${CFG_EXPTNAME}.dmp";
$logfile = "$CFG_VQ_LOG_DIR/${CFG_EXPTNAME}.vq.agg_seg.log";
&ST_HTML_Print ("\t<A HREF=\"$logfile\">Log File</A> ");

# run it here 
#system ("$AGG_SEG -segdmpdirs $segdmpdir -segdmpfn $dumpfile  -segtype all -ctlfn $CFG_LISTOFFILES -cepdir $CFG_FEATFILES_DIR -cepext $CFG_FEATFILE_EXTENSION -ceplen $CFG_VECTOR_LENGTH -agc $CFG_AGC -cmn $CFG_CMN -feat $CFG_FEATURE -stride $stride");

if (open PIPE,"$AGG_SEG -segdmpdirs $segdmpdir -segdmpfn $dumpfile  -segtype all -ctlfn $CFG_LISTOFFILES -cepdir $CFG_FEATFILES_DIR -cepext $CFG_FEATFILE_EXTENSION -ceplen $CFG_VECTOR_LENGTH -agc $CFG_AGC -cmn $CFG_CMN -feat $CFG_FEATURE -stride $stride  2>&1 |") {
    open LOG,">$logfile";
    while (<PIPE>) {
	print LOG "$_";
    }
    close LOG;
    close PIPE;
    &ST_HTML_Print ("\t\t<font color=\"$CFG_OKAY_COLOR\"> completed </font>\n");
    exit (0);
}

&ST_HTML_Print ("\t\t<font color=\"$CFG_ERROR_COLOR\"> FAILED </font>\n");
exit (-1);

