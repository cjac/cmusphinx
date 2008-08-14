#!/usr/bin/perl
# resolve class references in a .gra file
# produce an expanded version for Phoenix, abstracted version for generate_*
#
# path to a class file is notated as "%[File]%"  --> File.class
# a file of that name should exist, in the folder with the .gra file
# [20070923] (air)

use Getopt::Long;
use File::Spec;

my ($inpath,$infile,$expfile);
my $usage = "usage: resolve --inpath <path> --infile <.gra> --expgra <_exp.gra> --abstgra <_abs.gra> \n";
if ( scalar @ARGV eq 0 or
     not GetOptions ( "inpath=s", \$inpath,
		      "infile:s" => \$infile,
		      "expgra:s" => \$expfile,
		      "absgra:s" => \$absfile,
		    ) ) { die $usage; }
print STDERR "resolve.pl  [in ",File::Spec->rel2abs(File::Spec->curdir),"]\n";
print STDERR "\t> infile-> $infile\n\t> graex-> $expfile\n\t> grabs-> $absfile\n";
open(IN,"$infile") or die "resolve: can't open infile: $infile!\n";
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
    print STDERR "resolve: defining '$file'\n";
    open(CLASS,File::Spec->catfile($inpath,"$file.class")) or die "resolve: missing class file: $file.class\n";
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
