#!/usr/bin/perl
# resolve class references in a .gra file
# produce an expanded version for Phoenix, abstracted version for generate_*
#
# path to a class file is notated as "%[File]%"  --> File.class
# a file of that name should exist, in the folder with the .gra file
# [20070923] (air) started

# [20090220] (air)
# fixed a rather major bug due to multiple classes per line (only first was processed)

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
open(FLAT,">$expfile") or die "resolve: can't open expgra: $expfile!\n";
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

print FLAT $preamble;
print ABS $preamble;

# pick out class nets, expand them
while (<IN>) {
    s/[\r\n]+$//;
    $line = $_;
    print ABS "$line\n";   # pass through
    $flat = "";
    while ( $line =~ /(.+?)%\[(.+?)\]%(.*?)$/ ) {   # resolve %[class]% markers in the line
	$pre=$1; $classname=$2; $line=$3;
	$flat .= "$pre\[$classname\]";
    
	if ( not defined $classnet{$classname} ) {
	    print STDERR "resolve: defining '$classname'\n";
	    open(CLASS,File::Spec->catfile($inpath,"$classname.class"))
		or die "resolve: missing class file: $classname.class\n";
	    my $classset = "\n[$classname]\n";
	    while (<CLASS>) {
		s/[\r\n]+$//;
		if ( /#/ ) {
		    ($text,$com) = split /\s*\#\s*/,$_,2; $div="#";
		} else {
		    $text = $_; $com = ""; $div = "";
		}
		$text =~ s/^\s*(.+?)\s*$/$1/;
		$classset .= "\t$text\t$div$com\n";
	    }
	    $classset .= ";\n";
	    $classnet{$classname} = $classset;
	    close(CLASS);
	}
    }
    if ( $flat eq "" ) { print FLAT "$line\n"; } else { print FLAT "$flat$line\n"; }
}
close(IN);
close(ABS);
    
# add class nets at the end of the file
print FLAT $postscript;
foreach $net (sort keys %classnet) { print FLAT $classnet{$net}; }
close(FLAT);
    
#
