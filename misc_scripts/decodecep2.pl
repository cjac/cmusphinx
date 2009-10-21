#!/usr/bin/perl -w
## ====================================================================
##
## Copyright (c) 1996-2004 Carnegie Mellon University.  All rights 
## reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
##
## 1. Redistributions of source code must retain the above copyright
##    notice, this list of conditions and the following disclaimer. 
##
## 2. Redistributions in binary form must reproduce the above copyright
##    notice, this list of conditions and the following disclaimer in
##    the documentation and/or other materials provided with the
##    distribution.
##
## This work was supported in part by funding from the Defense Advanced 
## Research Projects Agency and the National Science Foundation of the 
## United States of America, and the CMU Sphinx Speech Consortium.
##
## THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
## ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
## THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
## PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
## NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
## SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
## LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
## DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
## THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
## OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##
## ====================================================================
# For fun, also read ./doc/APD.disclaimer

use strict;
use File::Basename;
use File::Spec;
use File::Path;
use lib dirname($0);
use Config::General;
use Sys::Hostname;
use Getopt::Long;
use Pod::Usage;
use Cwd;

sub system_log {
    my $logfile = shift;
    die "fork failed: $!" unless defined(my $pid = fork);
    if ($pid) {
	waitpid $pid, 0;
	if ($? != 0) {
	    my (undef, $file, $line) = caller();
	    print STDERR "$_[0] failed with status $? at $file:$line\n";
	    exit 1;
	}
    }
    else {
	if (defined($logfile)) {
	    open STDOUT, ">>$logfile";
	    open STDERR, ">&STDOUT";
	    print "@_\n";
	}
	exec @_;
	die "exec failed: $!";
    }
}

# Command-line options
my @argspec = ('npart=i', 'part=i', 'ctlfile=s', 'cepext=s', 'cepdir=s',
	       'rawdir=s', 'expid=s', 'feat=s', 'acmoddir=s', 'mdef=s',
	       'mean=s', 'var=s', 'mixw=s', 'tmat=s', 'args=s', 'ceplen=i',
	       'mfclogdir=s', 'lattice', 'matchseg', 'matchconf', 'help|h|?', 'logdir=s',
	       'config|cfg=s', 'pl_window=i', 'pl_beam=s', 'pheurtype=i', 'op_mode=s',
	       'wend_beam=s', 'dsratio=i', 'cond_ds=i', 'ci_pbeam=s',
	       'gsfile=s', 'svqfile=s', 'svq4svq=i', 'subvqbeam=s', 'inlatdir=s',
	       'latext=s', 'lpbeam=s', 'lponlybeam=s', 'succtab=s',
	       'vqeval=i', 'gs4gs=i', 'bin=s', 'lminmemory=i', 'dither',
	       'lmctlfile=s', 'lmname=s', 'ctl_lm=s', 'ctl_mllr=s', 's3',
	       's3_3', 's3_3align', 's3_X', 's3_lp', 'ctlpath', 'fwdflat=s', 'bestpath=s',
	       's3_align', 's3_dag', 's3_new', 's2', 'compallsen', 'cachesen',
	       'memchk', 'dictfn=s', 'fdictfn=s', 'debug', 'kdtree=s',
	       'topn=i','silpen=s','noisepen=s','phonepen=s', 'lw=s',
	       'beam=s', 'pbeam=s', 'wbeam=s', 'maxhistpf=i', "lda=s", "ldadim=i",
	       'maxhmmpf=i', 'maxwpf=i', 'maxcdsenpf=i', 'senmgau=s',
	       'cmn=s', 'uw=s', 'wip=s', 'logbase=s', 'lmfile=s', 'mllr=s',
	       'Nlextree=s', 'other=s', 'reffile=s', 'pbs', 'alignonly', 'qname=s',
	       'mapfn=s', 'phnfn=s', 'cbdir=s', 'hmmdir=s', 'hmmdirlist=s',
	       'adcin=s', "min_endfr=i", "dagfudge=i", 'srate=i', "insert_sil",
	       'fwdflatefwid=i', 'fwdflatsfwin=i', 'sendumpfn=s', 'varfloor=s',
	       'silent', 'cer'
	      );

# Get options into a hash
my %cmdargs;
GetOptions(\%cmdargs, @argspec) or pod2usage(1);
pod2usage(0) if $cmdargs{help};

# Try not to create unreadable files (but it might not work)
umask 0002;

unless (defined($cmdargs{config})) {
    print "The -config option is required.\n";
    pod2usage(1);
}

#Do not change this line to printf !  It is a designer's choice, this
#ensure users of this script will specify experiment ID.  The hope is
#that the users will make sure every experiment they run are recorded.

if(!defined($cmdargs{expid})) {
   die("Experiment ID is not specified. Please use -expid to specify it. \n");
}

my $hostname = hostname();
$hostname =~ s/\..*$//;
my $cwd = cwd;
my $basedir = network_path($hostname, $cwd);
my $conf = new Config::General(
    -ConfigFile     => $cmdargs{config},
    -ExtendedAccess => 1,
    -InterPolateVars => 1,
    # Predefine some useful variables for interpolation
    -MergeDuplicateBlocks => 1,
    -DefaultConfig => {
		       PWD => $cwd,
		       BASEDIR => $basedir,
		       HOME => $ENV{HOME},
		      },
);

# Overide defaults with configuration file with command-line
my %defaults = (cepext => 'mfc',
		logdir => './log',
		feat => '1s_c_d_dd',
		lw => '7.5',
		topn => '4',
		logbase => '1.0001',
		beam => '1e-80',
		pbeam => '1e-60',
		wbeam => '1e-40',
		maxhistpf => '200',
		maxhmmpf => '100000',
		maxwpf => '40',
		agc => 'none',
		varnorm => 'no',
		);
# Overide defaults with configuration file with command-line
$cmdargs{ACMODDIR} = $cmdargs{acmoddir} if defined $cmdargs{acmoddir};
my %conf = $conf->getall;
# I don't know where <ResPath> ever came from...
if (defined($conf{ResPath})) {
    %conf = %{$conf{ResPath}};
}
# Do stuff to the defaults for Sphinx2 whose beams are *8
if ($cmdargs{s2} or $conf{s2}) {
    %defaults = (%defaults,
		 beam => '1e-6',
		 pbeam => '1e-6',
		 wbeam => '3e-4');
}
my %options = (%defaults, %conf, %cmdargs);

# Set up model paramter files based on ACMODDIR
if (defined($options{ACMODDIR})) {
    # Sphinx 3 options
    if ($options{s3_3}) {
	$options{hmmdir} ||= $options{ACMODDIR};
    }
    elsif (defined($options{mdef})) {
	$options{mean} ||= "$options{ACMODDIR}/means";
	$options{var} ||= "$options{ACMODDIR}/variances";
	$options{mixw} ||= "$options{ACMODDIR}/mixture_weights";
	$options{tmat} ||= "$options{ACMODDIR}/transition_matrices";
    }
    else {
	# Sphinx 2 options
	$options{mapfn} ||= "$options{ACMODDIR}/map";
	$options{phnfn} ||= "$options{ACMODDIR}/phone";
	$options{cbdir} ||= $options{ACMODDIR};
	$options{hmmdir} ||= $options{ACMODDIR};
	$options{hmmdirlist} ||= $options{ACMODDIR};
    }
}

# Guess a binary from binpath if it exists
if (!defined($options{bin}) and defined($options{binPath})) {
    if ($options{s3}){
	$options{bin} = "$options{binpath}/s3decode-anytopo";
    }
    elsif($options{s2}) {
	$options{bin} = "$options{binpath}/sphinx2-batch";
    }
    elsif($options{s3_3} || $options{s3_X}) {
	$options{bin} = "$options{binpath}/decode";
    }
    elsif($options{s3_lp}){
	$options{bin} = "$options{binpath}/livepretend";
    }
    elsif($options{s3_3align}){
	$options{bin} = "$options{binpath}/align";
    }
    elsif($options{s3_dag}){
	$options{bin} = "$options{binpath}/dag";
    }
    elsif($options{s3_astar}){
	$options{bin} = "$options{binpath}/astar";
    }
    elsif($options{s3_align}){
	$options{bin} = "$options{binpath}/align";
    }
    elsif($options{s3_allp}){
	$options{bin} = "$options{binpath}/allphone";
    }
    elsif($options{s3_new}){
	$options{bin} = "$options{binpath}/decode_anytopo";
    }
    else{
	die "cannot decide which binary to use. (no -s3|-s3_3|s3_X|-s3_lp|-s3_dag|-s3_align|-s3_allp|-s3_new and no -bin)\n";
    }
}

#Do not change this line to printf !  It is a designer's choice, a
#default binary file usually cause confusion in future users.  Also,
#default binary file will always be missing in the future

if(!defined($options{bin})){
    die("Binary of the decoder is not specified.\nIn command line, you can specify it by -bin.\nIn config, you can specify it with the 'bin' option\n");
}

my $resultdir="$options{logdir}/$options{expid}";
mkpath($resultdir);
my $segdir = $resultdir;
$segdir .= ",CTL" if $options{ctlpath};
my $outlatdir = $resultdir;
$outlatdir .= ",CTL" if $options{ctlpath};

sub network_path {
    my ($host, $path) = @_;
    $path = File::Spec->rel2abs($path);
    $path =~ s,/.automount/([^/]+)/root,/net/$1,;
    if ($path !~ m,^/net/,) {
	$path = "/net/${host}$path";
    }
    return $path;
}

my ($ctlcount, $ctloffset);
my ($logfile, $matchfile, $matchsegfile, $matchconffile);
if (defined($options{part}) and defined($options{npart})) {
    open CTLFILE, "<$options{ctlfile}" or die "Failed to open $options{ctlfile}: $!\n";
    my $lines;
    while (<CTLFILE>) { $lines++ };
    close CTLFILE;

    $cmdargs{npart} = $options{npart} = $lines if $options{npart} > $lines;
    $ctlcount = int(($lines / $options{npart}) + 0.5);
    $ctloffset = $ctlcount * ($options{part} - 1);
    if ($options{part} == $options{npart}) {
	$ctlcount = $lines - $ctloffset;
    }
    $logfile = "$resultdir/$options{expid}.$options{part}.log";
    $matchfile = "$resultdir/$options{expid}.$options{part}.match";
    $matchsegfile = "$resultdir/$options{expid}.$options{part}.matchseg";
    $matchconffile = "$resultdir/$options{expid}.$options{part}.matchconf";
}
else {
    $logfile = "$resultdir/$options{expid}.log";
    $matchfile = "$resultdir/$options{expid}.match";
    $matchsegfile = "$resultdir/$options{expid}.matchseg";
    $matchconffile = "$resultdir/$options{expid}.matchconf";
}

if (defined($cmdargs{pbs})) {
    # Adjust some options and re-run self in PBS queue
    my $qsub = dirname($0)."/qsub.pl";
    my $qname = defined($options{qname}) ? $options{qname} : "workq";
    my $hostname = hostname();
    $hostname =~ s/\..*$//;

    # Get rid of this for queue jobs
    delete $cmdargs{pbs};
    # Queue jobs need to have the right path here!
    $cmdargs{logdir} = $options{logdir};
    foreach my $arg (qw(ctlfile cepdir rawdir acmoddir mdef mean var tmat mixw
			lmfile lmctlfile bin lda sendumpfn inlatdir
			dictfn fdictfn mapfn phnfn cbdir hmmdir hmmdirlist
			ctl_lm ctl_mllr mllr gsfile svqfile reffile config logdir)) {

	next unless exists($cmdargs{$arg}) and defined($cmdargs{$arg});
	$cmdargs{$arg} = network_path($hostname, $cmdargs{$arg});
    }
    my $self = network_path($hostname, $0);
    my $npart = $cmdargs{npart};
    delete $cmdargs{part} if defined($npart);

    # PBS doesn't like job names that start with numbers
    my $qexpid;
    if ($cmdargs{expid} =~ /^[^A-Za-z]/) {
	$qexpid = "Q$cmdargs{expid}";
    } else {
	$qexpid = $cmdargs{expid};
    }
    $qexpid =~ tr/_//d;

    if (defined($npart)) {
	my @jobs;
	for (my $i = 1; $i <= $cmdargs{npart}; ++$i) {
	    # Quoted args for the shell
	    my @args;
	    while (my ($k, $v) = each %cmdargs) {
		push @args, "-$k" => "'$v'" if defined $v;
	    }
	    my $jobname = substr($qexpid, 0, 15-length(".$i")) . ".$i";
	    my $efile = network_path($hostname, "$resultdir/$jobname.err");
	    my $ofile = network_path($hostname, "$resultdir/$jobname.out");
	    print "$qsub --cwd '$basedir' -q $qname -e '$efile' -o '$ofile' -J $jobname -- '$self' @args -part $i\n";
	    my $jobid = `$qsub --cwd '$basedir' -q $qname -e '$efile' -o '$ofile' -J $jobname -- '$self' @args -part $i`; # FIXME...
	    if ($jobid =~ /^Job <(.+)>$/) {
		print "Part $i = job $1\n";
		push @jobs, $1;
	    }
	    else {
		die "qsub failed";
	    }
	}
	my $deps = "-Wdepend=afterok:" . join ':', @jobs;
	my $efile = network_path($hostname, "$resultdir/$cmdargs{expid}.align.err");
	my $ofile = network_path($hostname, "$resultdir/$cmdargs{expid}.align.out");
	my $jobname = substr($qexpid, 0, 15-length(".aln")) . ".aln";
	# Wait a little while for jobs to reach the queue (HACK!)
	sleep 2;
	# Unquoted args for system()
	my @args;
	while (my ($k, $v) = each %cmdargs) {
	    push @args, "-$k" => $v if defined $v;
	}
	system_log(undef, $qsub, $deps, -J => $jobname, '--cwd' => $basedir,
		   -q => $qname, -e => $efile, -o => $ofile,
		   '--', $self, @args, '-alignonly');
    }
    else {
	my $jobname = "$cmdargs{expid}";
	my $efile = network_path($hostname, "$resultdir/$jobname.err");
	my $ofile = network_path($hostname, "$resultdir/$jobname.out");
	# Unquoted args for system()
	my @args;
	while (my ($k, $v) = each %cmdargs) {
	    push @args, "-$k" => $v if defined $v;
	}
	system_log(undef, $qsub, -J => $qexpid, -q => $qname, '--cwd' => $basedir,
		   -e => $efile, -o => $ofile, '--', $self, @args);
    }

    exit;
}

if (defined($options{alignonly})) {
    # Yes, I know, this is awful
    goto ALIGNONLY;
}

open (LOGFILE,">$logfile") ||die "Cannot open logfile $logfile for output\n";

if(! (-e $options{cepdir} or -e $options{rawdir})) {
    printf LOGFILE "CEP dir %s doesn't exist\n" , $options{cepdir} ;
    printf("CEP dir %s doesn't exist\n",$options{cepdir});
    exit(-1);
}

if(! -e $options{ACMODDIR}) {
    printf LOGFILE "ACMOD dir %s doesn't exist\n", $options{ACMODDIR};
    printf("ACMOD dir %s doesn't exist\n",$options{ACMODDIR});
    exit(-1);
}

if(!($options{s3_align} || $options{s3_allp})
   && !(-e $options{lmfile} || -e $options{lmctlfile})) {
    printf LOGFILE "LM file %s doesn't exist\n", $options{lmfile};
    printf("LM file %s doesn't exist\n",$options{lmfile});
    exit(-1);
}

if(! -e $options{ctlfile}) {
    printf LOGFILE "CTL file %s doesn't exist\n",$options{ctlfile};
    printf("CTL file %s doesn't exist\n",$options{ctlfile});
    exit(-1);
}

if(!(defined($options{s3_3}) or defined($options{s3_X}) or
     defined($options{s3}) or defined($options{s2}) or defined($options{s3_lp}) or 
     defined($options{s3_dag}) or defined($options{s3_align}) or
     defined($options{s3_astar}) or defined($options{s3_allp}) or
     defined($options{s3_new}) or defined($options{s3_3align})
     )
   )
{
    printf STDERR "Please define -s2/-s3/-s3_3/-s3_allp/-s3_X/-s3_lp/-s3_dag/-s3_astar/-s3_new in your config file. \n";
    exit -1;
}

my @cmdoptions;
push @cmdoptions, split ' ', $options{args} if defined($options{args});

#Define command-line options for s3.0 family of decoding tools at here. 
my @s3opts = qw(mdef mean var mixw trans dictfn fdictfn lmfile cepdir ctlfile
		feat cmn beam wbeam agc varnorm ctl_mllr lw uw wip topn varfloor
		ceplen phonepen silpen noisepen logbase senmgau cepext inlatdir latext);
my %s3map = (mdef => 'mdeffn', mean => 'meanfn', var => 'varfn', mixw => 'mixwfn',
	     tmat => 'tmatfn', wbeam => 'nwbeam', ctlfile => 'ctlfn', lmfile => 'lmfn',
	     ctl_mllr => 'mllrctlfn', lw => 'langwt', uw => 'ugwt', wip => 'inspen',
	     senmgau => 'senmgaufn');

my @s3dagopts = qw(mdef dictfn fdictfn lmfile ctlfile lmctlfile lmname
		   lw uw wip phonepen silpen noisepen inlatdir latext min_endfr
		   dagfudge);

my %s3dagmap = (ctlfile => 'ctl', lmfile => 'lm', lmctlfile => 'lmctlfn',
		dictfn => 'dict', fdictfn => 'fdict');

my @s3astaropts = qw(mdef dictfn fdictfn lmfile ctlfile lmctlfile lmname
		    lw uw wip phonepen silpen noisepen inlatdir latext min_endfr
		    dagfudge);

my %s3astarmap = (ctlfile => 'ctl', lmfile => 'lm', lmctlfile => 'lmctlfn',
		  dictfn => 'dict', fdictfn => 'fdict');

my @s3allpopts = qw(mdef mean var mixw tmat
		    cepdir ctlfile feat cmn beam agc varnorm
		    ctl_mllr topn phsegdir logbase lmfile lw
		    phonetpfile phonetpwt phonetpfloor
		  );
my %s3allpmap = (tmat => 'tmat', ctlfile => 'ctl',
		 phonetpfile => 'phonetp', lmfile => 'lm',
		 lw => 'phonetpwt');

my @s3alignopts = qw(mdef mean var mixw tmat dictfn fdictfn
		      cepdir ctlfile feat cmn beam agc varnorm
		      ctl_mllr topn insentfn outsentfn wdsegdir
		      phsegdir reffile logbase insert_sil
		     );
my %s3alignmap = (dictfn => 'dict', fdictfn => 'fdict',
		   tmat => 'tmat', ctlfile => 'ctl',
		   reffile => 'insent', insentfn => 'insent',
		   outsentfn => 'outsent');

#Define command-line options for s3.x family of decoding tools at here. 
my @s3newopts = qw(mdef mean var mixw trans dictfn fdictfn lmfile lda ldadim
		cepdir ctlfile feat cmn beam wbeam agc varnorm logbase varfloor
		ctl_mllr lw uw wip topn phonepen silpen noisepen senmgau inlatdir latext);

my %s3newmap = (dictfn => 'dict', fdictfn => 'fdict', tmat => 'tmat',
		gsfile => 'gs', svqfile => 'subvq', lmfile => 'lm',
		ctlfile => 'ctl', lmctlfile => 'lmctlfn', 
		silpen => 'silprob', noisepen=>'fillprob' );

my @s3xopts = qw(mdef mean var mixw tmat dictfn fdictfn lw uw wip dither lda ldadim
		 feat agc varnorm ctlfile cepdir cmn beam pbeam wbeam ceplen op_mode
		 svqfile lmctlfile lmname vqeval ci_pbeam cond_ds svq4svq inlatdir latext
		 gs4gs pl_window pl_beam pheurtype wend_beam ctl_lm logbase
		 ctl_mllr Nlextree subvqbeam tighten_factor dsratio senmgau
		 rawext cepext adcin adchdr adcendian allphone compallsen
		 upperf lowerf nfilt wlen srate frate nfft cachesen silpen
		 fwdtree fwdflat bestpath fwdflatlw bestpathlw lmfile
		 fwdflatefwid fwdflatsfwin kdtree kdmaxbbi succtab
		 sendumpfn fwdflatbeam fwdflatnwbeam noisepen hmmdir
                 lpbeam lponlybeam
	       );

my %s3xmap = (dictfn => 'dict', fdictfn => 'fdict', tmat => 'tmat', silpen => 'silprob',
	      gsfile => 'gs', svqfile => 'subvq', lmfile => 'lm', srate => 'samprate',
	      dsratio=> 'ds', ctlfile => 'ctl', lmctlfile => 'lmctlfn', noisepen => 'fillprob',
	      fwdflatnwbeam => 'fwdflatwbeam', hmmdir => 'hmm', sendumpfn => 'sendump');

my @s2opts = qw(mapfn phnfn cbdir hmmdir hmmdirlist dictfn fdictfn ctlfile cepdir rawdir
		lw lmfile lmctlfile uw wip silpen noisepen agc cmn topn beam wbeam
		adcin fwdtree fwdflat forwardonly bestpath backtrace kbdumpdir dsratio
		sendumpfn 8bsen mdef mean var mixw tmat kdtreefn pbeam lpbeam lponlybeam
		upperf lowerf nfilt wlen srate frate nfft maxwpf maxhmmpf rescorelw
		fwdflatbeam fwdflatnwbeam fwdflatlw oovdictfn topsenfrm topsenthresh
		compallsen phoneconf phonetpfn ptplw uptpwt reportpron ascrscale
		lmnamedir lmnameext datadir phperp);

my %s2map = (lw => 'langwt', lmctlfile => 'lmctlfn', lmfile => 'lmfn',
	     uw => 'ugwt', wip => 'inspen', noisepen => 'fillpen',
	     ctlfile => 'ctlfn', topn => 'top', wbeam => 'nwbeam',
	     rawdir => 'datadir', pbeam => 'npbeam',
	     mdef => 'mdeffn', mean => 'meanfn', srate => 'samp',
	     var => 'varfn', mixw => 'mixwfn', tmat => 'tmatfn',
	     fdictfn => 'ndictfn');

if (defined($options{s3_3}) or defined($options{s3_X})){
    # ARGH Sphinx3 has to be different
    $options{cepext} = "." . $options{cepext};
    push @cmdoptions, get_cmd_options(\%options, \@s3xopts, \%s3xmap);
    push @cmdoptions, -ctlcount => $ctlcount if defined($ctlcount);
    push @cmdoptions, -ctloffset => $ctloffset if defined($ctloffset);
    push @cmdoptions, -hyp => $matchfile;
    push @cmdoptions, -hypseg => $matchsegfile if defined($options{matchseg});
    push @cmdoptions, -hypconf => $matchconffile if defined($options{matchconf});
    push @cmdoptions, -outlatdir => $outlatdir if defined($options{lattice});
}
elsif(defined($options{s3})) {
    # Warn about S3X-only options
    foreach my $s3xopt (qw(pbeam maxhistpf maxhmmpf maxcdsenpf maxwpf vqeval dsratio
			   gsfile ci_pbeam cond_ds svqfile svq4svq Nlextree
			   subvqbeam gs4gs pl_window pl_beam pheurtype mllr
			   wend_beam lmctlfile lmname ctl_lm)) {
	if (defined($options{$s3xopt})) {
	    print "-$s3xopt is not used with Sphinx 3.0\n";
	    delete $options{$s3xopt};
	}
    }
    push @cmdoptions, get_cmd_options(\%options, \@s3opts, \%s3map);
    push @cmdoptions, -matchfn => $matchfile;
    push @cmdoptions, -matchsegfn => $matchsegfile if defined($options{matchseg});
    push @cmdoptions, -outlatdir => $outlatdir if defined($options{lattice});
    push @cmdoptions, -ctlcount => $ctlcount if defined($ctlcount);
    push @cmdoptions, -ctloffset => $ctloffset if defined($ctloffset);
}
elsif(defined($options{s3_dag})){
    push @cmdoptions, get_cmd_options(\%options, \@s3dagopts, \%s3dagmap);
    push @cmdoptions, -match => $matchfile;
    push @cmdoptions, -matchseg => $matchsegfile if defined($options{matchseg});
    push @cmdoptions, -ctlcount => $ctlcount if defined($ctlcount);
    push @cmdoptions, -ctloffset => $ctloffset if defined($ctloffset);
}
elsif(defined($options{s3_astar})){
    push @cmdoptions, get_cmd_options(\%options, \@s3dagopts, \%s3dagmap);
    push @cmdoptions, -nbestdir => $resultdir;
    push @cmdoptions, -ctlcount => $ctlcount if defined($ctlcount);
    push @cmdoptions, -ctloffset => $ctloffset if defined($ctloffset);
}
elsif(defined($options{s3_align})){
    # ARGH Sphinx3 has to be different
    $options{cepext} = "." . $options{cepext};
    push @cmdoptions, get_cmd_options(\%options, \@s3alignopts, \%s3alignmap);
    push @cmdoptions, -ctlcount => $ctlcount if defined($ctlcount);
    push @cmdoptions, -ctloffset => $ctloffset if defined($ctloffset);
    push @cmdoptions, -outsent => $matchfile;
    push @cmdoptions, -wdsegdir => $segdir if defined($options{wdseg});
    push @cmdoptions, -phsegdir => $segdir if defined($options{phseg});
    push @cmdoptions, -phlabdir => $segdir if defined($options{phlab});
    push @cmdoptions, -stsegdir => $segdir if defined($options{stseg});

}
elsif(defined($options{s3_allp})){
    # ARGH Sphinx3 has to be different
    $options{cepext} = "." . $options{cepext};
    push @cmdoptions, get_cmd_options(\%options, \@s3allpopts, \%s3allpmap);
    push @cmdoptions, -ctlcount => $ctlcount if defined($ctlcount);
    push @cmdoptions, -ctloffset => $ctloffset if defined($ctloffset);
    push @cmdoptions, -phsegdir => $segdir if defined($options{phseg});
    push @cmdoptions, -hyp => $matchfile;
    push @cmdoptions, -hypseg => $matchsegfile if defined($options{matchseg});
    push @cmdoptions, -phlatdir => $resultdir if defined($options{lattice});
}
elsif(defined($options{s3_lp})){
    # ARGH Sphinx3 has to be different
    $options{cepext} = "." . $options{cepext};
    if(!defined $options{rawdir})  {
	die "directory of raw files are not defined in s3_lp, please specify it using -rawdir. \n"
    };
    if(defined $options{cepdir}){printf("-cepdir is not supported in livepretend\n");}

    push @cmdoptions, get_cmd_options(\%options, \@s3xopts, \%s3xmap);
    push @cmdoptions, -ctlcount => $ctlcount if defined($ctlcount);
    push @cmdoptions, -ctloffset => $ctloffset if defined($ctloffset);
    push @cmdoptions, -hyp => $matchfile;
    push @cmdoptions, -hypseg => $matchsegfile if defined($options{matchseg});
    push @cmdoptions, -outlatdir => $outlatdir if defined($options{lattice});
    open(TMPCONFIG, ">$options{logdir}/tmpconfig.$$")
	||die "cannot open a temporary config file\n";
    printf TMPCONFIG "@cmdoptions";
    close(TMPCONFIG);
    END { unlink "$options{logdir}/tmpconfig.$$" }
    @cmdoptions = ($options{ctlfile}, $options{rawdir}, "$options{logdir}/tmpconfig.$$");
}elsif(defined($options{s3_new})){
    # ARGH Sphinx3 has to be different
    $options{cepext} = "." . $options{cepext};
    foreach my $s3xopt (qw(pbeam maxhistpf maxhmmpf maxcdsenpf maxwpf vqeval dsratio
			   gsfile ci_pbeam cond_ds svqfile svq4svq Nlextree
			   subvqbeam gs4gs pl_window pl_beam pheurtype
			   wend_beam lmctlfile lmname ctl_lm)) {
	if (defined($options{$s3xopt})) {
	    print "-$s3xopt is not used with Sphinx 3.0\n";
	    delete $options{$s3xopt};
	}
    }
    push @cmdoptions, get_cmd_options(\%options, \@s3newopts, \%s3newmap);
    push @cmdoptions, -hyp => $matchfile;
    push @cmdoptions, -hypseg => $matchsegfile if defined($options{matchseg});
    push @cmdoptions, -outlatdir => $outlatdir if defined($options{lattice});
    push @cmdoptions, -ctlcount => $ctlcount if defined($ctlcount);
    push @cmdoptions, -ctloffset => $ctloffset if defined($ctloffset);
}elsif(defined($options{s2})){
    push @cmdoptions, get_cmd_options(\%options, \@s2opts, \%s2map);
    push @cmdoptions, -matchfn => $matchfile;
    push @cmdoptions, -matchsegfn => $matchsegfile if defined($options{matchseg});
    push @cmdoptions, -dumplatdir => $resultdir if defined($options{lattice});
    push @cmdoptions, -ctlcount => $ctlcount if defined($ctlcount);
    push @cmdoptions, -ctloffset => $ctloffset if defined($ctloffset);
    if (defined($options{agc})) {
	if ($options{agc} eq 'max') {
	    push @cmdoptions, -agcmax => 'TRUE';
	}
	elsif ($options{agc} eq 'emax') {
	    push @cmdoptions, -agcemax => 'TRUE';
	}
	elsif ($options{agc} eq 'noise') {
	    push @cmdoptions, -agcnoise => 'TRUE';
	}
    }
    if (defined($options{cmn})) {
	if ($options{cmn} eq 'current') {
	    push @cmdoptions, -normmean => 'TRUE';
	}
	elsif ($options{cmn} eq 'prior') {
	    push @cmdoptions, -nmprior => 'TRUE';
	}
	else {
	    push @cmdoptions, -normmean => 'FALSE';
	}
    }
}

if ($options{memchk}) {
    # A lousy hack to deal with libtoolism
    if (`file $options{bin}` =~ /shell script/) {
	unshift @cmdoptions, "--mode=execute", "valgrind",
	    "--tool=memcheck", "--leak-check=full", $options{bin};
	$options{bin} = File::Spec->catfile(dirname($options{bin}),
					    File::Spec->updir,
					    File::Spec->updir, "libtool");
    }
    else {
	unshift @cmdoptions, "--tool=memcheck", "--leak-check=full";
	$options{bin} = "valgrind";
    }
}
elsif ($options{debug}) {
    # A lousy hack to deal with libtoolism
    if (`file $options{bin}` =~ /shell script/) {
	unshift @cmdoptions, "--mode=execute", "gdb", "--args", $options{bin};
	$options{bin} = File::Spec->catfile(dirname($options{bin}),
					    File::Spec->updir,
					    File::Spec->updir, "libtool");
    }
    else {
	unshift @cmdoptions, "--args", $options{bin};
	$options{bin} = "gdb";
    }
}

if ($options{debug}) {
    print("$options{bin} @cmdoptions \n");
    system($options{bin}, @cmdoptions);
}
else {
    print("$options{bin} @cmdoptions >> $logfile \n")
	unless $options{silent};
    close(LOGFILE);
    system_log($logfile, $options{bin}, @cmdoptions);
}

ALIGNONLY:
if (!defined($options{part}) and defined($options{reffile})) {
    my $wordalign = dirname($0)."/word_align.pl";
    my $alignfile = "$resultdir/$options{expid}.align";

    if (defined($options{npart})) {
	open MATCHFILE, ">$matchfile" or die "Failed to open $matchfile: $!";
	if ($options{matchseg}) {
	    open MATCHSEGFILE, ">$matchsegfile" or die "Failed to open $matchsegfile: $!";
	}
	if ($options{matchconf}) {
	    open MATCHCONFFILE, ">$matchconffile" or die "Failed to open $matchconffile: $!";
	}
	for (my $i = 1; $i <= $options{npart}; ++$i) {
	    my $partfile = "$resultdir/$options{expid}.$i.match";
	    open PART, "<$partfile" or die "No match file $partfile: $!";
	    while (<PART>) {
		print MATCHFILE;
	    }
	    close PART;
	    if ($options{matchseg}) {
		my $partfile = "$resultdir/$options{expid}.$i.matchseg";
		open PART, "<$partfile" or die "No match file $partfile: $!";
		while (<PART>) {
		    print MATCHSEGFILE;
		}
		close PART;
	    }
	    if ($options{matchconf}) {
		my $partfile = "$resultdir/$options{expid}.$i.matchconf";
		open PART, "<$partfile" or die "No match file $partfile: $!";
		while (<PART>) {
		    print MATCHCONFFILE;
		}
		close PART;
	    }
	}
	close MATCHFILE;
	close MATCHSEGFILE if $options{matchseg};
	close MATCHCONFFILE if $options{matchconf};
    }
    unlink($alignfile);
    die "$matchfile is not created, Did you specify -npart?\n" unless -s $matchfile;
    if ($options{cer}) {
	system_log($alignfile, $wordalign, '-i', '-c', $options{reffile}, $matchfile)
    }
    else {
	system_log($alignfile, $wordalign, '-i', $options{reffile}, $matchfile)
    }
}

sub get_cmd_options {
    my ($options, $arglist, $argmap) = @_;

    my @cmdln;
    foreach my $opt (@$arglist) {
	my $optname = exists($argmap->{$opt}) ? $argmap->{$opt} : $opt;
	push @cmdln, "-$optname" => $options->{$opt} if defined($options->{$opt});
    }
    push @cmdln, split / /, $options->{other} if defined($options->{other});
    return @cmdln;
}

__END__

=head1 NAME

decodecep2.pl - Perform a decoding experiment using Sphinx 3.0's
s3decode_anytopo, Sphinx 3.X's decode, livepretend. 

=head1 SYNOPSIS

    decodecep2.pl [options]

    Executable Options:
     -bin     : The binary of the decoder.
     -help|h|? : For help. 

    Perl Script 
     
     -s3 : Specify the script to use s3_3's recognizer
     -s3_3 |-s3_X : Specify the script to use s3_X's recognizer
     -s3_lp : Specify the script to use s3_X's livepretend. 
     -expid : Experiment ID, users are compulsory to specify an experiment ID for every decoding experiments. 
     -npart   : The number of partitions of the ctl files.
     -part    : The partition of the ctl file which recognition will run on.
     -ctlfile : The control file.
     -cepext  : The extension of the cepstral file.
     -cepdir  : The directory for cepstral file.
     -rawdir  : The directory for raw wave file.
     -feat    : The feature type used by the acoustic model.
     -lattice : Output word lattices in addition to hypothesis file.
     -matchseg: Output word segmentation file as well as hypothesis.
     -reffile : Reference transcription file for calculating word error rate.
     -pbs     : For sites with PBS system installed, the scripts will automatically parallelize the decoding task. 
     -alignonly : Run alignment only. 

    Decoding 
     -acmoddir : Directory that stores the acoustic model parameters. It superseds the following parameters:
       -mean :  the mean vector. 
       -others are not command-line specified until it is needed. 

    s3 decoding options
     -topn : s3's topn
     -silpen : silence probability 
     -noisepen : noise probability
     -phonepen : phone penalty
	      
    s3_3 Speed-up related:
     -dsratio : Down-sampling ratio
     -cond_ds : Conditional down-sampling ratio
     -ci_pbeam : Beam effected on the CI phone set
     -svqfile : SVQ file
     -gs4gs : Is Gs for GS?
     -svq4svq : Is SVQ for SVQ?
     -subvqbeam : SVQ beam
     -vqeval : Number of sub-vector evaluated
     -lmctlfile : LM Control file
     -lmname : the LM to use by default
     -ctl_lm : the LM map
     -beam : normal hmm beam
     -pbeam : phone beam
     -wbeam : word beam
     -maxhistpf : max history per frame
     -maxhmmpf : max hmms per frame
     -maxwpf : max word per frame
     -maxcdsenpf : max cd senone per frame
     -memcheck : run memory checking
     -lminmemory: whether LM should be in memory.
     -Nlextree : # of lexical trees. 

   Other options are supposed to be private, you may want to read
   trace the script if you want to know them.

=head1 DESCRIPTION

The purpose of decodecep2.pl:

   To deal with the complexity of the command line of the sphinx 3
   generations of decoder include sphinx 3.0 slow, sphinx 3.X slow,
   fast and live mode decoder. This script was created to act as a
   unified wrapper of all these decoders. 

   The other reasons why a separate script is created is because CMU's
   researchers (faculties as well as students) tend not to record the
   dictionary with meticulosity.  That's why option -expid is
   incorporated in this script.  This urge every user to use different
   experiment ID. 
  
   The script is configurable by a separate config file. -config This
   allow experiment to be recorded easily.

=head1 AUTHORS

David Huggins-Daines <dhuggins@cs.cmu.edu>, Arthur Chan <archan@cs.cmu.edu>

=cut
