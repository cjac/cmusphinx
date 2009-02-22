#!E:/Perl/bin/perl.exe -w
# 
# convert absolute task grammar and its .class files into:
#    a) .ctl file
#    b) .probdef file
#    c) .token file (for pronounce dict)
#    d) .words (for lm wordlist)

# [20070923] (air) Created.
# [20090208] (air) Added the 'pocket' option to select .ctl lm path format
# [20090209] (air) check for empty class file; die if so.
# [20090220] (air) clean up how class names are manipulated (now works correctly for '.'s in class names)

use Getopt::Long;
use File::Basename;
use File::Spec;

my ($inpath,$grafile,$project,$pocket_flag,$wordfile);
my $usage="usage: tokenize -grammar <file> -project <name> [-pocket]\n";
if (scalar @ARGV eq 0
    or not GetOptions (
	"inpath=s" => \$inpath,
	"grammar=s" => \$grafile,
	"project=s" => \$project,
	'pocket' => \$pocket_flag,
    ) ) { die $usage; }
$probdefile = "$project.probdef";
$tokenfile = "$project.token";
$wordfile = "$project.words";
print STDERR "tokenize.pl  [in ",File::Spec->rel2abs(File::Spec->curdir),"]\n";
print STDERR "\tgrammar-> $grafile\n\tproject->$project\n\twordfile->$wordfile\n";
my $classcount = 0;

my $epsilon = 0.0001;  # slop factor for probability distribution (10^-4)
my $fault = 0;

# scan .gra file; make a list of classes that need to be processed
# also collect all terminals to make a wordlist (used in lm compilation)
my %classes = (); my %wordlist = ();
open(GRA,"$grafile") or die "tokenize.pl: $grafile not found!\n$usage\n";
while (<GRA>) {
  chomp;
  if ( /^\s*#/ or /^\s+$/ ) { next; }  # skip comments, blank lines
  if ( /\s+\(\s*(.+?)\)\s*/) {  # look only at ()'s
    @toks = split /\s+/, $1;
  } else { next; }
  foreach $tok (@toks) {
    $tok =~ s/^\**(.+)/$1/;  # strip off Kleene star
    if ( $tok =~ /^[A-Z]+/ ) { next; }  # skip macros
    if ( $tok =~ /%(\[.+?\])%/) { # keep protected net names, with their []'s
      # insert new ones into class net list, also treat them as "words" for lm
      if ( not defined $classes{$1} ) {
        $classcount++;
        $classes{$1} = sprintf "C%02d",$classcount;
        print STDERR "tokenize: found $1 [class: $classes{$1}]\n";
      }
      $wordlist{$1} = "c";  # remember type
    } elsif ( $tok =~ /^\[.+?\]/ ) { next; }  # non-class net, ignore
    else {
      $w = $1;
      $wordlist{$w} = "w";
    }
  }
}
close(GRA);


# do each class
open(PROB,">$probdefile") or die "tokenize: can't open $probdefile";
foreach $classname (sort keys %classes) {
  $classid = $classes{$classname};
  $classname =~ s/\[(.+?)\]/$1/;  # strip []'s
  $classfil = "$classname.class";  # get the file name itself
  open(CLASS,File::Spec->catfile($inpath,$classfil)) or die "tokenize: class file $classfil not found";
  my %lexset = ();
  my $lexcount = 0;
  while (<CLASS>) {
    chomp;
    $line = $_;
    if ( /#/ ) { # has a comment, may include a probability
      ($text,$com) = split /\s*#\s*/,$line,2;
      if ( $com =~ /%%(\d\.\d+)%%/ ) { $prob = $1; }
      else { $prob = undef; }
    } else { # unspecified: "implicit"
      $text = $line; $prob = undef;
    }
    $text =~ s/^\s*\((.+?)\)\s*$/$1/;  # trim spaces from ends, strip ()'s
    # tokenize the text by substituting spaces with '='s (to distinguish from dict ligatures)
    $text =~ s/\s+/=/g;
    # class-tag delimiter is ":::" (in case users put : or :: into their words)
    $tokens{"${text}:::$classid"}++;
    $lexset{"${text}:::$classid"} = $prob;
    $lexcount++;
  }
  close(CLASS);
  # if there's a class file, it must have at least one entry. If not, die.
  # otherwise downstream processes will behave erratically.
  if ( $lexcount eq 0 ) { die "tokenize: $classname appears to be empty!\n"; }

  # evaluate probabilities
  $mass = 0.0; $empty = 0;
  foreach $lex (keys %lexset) {
    if ( defined $lexset{$lex}) { $mass += $lexset{$lex}; }
    else { $empty++; }
  }
  if ($mass<0.0 or $mass>1.0) {
    print STDERR "tokenize: $classname -> explicit probs add up to $mass!\n";
    $fault++;
  }
  # fix up the probabilities so that everything adds up right
  $adjust = 1.0; $dist = 0.0;
  if ($empty eq 0 and $mass gt 0.0 and $mass lt (1.0-$epsilon)) { # all probs explicit
    $adjust = 1.0 / $mass; # not enough mass: scale all probs upwards
    print STDERR "tokenize: $classname -> explicit probs scaled by $adjust\n";
  } elsif ($mass lt 1.0 and $empty gt 0) {
    $dist = (1.0 - $mass)/$empty; # some probs not specified: split remaining mass
    print STDERR "tokenize: $classname -> token implicit probabilities set to $dist\n";
  } elsif ( $mass gt 1.0) {  # something not right...
    $adjust = 1.0 / ($mass+($epsilon*$empty)); # too much mass: scale all probs down
    print STDERR "tokenize: $classname -> explicit probs scaled by $adjust\n";
    $dist = $epsilon; # but set all other tokens to min prob
    print STDERR "tokenize: $classname -> $empty token probs set to $epsilon\n";
  }

  # readjust the class member probabilities
  foreach $lex (keys %lexset) {
    if ( defined $lexset{$lex} ) { $lexset{$lex} *= $adjust; }
    else { $lexset{$lex} = $dist; }
  }

  # add to the .probdef file
  print PROB "LMCLASS [$classname]\n";
  foreach $lex (sort keys %lexset) {
    printf PROB "%s\t%8.6f\n", uc($lex),$lexset{$lex};
  }
  print PROB "END [$classname]\n\n";
}
close(PROB);

# create .words file (for lm compilation); includes class []'s  --> UPPERCASE
open(WRD,">$wordfile") or die "tokenize: can't open $wordfile!\n";
foreach $t  (sort keys %wordlist) {
  if ( $t =~ /\[.+?\]/ ) { print WRD "$t\n"; } else { print WRD "\U$t\n"; }
}
close(WRD);

# create the .token file (for pronunciation dict); excludes []'s -> UPPERCASE
open(TOK,">$tokenfile") or die "tokenize: can't write to $tokenfile\n";
foreach (keys %tokens) { $wordlist{$_}="t";}  # add in the wordlist
foreach $tok (sort keys %wordlist) {
  if ($tok =~ /\[.+?\]/ ) { next; }  # but ignore nets []'s
  print TOK "\U$tok\n";  # for compatibility with pronounce
}
close(TOK);

# create a .ctl file
open(CTL,">$project.ctl") or die "tokenize: can't write to .ctl file!\n";
if (pocket_flag) {  # pocketsphinx does not want the path specified, just the file names
    print CTL "{ $project.probdef }\n$project.arpa general {\n";
} else {  # sphinx3 expects a full path relative to the execution folder
    print CTL "{ LanguageModel\\$project.probdef }\nLanguageModel\\$project.arpa general {\n";
}
foreach $class (sort keys %classes) { print CTL "$class\n"; }
print CTL "}\n";
close(CTL);
