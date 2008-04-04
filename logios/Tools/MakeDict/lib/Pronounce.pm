#
# Copyright (C) 2007-2008 Carnegie Mellon University
# for a given input word list, generate corresponding pronunciations
# Originally developed by Thomas Harris

# [20080324] (air) reorganized as a perl module; integrated into Logios platform
#                  supports both local and web-based resolution (due to licensed LtoS sw)
#                  new interface to pronounce
#

package Pronounce;
use strict;
use Config;
use File::Spec;


# problematic "globals"
my $SOURCE = 'loc';   # default
my ($PRONOUNCE);
my $pron_bin;

my ($LEXDATA, $LEXILIB, $LEXICON, $OUTDIR);
my ($wordfile, $handdict, $outfile, $logfile);


# [20071011] (air) factored out from makelm.pl
# DEPENDS ON THE MakeDict FOLDER STRUCTURE
BEGIN {
  if ( ($^O =~ /win32/i) or ($^O =~ /cygwin/) ) {$pron_bin = "x86-nt/pronounce.exe";}
  else { $pron_bin = "x86-linux/pronounce"; }  # assume we're going to be on linux
}


#########  create pronouncing dictionary from vocab list  ##########
sub make_dict {
  my ($tools,$resources,$vocfn, $handicfn, $outfn, $logfn) = @_;

  # require logging module
  require File::Spec->catfile($tools,'lib','LogiosLog.pm');

  # set required tools and project paths
  $PRONOUNCE = File::Spec->catfile($tools,'MakeDict','bin',$pron_bin);
  $LEXILIB = File::Spec->catdir($tools,'MakeDict','lib');
  $LEXICON = "cmudict.0.7a_SPHINX_40";
  $OUTDIR = $resources;

  $wordfile = File::Spec->catfile($OUTDIR,$vocfn);
  $handdict = File::Spec->catfile($OUTDIR,$handicfn);
  $outfile = File::Spec->catfile($OUTDIR,$outfn);
  $logfile = File::Spec->catfile($OUTDIR,$logfn);

  &LogiosLog::info("Pronounce::make_dict(): generating pronunciation dict");
# print STDERR ">>> $handdict, $wordfile, $outfile, $logfile <<<";
  &do_pronounce( $handdict, $wordfile, $outfile, $logfile );

  return;
}


####  pronounce, either by local code or by web lookup  ####
sub do_pronounce {
  my ($handdf, $wordf, $outf, $logf) = @_;

  if ($SOURCE eq 'web') { return &get_dic_web($handdf,$wordf,$outf,$logf); }
  if ($SOURCE eq 'loc') { return &get_dic_loc($handdf,$wordf,$outf,$logf); }
  &LogiosLog::fail("Pronounce::getdict(): unknown pronunciation source ".$SOURCE);
}



# pronunciation to be done locally; invoke pronunciation
sub get_dic_loc {
  my ($handdf, $wordf, $outf, $logf) = @_;

  my @pronounce_args = ('-P40',  # phone set; it's 40 by default, but be careful
			'-r', $LEXILIB, # root of ./lib; will prepend dict/ and lexdata/
			'-d', $LEXICON, # name of the dictionary to use
			'-i', $wordf,  # input words file
			'-o', $outf, # output dictionary file
			'-e', $logf,  # logging data
			'-v',  # verbose output
		       );
  push (@pronounce_args, '-H', $handdf) if -e $handdf;

 print STDERR "\n$PRONOUNCE ", join " ", @pronounce_args,"\n";
  return system("$PRONOUNCE",@pronounce_args);
}


# get the pronunciation from the website
sub get_dic_web {
  my $corpus = shift;

  my $ua = new LWP::UserAgent;
  my $res = $ua->request(POST => 'http://www.speech.cs.cmu.edu/cgi-bin/tools/lmtool.2.pl',
			 Content_Type => 'form-data', 
			 Content => [formtype => 'simple', 
				     corpus => [$corpus], 
				     #handdict => undef,
				     #extrawords => undef,
				     #phoneset => '40',
				     #bracket => 'Yes',
				     #model => 'Bigram',
				     #class => 'nil',
				     #discount => '0.5',
				     submit => 'COMPILE KNOWLEDGE BASE']);

  my $result;
  if ($res->is_success) {
    $result = $res->content;
  } else {
    &LogiosLog::fail("Pronounce::get_dict_web(): couldn't execute the perl script, probably error in the form"); }

# grep !(/CONTENT-TYPE/ || /TEXT\/PLAIN/), &getdic($pron_tmp);

  if ($result =~ /\!-- DIC.*ct\/\/(.*)\">/) {

    my $blah = "http://fife.speech.cs.cmu.edu/tools/product//$1";
    $res = $ua->request(GET $blah);

    if ($res->is_success) {
      return split(/\n/, $res->content);
    } else { &LogiosLog::fail("Pronounce::get_dict_web():Can't find dictionary file"); }
  } else {   &LogiosLog::fail("Pronounce::get_dict_web(): Couldn't parse the result: $result"); }
}


#####
1;  #
#####
