#!/usr/bin/perl

use strict;
use Getopt::Long;
use File::Basename;
use Pod::Usage;
use POSIX qw(setsid _exit);

Getopt::Long::Configure('bundling', 'no_getopt_compat', 'require_order',
			'prefix_pattern=(--|-)', 'no_ignore_case');

# This script converts from LSF format to PBS format and
# launches the job in the corresponding queue

my (@variables, @resources);
my ($junk, $efile, $ofile, @deps, $lsfdeps, $qtype, $name, $rerun, $excl, $cwd);
my ($runlocal, $foreground);
GetOptions('J=s' => sub { my ($opt, $arg) = @_; $name = substr $arg, 0, 15 },
	   'm=s' => sub { warn "You cannot specify machines.  -m option ignored" },
	   'R=s' => \$junk,
	   'e=s' => \$efile,
	   'o=s' => \$ofile,
	   'q=s' => \$qtype,
	   'W=s@' => \@deps,
	   'w=s' => \$lsfdeps,
	   'v=s' => \@variables,
	   'l=s' => \@resources,
	   'N=s' => sub { my ($opt, $arg) = @_; $name = substr $arg, 0, 15 },
	   'local' => \$runlocal,
	   'foreground' => \$runlocal,
	   'cwd=s' => \$cwd,
	   'r' => \$rerun,
	   'x' => \$excl,
	   'h|?|help' => sub { pod2usage(1) })
    or pod2usage(2);

if (defined($lsfdeps)) {
    $lsfdeps =~ s/\s*&&\s*/:/g;
    $lsfdeps = "depend=afterok:$lsfdeps";
}
$efile = "/dev/null" unless defined($efile);
$ofile = "/dev/null" unless defined($ofile);

unless (@ARGV) {
    pod2usage(2);
}

if ($qtype eq 'local') {
    $runlocal = 1;
}

my $prog = shift;
my $fnhead = basename($prog);
$cwd = dirname($prog) unless defined($cwd);

my $jobID;
if ($runlocal) {
    my $pid = fork;
    die "fork() failed: $!" unless defined($pid);
    if ($pid == 0) {
	chdir($cwd);
	setsid() unless $foreground;
	close STDIN;
	open STDERR, ">$efile" or die "Failed to open $efile: $!";
	open STDOUT, ">$ofile" or die "Failed to open $ofile: $!";

	my @pids;
	if (defined($lsfdeps)) {
	    $lsfdeps =~ s/^depend=afterok://;
	    push @pids, split /:/, $lsfdeps;
	}
	if (@deps) {
	    foreach (@deps) {
		s/^depend=afterok://;
		push @pids, split /:/;
	    }
	}
	if (@pids) {
	    my $alive = @pids;
	    while ($alive > 0) {
		$alive = kill(0, @pids);
		sleep(1);
	    }
	}

	exec $prog, @ARGV;
	_exit(127);
    }
    else {
	$jobID = $pid;
	if ($foreground) {
	    waitpid $pid, 0;
	    die "Job $pid failed with status $?" if $?;
	}
    }
}
else {
    # Make temporary script file containing job (UNSAFE!)
    my $fn = "/tmp/$fnhead.$$.sh";
    open FN, ">$fn"
	or die "Failed to open temporary file $fn: $!\n";
    END { unlink $fn }
    $SIG{__DIE__} = $SIG{INT} = sub { unlink $fn };
    print FN "#!/bin/sh\n";

    print FN "cd \"$cwd\"\n";
    print FN "\"$prog\" \\\n";
    foreach (@ARGV) {
	print FN "\t\"$_\" \\\n";
    }
    print FN "\nexit 0\n";
    close FN or die "Failed to close temporary file $fn: $!\n";
    chmod $fn, 0777;

    # Make argument list for qsub
    my @Qargs = (-e => $efile, -o => $ofile);
    push(@Qargs, -W => $_) foreach @deps;
    push(@Qargs, -q => $qtype) if defined($qtype);
    push(@Qargs, -W => $lsfdeps) if defined($lsfdeps);
    push(@Qargs, -N => $name) if defined($name);
    push(@Qargs, -r => ($rerun ? 'y' : 'n'));
    foreach (@resources) {
	push @Qargs, -l => $_;
    }
    foreach (@variables) {
	push @Qargs, -v => $_;
    }
    push(@Qargs, -l => 'nodes=1') if $excl;

    # Quote them
    foreach (@Qargs) {
	$_ = quotemeta($_);
    }
    #print STDERR "/usr/pbs/bin/qsub @Qargs $fn\n";
    $jobID = `/usr/pbs/bin/qsub @Qargs $fn`;
    # To make output compatible with script, the output has to be
    # Job <jobID>
    chomp $jobID;
    $jobID =~ s/\..*//g;
}
print "Job <$jobID>\n";

__END__

=head1 NAME

qsub.pl - Generic interface to queues (emulates LSF queue)

=head1 SYNOPSIS

 qsub.pl <-l resources> <-W dependencies> <-e errfile>
	 <-o outfile> program arguments

=head1 ARGUMENTS

=over 4

=item B<-J> I<job_name>

=item B<-e> I<err_file>

=item B<-o> I<output_file>

=item B<-W> I<PBS_attributes>

=item B<-w> I<dependencies>

=item B<-v> I<variable> ...

=item B<-l> I<resource> ...

=item B<-N> I<job_name>

=item B<--local>

=item B<--foreground>

=item B<-r>

=item B<-x>

=back

=head1 DESCRIPTION

=head1 AUTHORS

Written by someone, maybe Rita Singh?  Modified and documented by
David Huggins-Daines <dhuggins@cs.cmu.edu>

=cut
