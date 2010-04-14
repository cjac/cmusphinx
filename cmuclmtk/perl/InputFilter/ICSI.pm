# -*- cperl -*-

# Copyright (c) 2006 Carnegie Mellon University.  All rights
# reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#
# This work was supported in part by funding from the Defense Advanced
# Research Projects Agency and the National Science Foundation of the
# United States of America, and the CMU Sphinx Speech Consortium.
#
# THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND
# ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
# NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

use strict;
package Text::CMU::InputFilter::ICSI;
use IO::File;
use XML::Parser;
use base 'Text::CMU::InputFilter';

sub initialize {
    my ($self, %opts) = @_;

    $self->SUPER::initialize(%opts);
    $self->{fps} = defined($opts{fps}) ? $opts{fps} : 105;
    $self->{tagstack} = [];
}

sub process_transcript {
    my ($self, $transfile) = @_;

    $self->{parser} =
	 XML::Parser->new(Handlers => { Start => sub { StartTag($self, @_) },
					End   => sub { EndTag($self, @_) },
					Char  => sub { Text($self, @_) }});

    $self->{parser}->parsefile($transfile);
    return 1;
}

use vars qw(%StartHandlers %EndHandlers);
%StartHandlers = (
		  Channel => sub {
		      my ($self, $expat, $elem, %attr) = @_;
		      # Record the channel to microphone ID mapping
		      $self->{MICID}{$attr{Name}} = $attr{Mic};
		  },
		  Participant => sub {
		      my ($self, $expat, $elem, %attr) = @_;
		      # Record the participant to channel mapping
		      $self->{Channel}{$attr{Name}} = $attr{Channel};
		  },
		  Transcript => sub {
		      my ($self, $expat, $elem, %attr) = @_;
		      $self->{startTime}=int($attr{StartTime}*$self->{fps});
		      #HACK! To avoid potential spurious frames at the end.
		      #HACK! This is just for safety.
		      $self->{endTime}=int($attr{EndTime}*$self->{fps})-3;
		  },
		  Meeting => sub {
		      my ($self, $expat, $elem, %attr) = @_;
		      # Record the meeting ID
		      $self->{MeetingID} = $attr{Session};
		  },
		  Segment => \&StartSegment,
		 );
%EndHandlers = (
		Segment => \&EndSegment,
	       );

sub StartTag {
    my($self, $expat, $elem) = @_;
    my $stack = $self->{tagstack};
    push @$stack, $elem;
    if (exists $StartHandlers{$elem}) {
	$StartHandlers{$elem}->(@_);
    } else {
	StartMisc(@_);
    }
}

sub EndTag {
    my($self, $expat, $elem) = @_;
    my $stack = $self->{tagstack};
    my $e = pop @$stack;
    warn "Malformed XML, element $e != $elem" unless $e eq $elem;

    if (exists $EndHandlers{$elem}) {
	$EndHandlers{$elem}->(@_);
    } else {
	EndMisc(@_);
    }
}

sub StartSegment {
    my ($self, $expat, $elem, %attr) = @_;
    my($sid, $chan);
    my ($st, $et);

    my $micid = "none";
    # Figure out which channel to use.
    # If the Segment has a Channel attribute, just use it
    if (exists($attr{Channel})) {
	$chan = $attr{Channel};

	# If the Segment does NOT have a Channel attribute, but does
	# have a Participant, and CloseMic is true, use the default
	# channel for the participant.

    } elsif (exists($attr{Participant}) && 
	     (!exists($attr{CloseMic}) || $attr{CloseMic} eq "true")) {
	$chan = $self->{Channel}{$attr{Participant}};
	$micid = $self->{MICID}{$chan};
	if (!$micid) {
	    printf(STDERR "Microphone ID not found.");
	}
    } else {
	# Otherwise, just use "far"
	# Far field channels are ignored.
	$chan = 'far';
	$self->{isIgnore}=1;
    }

    # Get the Participant attribute, or "none" if none provided.
    if (exists($attr{Participant})) {
	$sid = $attr{Participant};
    } else {
	$sid = "none";
    }

    $st=$attr{StartTime};
    $et=$attr{EndTime};
    $self->{Header} = "$self->{MeetingID}/${chan}_${sid}_${micid}_${st}_${et}";
    $self->{StartTime} = $st;
    $self->{EndTime} = $et;
    $self->{ChannelID} = $chan;

    # Start collecting up text
    $self->{InSegment} = 1;
    $self->{TextSoFar} = '';

    # Any other tag is passed verbatim if we're in a <Segment> ... </Segment>
}

sub StartMisc {
    my ($self, $expat, $elem, %attr) = @_;
    my $opts = $self->{opts};

    #This routine takes care of the tags inside the <Segment> tag.
    #This is the current rule
    # -when <Foreign> <Pronounce> and <Uncertain> tags were encountered, 
    # the sentence is ignored. 
    # -when <Emphasis> is encountered, the tag is ignored. 
    # -when <Pause> is encountered, the tag is replaced by <sil>
    # -when <Comment>/<VocalSound>/<NonVocalSound> are encountered,
    # they are handled with specific rules encoded in the rule file(s). 
    #
    #           Uncomment this line to debug
    #	    $self->{TextSoFar} .= " " . $expat->original_string();
    if ($elem eq "Comment") {
	#Tag Comment handling
	$self->{TextSoFar} .= " " ;
	if ($attr{"Description"} ne "Digits") {
	    $self->{isIgnore}=1;
	}
    } elsif ($elem eq "Emphasis") {
	#Tag Emphasis handling
	#Skip the tag. 
	$self->{TextSoFar} .= "" ;
    } elsif ($elem eq "Foreign") {
	#Tag Foreign handling
	$self->{TextSoFar} .= " " . $expat->original_string();
	$self->{isIgnore}=1;
    } elsif ($elem eq "Pause") {
	#Tag Pause handling
	$self->{TextSoFar} .= " " . "<sil>";
    } elsif ($elem eq "Pronounce") {
	#Tag Pronounce handling
	$self->{TextSoFar} .= " " ;
    } elsif ($elem eq "Uncertain") {
	#Tag Uncertain handling
	$self->{TextSoFar} .= " " ;
    } elsif ($elem eq "VocalSound" or $elem eq "NonVocalSound") {
	# Filler word handling
	my $label = $self->{FillerMap}{$attr{"Description"}};
	if (defined($label)) {
	    $self->{TextSoFar} .= " " . $label;
	} else {
	    warn "No corresponding label for <VocalSound> with description $attr{Description}";
	    $self->{isIgnore}=1;
	}
    }
}

sub EndSegment {
    my($self, $expat, $elem) = @_;
    my $opts = $self->{opts};

    # Clean everything up and print it out
    $self->{TextSoFar} =~ s/\n/ /g;
    $self->{TextSoFar} =~ s/\s\s+/ /g;
    $self->{TextSoFar} =~ s/^\s+//;
    $self->{TextSoFar} =~ s/\s+$//;
    if ($self->{TextSoFar} !~ /^\s*$/ && #only if file is not empty
	!$self->{isIgnore} #there are no special tags that caused us ignore this string. 
       ) {
	# Do text normalization now.
	my @words = $self->text_norm($self->{TextSoFar});
	# And output an utterance
	$self->output_sentence(\@words, $self->{Header},
			       "$self->{MeetingID}/$self->{ChannelID}",
			       $self->{StartTime}, $self->{EndTime})
	    if @words;
    }
    $self->{InSegment} = 0;
    $self->{isIgnore} = 0 ;
    $self->{TextSoFar} = '';
}

sub EndMisc {
    my ($self, $expat, $elem) = @_;
    my $opts = $self->{opts};
    if ($self->{InSegment}) {
	if ($elem eq "Comment") {
	    #Tag Comment handling
	    $self->{TextSoFar} .= " " . $expat->original_string();
	} elsif ($elem eq "Emphasis") {
	    #Tag Emphasis handling
	    #Skip the tag. 
	    $self->{TextSoFar} .= "" ;
	} elsif ($elem eq "Foreign") {
	    #Tag Foreign handling
	    $self->{TextSoFar} .= " " . $expat->original_string();
	} elsif ($elem eq "Pause") {
	    #Tag Pause handling
	    $self->{TextSoFar} .= " " . $expat->original_string();
	} elsif ($elem eq "Pronounce") {
	    #Tag Pronounce handling
	    $self->{TextSoFar} .= " " ;
	} elsif ($elem eq "Uncertain") {
	    #Tag Uncertain handling
	    $self->{TextSoFar} .= " " ;
	} elsif ($elem eq "VocalSound") {
	    #Tag VocalSound handling.
	    $self->{TextSoFar} .= " " . $expat->original_string();
	} elsif ($elem eq "NonVocalSound") {
	    #Tag NonVocalSound handling
	    $self->{TextSoFar} .= " " . $expat->original_string();
	}
    }
}

#WARNING BY ARCHAN!!!!!
#DON'T DO TEXT NORMALIZATION IN THIS FUNCTION!!!!!!
#$string can be some incomplete characters.
#Only thing I have done in this function is to make sure text with tags are correct. 
#That proves to be tricky.
sub Text {
    my($self, $expat, $string) = @_;
    my $elem=$expat->current_element();
    my $opts = $self->{opts};
    if ($self->{InSegment}) {
	if ($elem eq "Uncertain" || $elem eq "Pronounce") {
	    $string =~ s/^\s+//;
	    $string =~ s/\s+$//;

	    my @words=split(/\s+/,$string);
	    foreach my $word (@words) {
		$self->{TextSoFar} .= " ++GARBAGE++ " . " " ;
	    }
	} elsif ($elem eq "Emphasis") {
	    $string =~ s/^\s+//;
	    $string =~ s/\s+$//;
	    $self->{TextSoFar} .= $string  ;
	} else {
	    $self->{TextSoFar} .= $string  ;
	}
    }
}

sub NormalizeWord{
    my ($self, $word) =@_;
    my $opts = $self->{opts};
    if($word eq "<sil>" || $word =~ /\+\+/){
	return "" unless $opts->{fillers};
    }
    #Get rid of comma, footstop, question mark and exclamation point.
    #Also '{' and '}'
    $word =~ s/,//g;
    $word =~ s/\.//g;
    $word =~ s/\!//g;
    $word =~ s/\?//g;
    $word =~ s/://g;
    #Get rid of asterisk symbol
    $word =~ s/\*//g;
    #Get rid of double approsothe
    $word =~ s/\"//g;

    # Delete stray '-'
    if ($word eq '-') {
	$word = "";
    }

    # ICSI has X_ for single letters which we claim is bogus
    if ($word =~ /^([A-Za-z])_$/) {
	$word = $1;
    }

    return $word;
}

sub text_norm{
    my($self, $string)=@_;
    $string=~s/^\s+//;
    $string=~s/\s+$//;

    my @words=split(" ",$string);
    my @outwords;
    my $norm_string = "";
    foreach my $word (@words){
	#This is the word normalization routine,
	#Entry point of text normalization.
	my $normword = $self->NormalizeWord($word);
	push @outwords, split(" ",$normword);
    }

    return @outwords;
}

1;
__END__

=head1 NAME

Text::CMU::InputFilter::ICSI - Input filter for ICSI format transcripts

=head1 SYNOPSIS

=head1 OPTIONS

=over 4

=item fillermap

A file listing a mapping of filler words from the input to filler
words in the output.  Each line has an input and an output separated
by whitespace.

=item dict

A dictionary to use for constructing a list of OOVs.

=item split

Whether to split compound words

=item partials

Whether to include partial words

=item filledpauses

Whether to include filled pauses

=item fillers

Whether to include filler words

=item noisy

Whether to include noise-only utterances

=back

=head1 AUTHORS

Arthur Chan E<lt>archan@cs.cmu.eduE<gt>, David Huggins-Daines
E<lt>dhuggins@cs.cmu.eduE<gt>.

=cut
