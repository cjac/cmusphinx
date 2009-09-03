#!/usr/bin/perl -w
## ====================================================================
##
## Copyright (c) 1996-2005 Carnegie Mellon University.  All rights 
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
use lib dirname($0);
use Config::General;
use Sys::Hostname;
use Getopt::Long;
use Pod::Usage;
require 'CORAL_system.pl';

my @argspec = ('expid=s@','npart=i',
	    'help|h|?','bin=s','logdir=s','config=s');

my %cmdargs;
GetOptions(\%cmdargs,@argspec) or pod2usage(1);
pod2usage(1) if $cmdargs{help};

unless (defined($cmdargs{config})) {
    print "The -config option is required.\n";
    pod2usage(1);
}

my $conf = new Config::General(
    -ConfigFile     => $cmdargs{config},
    -ExtendedAccess => 1,
    -InterPolateVars => 1,
);

my %defaults = ();
my %conf=$conf->getall;
my %options=(%defaults,%{$conf{ResPath}}, %cmdargs);

my @expArray=@{$options{expid}};


#Convert all s3 segment files to CTM
foreach my $exp (@expArray){
    print "Processing experiment $exp\n";
    s3seg2ctm($exp,$options{logdir},$options{npart});
}

#Combine all three results using rover
my $comresultdir="$options{logdir}/$options{comexpid}/";
my $comctmfile="$options{logdir}/$options{comexpid}/$options{comexpid}.ctm";
my $commatchfile="$options{logdir}/$options{comexpid}/$options{comexpid}.match";
my $comlogfile="$options{logdir}/$options{comexpid}/$options{comexpid}.rover.log";
mkdir($comresultdir);

#Not configurable at this moment because the rover setting is not yet
#set at this point.

my @rovercmdargs;
foreach my $exp (@expArray)
{
    my $resultdir="$options{logdir}/$exp";
    my $ctmfile = "$resultdir/$exp.ctm";
    push @rovercmdargs,"-h", "$ctmfile", "ctm";
}

$ENV{"MFALIGN_DBG"}=10;
push @rovercmdargs, "-o","$comctmfile";
#push @rovercmdargs,"-f","10";
push @rovercmdargs,"-m","avgconf";
push @rovercmdargs,"-a","1";
push @rovercmdargs,"-c","0.001";
push @rovercmdargs,"-T";
system_log($comlogfile,$options{rover},@rovercmdargs);

# ctm to s3 match file. 
ctm2match($options{ctlfile},$comctmfile,$commatchfile);

sub s3seg2ctm{
    use POSIX qw(strtod);
    my ($expid,$logdir,$npart)=@_;
    my $resultdir="$logdir/$expid";
    my $matchsegfile = "$resultdir/$expid.matchseg";
    my $logfile = "$resultdir/$expid.convert.log";
    my $s2segfile = "$resultdir/$expid.s2seg";
    my $ctmfile = "$resultdir/$expid.ctm";

    # 1, concatentate the matchseg files if parallel training was used. 
    print "\tConcatenate matchseg. \n";
    if (defined($npart)) {
	open MATCHFILE, ">$matchsegfile" or die "Failed to open $matchsegfile: $!";
	for (my $i = 1; $i <= strtod($npart); ++$i) {
	    my $partfile = "$resultdir/$expid.$i.matchseg";
	    open PART, "<$partfile" or die "No match file $partfile: $!";
	    while (<PART>) {
		print MATCHFILE ;
	    }
	    close PART;
	}
	close MATCHFILE;
    }
    
    system("wc -l $matchsegfile");
    my @cmdoptions;
      
    print "\tConvert s3 matchseg to s2 matchseg.\n";
    # 2, convert the s3 matchseg to s2 matchseg
    my $cmd = $options{s3seg_2_s2seg} . " " .  $matchsegfile . " " . $s2segfile;
    system_log($logfile,$options{s3seg_2_s2seg},$matchsegfile,$s2segfile);

    print "\tConvert s2 matchseg to ctm.\n";
    # 3, convert the s2 matchseg to a CTM file.
    system_log($logfile,$options{convert},$s2segfile,$ctmfile);
}

sub ctm2match {
    my ($ctl_file,$input_file,$output_file)=@_;
    my @lines;
    my %input_hash;
    my $uttid;
    
    open(IN,"<$input_file") || die "cannot open input file $input_file\n";
    @lines=<IN>;
    foreach my $line (@lines){
	my ($uttid,$chan,$st,$ed,$word,$score)=split(/\s+/,$line);
	$uttid = lc $uttid;
	if(defined $input_hash{$uttid}){
	    $input_hash{$uttid} .= " " . $word;
	}else{
	    $input_hash{$uttid} = $word;
	}
    }
    close(IN);
    
    
    open(CTL,"<$ctl_file") ||die "cannot open ctl file $ctl_file\n";
    open(OUT,">$output_file") || die "cannot open output file $output_file\n";
    @lines=<CTL>;
    chomp(@lines);
    foreach my $line (@lines){
	chomp($line);
	my $stub;
	($stub,$stub,$stub,$uttid)=split(/\s+/,$line);
	$uttid = lc $uttid;
	if(defined $input_hash{$uttid}){
	    printf(OUT "%s ",$input_hash{$uttid});
	}
	printf(OUT " (%s)\n",$uttid);
    }

    close(OUT);				 
    close(CTL);
}

