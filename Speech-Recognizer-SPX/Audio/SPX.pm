use strict;

# Audio::SPX:  Perl interface to the Sphinx audio library.

# Copyright (c) 2000 Cepstral LLC.
#
# This module is free software; you can redistribute it and/or modify
# it under the same terms as Perl itself.
#
# Written by David Huggins-Daines <dhuggins@cs.cmu.edu>

package Audio::SPX;
use Carp;
use vars qw($VERSION $AUTOLOAD @ISA @EXPORT_OK);

require Exporter;
require DynaLoader;

@ISA=qw(Exporter DynaLoader);
@EXPORT_OK=qw(
	      AD_EOF
	      AD_ERR_GEN
	      AD_ERR_NOT_OPEN
	      AD_ERR_WAVE
	      AD_OK
	      AD_SAMPLE_SIZE
	      DEFAULT_SAMPLES_PER_SEC
	      );

$VERSION=0.02;

sub AUTOLOAD {
    # This AUTOLOAD is used to 'autoload' constants from the constant()
    # XS function.  If a constant is not found then control is passed
    # to the AUTOLOAD in AutoLoader.

    my $constname;
    ($constname = $AUTOLOAD) =~ s/.*:://;
    croak "& not defined" if $constname eq 'constant';
    my $val = constant($constname, @_ ? $_[0] : 0);
    if ($! != 0) {
	if ($! =~ /Invalid/) {
	    croak "Undefined subroutine $AUTOLOAD";
	}
	else {
	    croak "Your vendor has not defined Audio::SPX macro $constname";
	}
    }
    no strict 'refs';
    if ($] >= 5.00561) {
	*$AUTOLOAD = sub () { $val };
    } else {
	*$AUTOLOAD = sub { $val };
    }
    goto &$AUTOLOAD;
}

bootstrap Audio::SPX $VERSION;

1;
__END__

=head1 NAME

Audio::SPX - Perl interface to the Sphinx audio library.

=head1 SYNOPSIS

  use Audio::SPX;
  my $ad = Audio::SPX->open_sps(16000);

  $ad->start_rec or die "start_rec failed";
  $ad->stop_rec or die "stop_rec failed";
  my $samples = $ad->read($buf, $max);

  my $cad = Audio::SPX::Continuous->init($ad);
  my $cad = Audio::SPX::Continuous->init_nbfh($fh, $sps);
  my $cad = Audio::SPX::Continuous->init_raw($fh, $sps);

  $cad->calib;
  $cad->calib_loop($buf);
  $cad->set_thresh($sil, $sp);
  $cad->set_params($delta_sil, $delta_speech, $min_noise,
		   $max_noise, $winsize, $speech_onset,
		   $sil_onset, $leader, $trailer);
  my ($delta_sil, $delta_speech, $min_noise, $max_noise,
      $winsize, $speech_onset, $sil_onset, $leader, $trailer)
         = $cad->get_params;

  # If init_raw was used, this will consume the data in $buf, then
  # write back any non-slience data.  Yes, this feature is
  # undocumented in the Sphinx headers.  Yes, it's very useful.
  my $samples = $cad->read($buf, $max);

  $cad->reset;
  $cad->detach;
  $cad->attach($ad);
  $cad->read_ts;
  $cad->set_logfp(\*FH);

=head1 DESCRIPTION

Warning!  This interface is suboptimal and is therefore probably going
to change, both in the Perl module and the underlying library.

=head1 BUGS

The only supported sample rate for Audio::SPX is 16kHz
(Audio::SPX::Continuous should be fine with others).  init_sps() will
simply fail rudely if you try something else... which means it isn't
really very useful.  I suggest either opening the audio device
yourself, setting non-blocking mode (beware, some sound drivers don't
like this...) and passing it to the C<init_nbfh> method in
C<Audio::SPX::Continuous>, or using C<init_raw> and managing the audio
device yourself.

There isn't enough documentation yet, partly because the API is
somewhat in flux, and partly because I haven't figured out what some
of this stuff does either :-)

=head1 AUTHOR

David Huggins-Daines <dhuggins@cs.cmu.edu>

=head1 SEE ALSO

perl(1), L<Speech::Recognizer::SPX>

=cut
