#!/usr/bin/perl -w
use strict;
use FileHandle;

die "Usage: $0 CONTROL TRANSCRIPT OUTDIR [ADAPT_MEETING TEST_MEETING SDSPEAKER]\n" unless @ARGV >=3;
my ($ctlfile, $transfile, $outdir, $adapt_meeting, $test_meeting, $sdspeaker) = @ARGV;

system('mkdir', -p => $outdir);
open CTLFILE, "<$ctlfile" or die "Failed to open $ctlfile: $!";
open TRANSFILE, "<$transfile" or die "Failed to open $transfile: $!";
if (defined($sdspeaker)) {
    open SITRAINCTL, ">$outdir/sitrain.ctl" or die "Failed to open $outdir/sitrain.ctl: $!";
    open SITRAINTRANS, ">$outdir/sitrain.lsn" or die "Failed to open $outdir/sitrain.lsn: $!";
    open SDTRAINCTL, ">$outdir/sdtrain.ctl" or die "Failed to open $outdir/sdtrain.ctl: $!";
    open SDTRAINTRANS, ">$outdir/sdtrain.lsn" or die "Failed to open $outdir/sdtrain.lsn: $!";
}

my (%adapt_ctl, %adapt_trans, %test_ctl, %test_trans);
my $i;
while (<CTLFILE>) {
    my ($basename, $sf, $ef, $uttid) = split;
    my ($dvd, $meeting, $channel, $speaker, $qqq, $st, $et) = split /_/, $uttid;
    my $trans = <TRANSFILE>;
    my ($tuttid) = ($trans =~ /\(([^)]+)\)$/);
    die "Control file and transcript mismatch: $uttid != $tuttid" unless $uttid eq $tuttid;

    $i++;
    if ((!defined($adapt_meeting) and ($i%4 == 0))
	or (defined($adapt_meeting) and $meeting eq $adapt_meeting)) {
	unless (defined $adapt_ctl{$speaker}) {
	    $adapt_ctl{$speaker} = FileHandle->new(">$outdir/$speaker.adapt.ctl")
		or die "Failed to open $outdir/$speaker.adapt.ctl: $!";
	    $adapt_trans{$speaker} = FileHandle->new(">$outdir/$speaker.adapt.lsn")
		or die "Failed to open $outdir/$speaker.adapt.lsn: $!";
	}
	print {$adapt_ctl{$speaker}} $_;
	print {$adapt_trans{$speaker}} $trans;
    }
    elsif (!defined($adapt_meeting) or $meeting eq $test_meeting) {
	unless (defined $test_ctl{$speaker}) {
	    $test_ctl{$speaker} = FileHandle->new(">$outdir/$speaker.test.ctl")
		or die "Failed to open $outdir/$speaker.test.ctl: $!";
	    $test_trans{$speaker} = FileHandle->new(">$outdir/$speaker.test.lsn")
		or die "Failed to open $outdir/$speaker.test.lsn: $!";
	}
	print {$test_ctl{$speaker}} $_;
	$trans =~ s,<[^>]+>\s*,,g;
	$trans =~ s,\(\d+\),,g;
	print {$test_trans{$speaker}} $trans;
    }
    elsif (defined($sdspeaker)) {
	if ($speaker eq $sdspeaker) {
	    print SDTRAINCTL;
	    print SDTRAINTRANS $trans;
	}
	print SITRAINCTL;
	print SITRAINTRANS $trans;
    }
}

close CTLFILE;
close TRANSFILE;
if (defined($sdspeaker)) {
    close SDTRAINCTL;
    close SDTRAINTRANS;
    close SITRAINCTL;
    close SITRAINTRANS;
  }
%adapt_ctl = ();
%adapt_trans = ();
%test_ctl = ();
%test_trans = ();
