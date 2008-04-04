#!E:/Perl/bin/perl -w
# resolve class references in a .gra file
# produce an expanded version for Phoenix, abstracted version for generate_*
#
# path to a class file is notated as "%[File]%"  --> File.class
# a file of that name should exist, in the folder with the .gra file
# [20070923] (air)

use Getopt::Long;
use File::Spec;

my ($infile,$expfile);
my $usage = "usage: resolve -infile <.gra> -expgra <_exp.gra> -abstgra <abs_.gra> \n";
if ( scalar @ARGV eq 0 or
     not GetOptions ( "infile:s" => \$infile,
		      "expgra:s" => \$expfile,
		      "absgra:s" => \$absfile,
		    ) ) { die $usage; }
print STDERR "resolve: \n > infile-> $infile  graex-> $expfile  grabs-> $absfile\n";
print STDERR " > executing in: ",File::Spec->rel2abs(File::Spec->curdir),"\n";
open(IN,"GRAMMAR/$infile") or die "resolve: can't open infile: $infile!\n";
open(OUT,">$expfile") or die "resolve: can't open expgra: $expfile!\n";
open(ABS,">$absfile") or die "resolve: can't open absgra: $absfile!\n";

my $postscript = <<EOS;


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ~                       EXPANDED CLASS NETS                     ~
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

EOS

my $preamble = <<EOS;

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# ~  AUTOMATICALLY GENERATED INTERMEDIATE GRAMMAR; DO NOT EDIT!   ~
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

EOS

print OUT $preamble;
print ABS $preamble;

while (<IN>) {
  chomp;
  if ( /(.+?)%\[(.+?)\]%(.*?)$/) {
    $pre  = $1; $file=$2; $post=$3;
    print OUT "$pre\[$file\]$post\n";
    print ABS "$pre%\[$file\]%$post\n";  # pass the marker through
  } else { print OUT "$_\n"; print ABS "$_\n";  next; }
  if ( not defined $classnet{$file} ) {
    print STDERR "resolve: defining $file\n";
    open(CLASS,"GRAMMAR/$file.class") or die "resolve: missing .class file: $file\n";
    my $classset = "\n[$file]\n";
    while (<CLASS>) {
      chomp;
      if ( /#/ ) { ($text,$com) = split /\s*#\s*/,$_,2; $div="#"; }
      else { $text = $_; $com = ""; $div = "";}
      $text =~ s/^\s*(.+?)\s*$/$1/;
      $classset .= "\t$text\t$div$com\n";
    }
    $classset .= ";\n";
    $classnet{$file} = $classset;
    close(CLASS);
  }
}
close(IN);

# add class nets at the end of the file
print OUT $postscript;
foreach $net (sort keys %classnet) { print OUT $classnet{$net}; }
close(OUT);

close(ABS);

#
