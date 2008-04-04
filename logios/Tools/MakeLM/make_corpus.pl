#
# Create a synthetic corpus of training sentences from a Phoenix grammar
#

package MakeCorpus;
require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw (get_corpus);

# generate a sentence corpus from the grammar
sub get_corpus {
  my $grabsfile = shift;
  my $corpusfile = shift;

  # flatten out the Kleene stars
  open(TEAMTALKGRA, $grabsfile) || &fail("get_corpus(): Can't open $grabsfile file");
  open(TEAMTALKFLAT, ">$FLATGRAMMARFILE") || &fail("Can't open grammar flat file");
  print TEAMTALKFLAT &flatten_gra(<TEAMTALKGRA>);  #
  close TEAMTALKGRA;
  close TEAMTALKFLAT;

  # generate corpus
  &say('compile', "$RANDOMSAMPS -n $SAMPSIZE -d $GRAMMARDIR");
  open(RANDOM,
       "perl $RANDOMSAMPS -n $SAMPSIZE -d $GRAMMARDIR -grammarfile $FLATGRAMMARFILE |")
    || &fail("Cannot execute $RANDOMSAMPS");
  open(CORPUS, ">$corpusfile") || &fail("Can't open $corpusfile");

  # normalize sentences for output
  binmode CORPUS;
  while (<RANDOM>) {
    my @line = ();
    my @words = ();
    chomp;
    #	$_ = uc($_);
    s/<\/?[sS]> //g; # remove any sentence delimiters
    @words = split /\s+/,$_;
    push @line,"<s>";
    foreach my $w (@words) { # do not uppercase protected tokens (%[Foo]%)
      if ( $w =~ /%(\[.+?\])%/ ) { push @line,$1; } else { push @line,uc($w); }
    }
    push @line,"</s>";
    print CORPUS join( " ", @line),"\n";
  }
  close CORPUS;
  close RANDOM;
}

# flatten out all instances of concepts with Kleene stars
# this is presumably because the random sentence generator can't deal with it
# if there's a probability (eg, #%%0.5%%), it's split evenly across lines
sub flatten_gra {
  my @unflat = @_;
  my @result;
  my ($entry, $com, $prob,$newprob);

  for (@unflat) {
    # check for comments on line: pass through as it
    chomp;
    if ( /^[\[#;]/ or /^\s+$/ ) { push @result,"$_\n"; next; } # ignore non-concepts
    if ( /#/ ) { # is there a comment?
      ($entry,$com) = ( $_ =~ /(.+?)(#.*)/);
      # is the comment a prob?
      if ( $com =~ /%%(\d\.\d+)%%/ ) { $prob = $1; } else { $prob = undef; }
    } else { $entry = $_; $com = ""; $prob = undef; }
    if (! ($entry =~ s/^\s*\((.*)\)/$1/) ) {
      push @result, $entry.$com."\n";
    } else {
      # concept entry: do flattening if needed
      my @stack;
      my %flathash;
      push(@stack, [split /\s+/,$entry]);
      while (my $buffref = shift @stack) {
	my $i = 0;  # index of token within line
	my @buff = @$buffref;
	my $flat;
	for (@buff) {
	  if (/^\*(.*)/) {
	    $flat .= "$1 ";
	    push(@stack, [ @buff[0..$i-1], @buff[$i+1..$#buff] ]);
	  } else {
	    $flat .= "$_ ";
	  }
	  $i++;
	}
	$flathash{$flat} = 1;
      }
      if (defined $prob) {  # distribute prob uniformly over variants
	my $variants = scalar keys %flathash;
	$newprob = sprintf "#%%%%%7.5f%%%%", $prob/$variants;  # small p() danger
      } else { $newprob = ""; }
      foreach (keys %flathash) {
	push @result, "\t( $_) $newprob\n";
      }
    }
  }
  @result;
}




#
