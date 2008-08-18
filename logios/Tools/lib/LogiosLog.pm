package LogiosLog;
#
# Simple-minded logging facility; takes care of creating a default log file
# [20071025] (air) Modified from code written by tkharris
# [20080304] (air) Added toggling
# [20080707] (air) make it do appends
#

my $LOGFILE = undef;
my $DEFAULT = "logios.log";
my $LOGGING = 0;

#END   {
#  if (defined $LOGFILE) {
#    print LOG "\n-----------     end log at ",scalar localtime,"----------\n";
#    close (LOG);
#  }
#}

# toggle the logging feature
sub control {
  my ($flag) = @_;
  if ($flag eq 2)  { $LOGGING =  2; }    # log to file only
  if ($flag eq 1)  { $LOGGING =  1; }    # log to STDERR and to file
  if ($flag eq 0)  { $LOGGING =  0; }    # log only to STDERR [default]
  if ($flag eq -1) { $LOGGING = -1; }    # stop all logging
  return $LOGGING;
}

sub open_logfile {
  my ($logf) = @_;
  if ( defined $LOGFILE and ($logf eq $LOGFILE)) { return 0; } # once is enough
  # open requested file
  $LOGFILE = $logf;
  open(LOG,">>$LOGFILE") or die "open_logfile(): can't open $LOGFILE\n";
  print LOG "\n-----------  start log at ",scalar localtime,"----------\n";

  return 1;
}

sub say {
  my ($type, @txt) = @_;

  if ((not ($LOGGING eq -1)) and (not ($LOGGING eq 2))) {print STDERR "$type> @txt$/"; }
  if ( not defined $LOGFILE ) { open_logfile ( $DEFAULT ); }
  print LOG "$type> @txt$/" if $LOGFILE;
}

sub info {
  my $message = shift;
  &say('INFO',$message);
}

sub warn {
  my $message = shift;
  &say('WARNING', $message);
}

sub fail {
  my $reason = shift;
  &say('FAIL', $reason);
  die "EXITING...\n";
}

####
 1;#
####
