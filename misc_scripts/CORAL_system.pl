# -*- cperl -*-
#
# rmdata.pl: Routines for managing RM speaker-dependent data for adaptation experiments
use strict;
use lib dirname($0)."/modules/Config-General-2.24/blib/lib/";
use Config::General;

# Verify that the scripts is run under single machine mode.  Currently
# if user is running using RunAll.pl, he can only use single mode and
# pbs mode because these are cases one can control the dependency.
sub limit_of_this_scripts
{
  my ($conf)=@_;
  my ($hardware_mode,$no_machines,$hmm_type,$script_name);

  my %cfghash;
  %cfghash=$conf->getall;

  $hardware_mode=$cfghash{"Train"}{"CFG_HARDWAREMODE"};
  $no_machines  =$cfghash{"Train"}{"CFG_NO_MACHINES"};
  $hmm_type     =$cfghash{"Train"}{"CFG_HMM_TYPE"};
  
  printf("$hardware_mode $no_machines $hmm_type");

  if($no_machines > 1 || $hardware_mode eq "pbs" || $hardware_mode eq "ssh" ){
    die("Number of machines used for training is $no_machines,
    currently, ${script_name} cannot be used for training in using
    mechanism \"pbs\" or mechanism \"ssh\"");
  }
  if($hmm_type eq "semi"){
    die("Currently ${script_name} cannot support semi-continuous HMM training\n");
  }
}

sub system_log {
    my $logfile = shift;
    print "@_\n";
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
	    print "Appending output to $logfile\n";
	    open STDOUT, ">>$logfile";
	    open STDERR, ">&STDOUT";
	}
	exec @_;
	die "exec failed: $!";
    }
}

sub checkSphinxTrainConfig{
  my ($config, $defaultconfig)=@_;
  if(defined $config) {
    die "-config specified, but unable to find file $config\n" unless(-s $config);
  }else{
    if(defined $defaultconfig){
    die "Unable to find Default config file $defaultconfig\n" unless(-s $defaultconfig);
    }
  }
}

sub ST_mkdir{
  my ($dir)=@_;
  #Use persimmistic permission for the perl script. 
  mkdir "$dir",0755 unless -d "$dir";
}
1;
