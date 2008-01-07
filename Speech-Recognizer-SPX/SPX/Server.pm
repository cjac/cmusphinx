# -*- cperl -*-
use strict;

# Speech::Recognizer::SPX::Server: Perl module for writing PocketSphinx
# streaming audio servers.

# Copyright (c) 2000 Cepstral LLC.
#
# This module is free software; you can redistribute it and/or modify
# it under the same terms as Perl itself.
#
# Written by David Huggins-Daines <dhuggins@cs.cmu.edu>

package Speech::Recognizer::SPX::Server;
use Speech::Recognizer::SPX qw(:fbs :uttproc $SPHINXDIR);
use Audio::SPX;
use Time::HiRes qw(usleep);
use Fcntl;
use Errno;

use vars qw($VERSION);
$VERSION=0.03;
my %defaults = ( -samprate	=> 16000,
		 -adcin	=> 'TRUE',
		 -lm		=> "$SPHINXDIR/model/lm/turtle/turtle.lm",
		 -dict 	=> "$SPHINXDIR/model/lm/turtle/turtle.dic",
		 -hmm  	=> "$SPHINXDIR/model/hmm/wsj1" );

sub init {
    my ($this, $args, $sock, $log, $verbose) = @_;
    my $class = ref $this || $this;

    unless (defined $log) {
	local *LOG;
	open LOG, ">&STDERR" or die "can't dup: $!";
	$log = *LOG;
	# Readjust die() so we actually see messages ;(
	$SIG{__DIE__} = sub { print $log @_; exit $! || ($? >> 8) || 255 };
    }

    print $log "initializing pocketsphinx\n" if $log;
    my @argv = (%defaults, %$args, -verbose => $verbose);
    {
	my $i = 0;
	foreach (@argv) {
	    unless ($i++ % 2) {
		# Make argument names look like arguments to sphinx
		$_ = "-$_" unless $_ =~ /^-/;
	    }
	}
    }
    my %argv = @argv;
    @argv = %argv;
    fbs_init(\@argv)
	or return undef;

    my $sps = $args->{-samprate} || $defaults{-samprate};
    my $self = { sock => $sock, log => $log,
		 sps => $sps, sockflags => 0,
		 timeout => 1000,
		 verbose => $verbose };

    bless $self, $class;
}

sub sock { return $_[1] ? ($_[0]->{sock} = $_[1]) : $_[0]->{sock} }
sub logfh { return $_[1] ? ($_[0]->{log} = $_[1]) : $_[0]->{log} }
sub timeout { return $_[1] ? ($_[0]->{timeout} = $_[1]) : $_[0]->{timeout} }

sub calibrate {
    my ($self) = @_;
    my ($sock, $sps, $log, $verbose) =
	@{$self}{qw/sock sps log verbose/};

    print $log "calibrating\n" if $log;

    # Use blocking mode for calibration
    print $log "initializing continuous audio\n" if $log;
    my $res = undef;
    my $cad = Audio::SPX::Continuous->init_nbfh($sock, $sps);
    if (defined($cad)) {
	$res = $cad->calib;
	$self->{cad} = $cad;
    }

    return $res;
}

sub next_utterance {
    my ($self, $cb_listen, $cb_not_listen, $audio_fh) = @_;

    my ($cad, $sock, $sps, $log, $verbose) =
	@{$self}{qw/cad sock sps log verbose/};

    my $adbuf = "";

    # Use non-blocking mode for reading data (FIXME: should use select really...)
    fcntl $sock, F_GETFL, $self->{sockflags};
    fcntl $sock, F_SETFL, $self->{sockflags} | O_NONBLOCK;

    print $log "waiting for audio\n" if $log;
    my $s;
    while (defined($s = $cad->read($adbuf, 2048)) && $s == 0) {
	usleep 50_000;
    }
    goto failure unless defined $s;

    my $ts = $cad->read_ts;
    print $log "listening at $ts\n" if $log;
    $cb_listen->($ts) if defined $cb_listen;

    print $audio_fh $adbuf if defined $audio_fh;

    uttproc_begin_utt() or goto failure;
    uttproc_rawdata($adbuf, 0) or goto failure;

    while (1) {
	$adbuf = "";
	$s = $cad->read($adbuf, 2048) or goto failure;
	if ($s == 0) {
	    last if $cad->read_ts - $ts > int($sps * $self->{timeout} / 1000);
	    usleep(20_000);
	} else {
	    $ts = $cad->read_ts;
	    print $audio_fh $adbuf if defined $audio_fh;
	    my $rem = uttproc_rawdata($adbuf, 0)
		or goto failure;
	}
    }

    $cad->reset;
    fcntl $sock, F_SETFL, $self->{sockflags};

    print $log "done listening at $ts\n" if $log;
    $cb_not_listen->($ts) if defined $cb_not_listen;

    uttproc_end_utt() or goto failure;

    goto failure
	unless (my ($fr, $hyp) = uttproc_result(1));

    print $log "text is $hyp\n" if $log;
    return $hyp;

 failure:
    # Need to restore this for the caller
    fcntl $sock, F_SETFL, $self->{sockflags};
    return undef;
}

sub fini {
    fbs_end();
}

1;
__END__

=head1 NAME

Speech::Recognizer::SPX::Server - Perl module for writing streaming audio speech recognition servers using PocketSphinx

=head1 SYNOPSIS

  my $sock = new IO::Socket(... blah blah blah ...);
  my $log = new IO::File('server.log');
  my $audio_fh = new IO::File('speech.raw');
  my $srvr
      = Speech::Recognizer::SPX::Server->init({ -arg => val, ... }, $sock, $log, $verbose)
        or die "couldn't initialize pocketsphinx: $!";

  my $client = new IO::Socket;
  while (accept $sock, $client) {
      next unless fork;
      $srvr->sock($client);
      $srvr->calibrate or die "couldn't calibrate audio stream: $!";
      while (!$done && defined(my $txt
			= $srvr->next_utterance(sub { print $log "listening\n" },
						sub { print $log "not listening\n },
						$audio_fh))) {
	  print "recognized text is $txt\n";
	  ...
      }
      $srvr->fini or die "couldn't shut down server: $!";
      exit 0;
  }

=head1 DESCRIPTION

This module encapsulates a bunch of the stuff needed to write a
PocketSphinx server which takes streaming audio as input on an arbitrary
filehandle.  It's not meant to be flexible or transparent - if you
want that, then read the code and write your own server program using
just the Speech::Recognizer::SPX module.

The interface is vaguely object-oriented, but unfortunately it is
presently not possible to create multiple instances of
Speech::Recognizer::SPX::Server within the same process, due to severe
limitations of the underlying PocketSphinx library.  You can, however,
create multiple distinct servers with judicious use of C<fork>, as
shown in the example above.

It is possible that this will be fixed in a future release of PocketSphinx.

=head1 METHODS

=over 4

=item C<init>

  my $srvr = Speech::Recognizer::SPX::Server->init(\%args, $sock, $log, $verbose);

C<%args> is a reference to a hash of argument => value pairs, exactly
like the arguments you would pass on the command line to one of the
sphinx example programs.  Argument names can be given either with or
without a leading dash.

C<$sock> is a socket or other filehandle (could be anything, really)
on which the server will read audio data.  This argument is optional
and not needed to initialize the server - you can set it later with
the C<sock> accessor.

C<$log> is a filehandle on which the server module will log messages.
This argument is optional.  Without a filehandle to log on, these
messages (boring things like "started listening at $foo") will not be
printed.

C<$verbose> determines the verbosity level of the Sphinx library.
Currently, due to limitations in the PocketSphinx library, there are only
two options for this value, namely a true value for 'be insanely
verbose', or a false value for 'say nothing at all'.

=item C<calibrate>

  $srvr->calibrate;

Calibrates the noise threshold for the continuous audio stream
(i.e. figures out when it should listen and when it shouldn't).  This
requires you to actually have a ready and willing source of input on
the socket you set in C<init> or with C<sock>.

=item C<next_utterance>

  my $text = $srvr->next_utterance($cb_listen, $cb_not_listen, $audio_fh);

Waits for and recognizes the next utterance in the data stream.  All
arguments are optional:

C<$cb_listen> is a reference to (or name of, but I encourage you not
to do that) a subroutine to be called when the recognizer has detected
speech input.

C<$cb_not_listen> is a reference to (or name of) a subroutine to be
called when the recognizer has detected the end of speech input.

Obviously this presumes a request/response model for your application.
If you need to be able to get partial results then you'll have to wait
for me to support them (which will undoubtedly happen sooner or
later), or write your own module.  Sorry.

C<$audio_fh> is a filehandle to which to save the speech data - this
may come in handy for debugging, or if you would like to only record
the user talking and not the hours and hours of silence in between.

=item C<fini>

Shuts down the PocketSphinx recognizer.  Doesn't close the socket or
anything though, you have to do that yourself.

=back

=head1 ACCESSORS

=over 4

=item C<sock>

  my $sockfh = $srvr->sock;
  $srvr->sock(\*FOO);

Sets or gets the socket on which the server reads audio data.

=item C<logfh>

  my $logfh = $srvr->logfh;
  $srvr->log(\*BAR);

Sets or gets the filehandle on which the server logs messages (if it's
being verbose).

=item C<timeout>

  $srvr->timeout(200);

Sets/gets the amount of time (in milliseconds) to wait after the end
of speech-level input before processing an utterance.  Default is one
second.

=back

=head1 SEE ALSO

perl(1), L<Speech::Recognizer::SPX>.

=head1 AUTHOR

David Huggins-Daines <dhuggins@cs.cmu.edu>

=cut
