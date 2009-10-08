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

# Command-line options
my @argspec = (
    'npart=i', 'part=i', 'expid=s', 'help|h|?', 'logdir=s',
    'config|cfg=s', 'bin=s', 'memchk', 'debug', 'silent', 'cer',
    'pbs', 'alignonly', 'ctlfile|ctl=s', 'reffile|ref=s',
    );

# Get options into a hash
my %cmdargs;
Getopt::Long::Configure qw(pass_through);
GetOptions(\%cmdargs, @argspec) or pod2usage(1);
pod2usage(0) if $cmdargs{help};
unless (defined($cmdargs{config})) {
    print "The -config or -cfg option is required.\n";
    pod2usage(1);
}
unless (defined($cmdargs{expid})) {
    print "Experiment ID is not specified. Please use -expid to specify it.\n";
    pod2usage(1);
}

# Try not to create unreadable files (but it might not work)
umask 0002;

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
my %defaults = (
		logdir => File::Spec->catdir($basedir, 'logdir'),
		);

my %conf = $conf->getall;
# Another weird quirk that should go away.
if (defined($conf{ResPath})) {
    %conf = %{$conf{ResPath}};
}
my %options = (%defaults, %conf, %cmdargs);
# A weird quirk that should go away
$options{acmoddir} = $options{ACMODDIR} if defined $options{ACMODDIR};
delete $options{ACMODDIR};
use Data::Dumper; print Dumper(\%options);

unless (defined($options{bin})) {
    print "Binary of the decoder is not specified.\nIn command line, you can specify it by -bin.\nIn config, you can specify it with the 'bin' option\n";
    pod2usage(1);
}

my $resultdir="$options{logdir}/$options{expid}";
mkpath($resultdir);
my $segdir = $resultdir;
$segdir .= ",CTL" if $options{ctlpath};
my $outlatdir = $resultdir;
$outlatdir .= ",CTL" if $options{ctlpath};

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
    # FIXME: This is an arbitrary list
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
    printf LOGFILE "cepdir %s doesn't exist\n" , $options{cepdir} ;
    printf("cepdir %s doesn't exist\n",$options{cepdir});
    exit(-1);
}

if(! -e $options{ctlfile}) {
    printf LOGFILE "ctlfile %s doesn't exist\n",$options{ctlfile};
    printf("ctlfile %s doesn't exist\n",$options{ctlfile});
    exit(-1);
}

# Build the basic set of command-line options.
my @cmdoptions;
push @cmdoptions, split ' ', $options{args} if defined($options{args});
push @cmdoptions, -ctlcount => $ctlcount if defined($ctlcount);
push @cmdoptions, -ctloffset => $ctloffset if defined($ctloffset);
push @cmdoptions, -hyp => $matchfile;
push @cmdoptions, -hypseg => $matchsegfile if defined($options{matchseg});
push @cmdoptions, -hypconf => $matchconffile if defined($options{matchconf});
push @cmdoptions, -outlatdir => $outlatdir if defined($options{lattice});

# Hack the cepext, because...
$options{cepext} = "." . $options{cepext} if $options{cepext} !~ /^\./;

# Map stuff from %options into the command-line
my @s3xopts = qw(mdef mean var mixw tmat dictfn fdictfn lw uw wip dither lda ldadim
		 feat agc varnorm ctlfile cepdir cmn beam pbeam wbeam ceplen op_mode
		 svqfile lmctlfile lmname vqeval ci_pbeam cond_ds svq4svq inlatdir latext
		 gs4gs pl_window pl_beam pheurtype wend_beam ctl_lm logbase
		 ctl_mllr Nlextree subvqbeam tighten_factor dsratio senmgau
		 rawext cepext adcin adchdr adcendian allphone compallsen
		 upperf lowerf nfilt wlen srate frate nfft cachesen silpen
		 fwdtree fwdflat bestpath fwdflatlw bestpathlw lmfile
		 fwdflatefwid fwdflatsfwin kdtree kdmaxbbi
		 sendumpfn fwdflatbeam fwdflatnwbeam noisepen acmoddir
	       );

my %s3xmap = (dictfn => 'dict', fdictfn => 'fdict', tmat => 'tmat', silpen => 'silprob',
	      gsfile => 'gs', svqfile => 'subvq', lmfile => 'lm', srate => 'samprate',
	      dsratio=> 'ds', ctlfile => 'ctl', lmctlfile => 'lmctlfn', noisepen => 'fillprob',
	      fwdflatnwbeam => 'fwdflatwbeam', acmoddir => 'hmm', sendumpfn => 'sendump');
push @cmdoptions, get_cmd_options(\%options, \@s3xopts, \%s3xmap);

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
    return @cmdln;
}

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

# FIXME: This is a CMU Sphinx-group specific quirk...
sub network_path {
    my ($host, $path) = @_;
    $path = File::Spec->rel2abs($path);
    $path =~ s,/.automount/([^/]+)/root,/net/$1,;
    if ($path !~ m,^/net/,) {
	$path = "/net/${host}$path";
    }
    return $path;
}


__END__

=head1 NAME

sphinx_eval.pl - Perform a decoding experiment using Sphinx.

=head1 SYNOPSIS

    sphinx_eval.pl [options]

    Executable Options:
     -help|h|? : For help.
     -cfg|config : The configuration file to use.
     -expid   : Experiment ID, users are compulsory to specify an experiment ID for every decoding experiments.
     -pbs     : For sites with PBS system installed, the scripts will automatically parallelize the decoding task.
     -npart   : The number of partitions of the ctl files.
     -part    : The partition of the ctl file which recognition will run on.
     -bin     : Path to the decoder binary
     -debug   : Run the decoder under GDB.
     -memcheck: Run the decoder under Valgrind's memcheck tool.
     -lattice : Output word lattices in addition to hypothesis file.
     -matchseg: Output word segmentation file as well as hypothesis.
     -cer     : Calculate character error rate (assuming UTF-8 transcripts)
     -alignonly : Run alignment only.

    Other options will be passed through to the decoder.

=head1 DESCRIPTION

=head1 AUTHORS

David Huggins-Daines <dhuggins@cs.cmu.edu>, Arthur Chan <archan@cs.cmu.edu>

=cut
