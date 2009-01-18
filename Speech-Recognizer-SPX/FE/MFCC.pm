use strict;

# Audio::MFCC: Perl module for computing Mel-Frequency Cepstral
# Coefficients from audio data

# Copyright (c) 2000 Cepstral LLC.
#
# This module is free software; you can redistribute it and/or modify
# it under the same terms as Perl itself.
#
# Written by David Huggins-Daines <dhuggins@cs.cmu.edu>

package Audio::MFCC;
use Carp;
use vars qw($VERSION $AUTOLOAD @ISA @EXPORT_OK);

require Exporter;
require DynaLoader;

@ISA=qw(Exporter DynaLoader);
@EXPORT_OK=qw(
	      BB_SAMPLING_RATE
	      DEFAULT_BB_FRAME_SHIFT
	      DEFAULT_BB_LOWER_FILT_FREQ
	      DEFAULT_BB_NUM_FILTERS
	      DEFAULT_BB_UPPER_FILT_FREQ
	      DEFAULT_FFT_SIZE
	      DEFAULT_FRAME_RATE
	      DEFAULT_NB_FRAME_SHIFT
	      DEFAULT_NB_LOWER_FILT_FREQ
	      DEFAULT_NB_NUM_FILTERS
	      DEFAULT_NB_UPPER_FILT_FREQ
	      DEFAULT_NUM_CEPSTRA
	      DEFAULT_PRE_EMPHASIS_ALPHA
	      DEFAULT_SAMPLING_RATE
	      DEFAULT_START_FLAG
	      DEFAULT_WINDOW_LENGTH
	     );
$VERSION=0.02;

sub AUTOLOAD {
    # This AUTOLOAD is used to 'autoload' constants from the constant()
    # XS function.  If a constant is not found then control is passed
    # to the AUTOLOAD in AutoLoader.

    my $constname;
    ($constname = $AUTOLOAD) =~ s/.*:://;
    print "constname is $constname\n";
    croak "& not defined" if $constname eq 'constant';
    my $val = constant($constname, @_ ? $_[0] : 0);
    if ($! != 0) {
	if ($! =~ /Invalid/) {
	    croak "Undefined subroutine $AUTOLOAD";
	}
	else {
	    croak "Your vendor has not defined Audio::MFCC macro $constname";
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

bootstrap Audio::MFCC $VERSION;

1;
__END__

=head1 NAME

Audio::MFCC - Perl module for computing mel-frequency cepstral coefficients

=head1 SYNOPSIS

  use Audio::MFCC;
  my $fe = Audio::MFCC->init(\%params)
  $fe->start_utt;
  my @ceps = $fe->process_utt($rawdata, $nsamps);
  my $leftover = $fe->end_utt;

=head1 DESCRIPTION

This module provides an interface to the Sphinx feature extraction
library which can be used to extract mel-frequency cepstral
coefficients from data.  These coefficients can then be passed to the
C<Speech::Recognizer::SPX::uttproc_cepdata> function.

You might find this useful if, for example, you wish to do the actual
recognition on a different machine from the audio capture, and don't
have the bandwidth to send a full stream of audio data over the
network.

Currently, Sphinx also uses delta and double-delta cepstral vectors
as input to its vector quantization module, but the calculation of
these values is done inside the recognizer's utterance processing
module..  In the future it may be possible to move the extraction of
these features into the feature extraction library, or to use entirely
different features as input (for example, LPC coefficients, though
currently, mel-scale cepstra give the best recognition performance).

=head1 INITIALIZATIONO

  my $fe = Audio::MFCC->init(\%params);

Initializes parameters for feature extraction, and return an object
which encapsulates the state of the extraction process.

The parameters are passed as a reference to a hash of parameter names
keyed to parameter values.  Available parameters include:

=over 4

=item C<sampling_rate>

Sampling rate at which the audio data to be processed was captured,
specified in samples per second.

=item C<frame_rate>

Number of frames of data to be processed per second of sampled audio.

=item C<window_length>

Size of the FFT window, in number of samples.

=item C<num_cepstra>

Number of cepstral coefficients to compute.

=item C<num_filters>

Number of filters to use for creating the mel-scale.

=item C<fft_size>

Frame size for FFT analysis (must be a power of 2).

=item C<lower_filt_freq>

Low end of filter band.

=item C<upper_filt_freq>

High end of filter band.

=item C<pre_emphasis_alpha>

Scaling factor for pre-emphasis of input audio data.

=back

=head1 OBJECT METHODS

=over 4

=item C<start_utt>

  $fe->start_utt or die "start_utt failed";

Prepares the C<$fe> object for cepstral extraction.  If it fails
(though I don't know why it would), it will return C<undef>.

=item C<process_utt>

  my @cepvectors = $fe->process_utt($rawdata, $nsamps);

Performs cepstral extraction on C<$nsamps> samples of audio data from
C<$rawdata>.  If any data is left over (under one frame), it will be
carried over to the next call to C<process_utt>, or analyzed and
returned by C<end_utt>.

Note that the audio data is currently B<always> represented as a
vector of 16-bit signed integers in native byte order.

Returns a list of array references, each of which points to the vector
of cepstral coefficients extracted from one frame of data.

=item C<end_utt>

  my $leftover = $fe->end_utt;

Finishes the processing of utterance data.  If there is any extra data
remaining to be processed, it will be padded with zeroes to a single
frame and cepstral extraction will be done, with the resulting vector
returned (as an array reference).  Otherwise, a false value is
returned.

=back

=head1 AUTHOR

David Huggins-Daines <dhuggins@cs.cmu.edu>

=head1 SEE ALSO

perl(1), L<Speech::Recognizer::SPX>

=cut
