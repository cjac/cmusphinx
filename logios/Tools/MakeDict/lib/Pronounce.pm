#
# Copyright (C) 2007-2008 Carnegie Mellon University
# for a given input word list, generate corresponding pronunciations
# Originally developed by Thomas Harris

# [20071011] (air) factored out from makelm.pl
# [20080324] (air) reorganized as a perl module; integrated into logios platform
#                  supports both local and web-based resolution (due to licensed LtoS sw)
#                  new interface to pronounce
#

package Pronounce;
use strict;
use Config;
use File::Spec;
use LWP::UserAgent;

sub new {
  my $class = shift;
  my %params = @_;

  my $objref = {'SOURCE' => $params{'SOUCRCE'} || 'loc',
                'PRONOUNCE' => ($params{'SOURCE'} eq 'web'? undef:
                                File::Spec->catfile($params{'TOOLS'}, 'MakeDict', 'bin',
                                                    (($^O =~ /win32/i or $^O =~ /cygwin/)?
                                                     'x86-nt/pronounce.exe':
                                                     'x86-linux/pronounce'))),
                'LEXILIB' => File::Spec->catdir($params{'TOOLS'}, 'MakeDict', 'lib'),
                'LEXICON' => 'CMUdict_SPHINX_40',
                'OUTDIR' => $params{'DICTDIR'},
                'WORDFILE' => File::Spec->catfile($params{'DICTDIR'}, $params{'VOCFN'}),
                'HANDDICT' => ($params{'HANDICFN'}?
                               File::Spec->catfile($params{'DICTDIR'}, $params{'HANDICFN'}):
                               undef),
                'OUTFILE' => File::Spec->catfile($params{'DICTDIR'}, $params{'OUTFN'}),
                'LOGFILE' => File::Spec->catfile($params{'DICTDIR'}, $params{'LOGFN'})
               };

  die "Need to knoe the LOGIOS Tools root." if !defined $params{'TOOLS'};
  require File::Spec->catfile($params{'TOOLS'}, 'lib', 'LogiosLog.pm');

  for ('DICTDIR', 'VOCFN', 'OUTFN', 'LOGFN') {
    &LogiosLog::fail("Must supply $_") if !$params{$_};
  }

  for (keys %$objref) {
    &LogiosLog::say('Pronounce', "\t$_ => $objref->{$_}");
  }

  bless $objref, $class;
}

####  pronounce, either by local code or by web lookup  ####
sub do_pronounce {
  my $self = shift;

  $self->clean_wordfile;
  return $self->get_dic_web if $self->{'SOURCE'} eq 'web';
  return $self->get_dic_loc if $self->{'SOURCE'} eq 'loc';
  &LogiosLog::fail("Pronounce::getdict(): unknown pronunciation source ".$self->{'SOURCE'});
}

# clean-up the tokens
# tokens that come from compile_gra.pl include context tokens <s> and </s>
# sphinx chokes if those tokens are in the dictionary, and pronounce fails to ignore them
# so we strip them out here
sub clean_wordfile {
  my $self = shift;

  open(WORDFILE, $self->{'WORDFILE'});
  my @clean_words = grep {!/^</} <WORDFILE>;
  close WORDFILE;
  open(WORDFILE, ">$self->{'WORDFILE'}");
  print WORDFILE @clean_words;
  close WORDFILE;
}

# pronunciation to be done locally; invoke pronunciation
sub get_dic_loc {
  my $self = shift;

  my @pronounce_args = ('-P40',  # phone set; it's 40 by default, but be careful
                        '-r', $self->{'LEXILIB'}, # root of ./lib; will prepend dict/ and lexdata/
                        '-d', $self->{'LEXICON'}, # name of the dictionary to use
                        '-i', $self->{'WORDFILE'},  # input words file
                        '-o', $self->{'OUTFILE'}, # output dictionary file
                        '-e', $self->{'LOGFILE'},  # logging data
                        '-v',  # verbose output
                       );
  push(@pronounce_args, '-H', $self->{'HANDDICT'}) if -e $self->{'HANDDICT'};

  &LogiosLog::say('Pronounce', join(" ", @pronounce_args));
  return system($self->{'PRONOUNCE'}, @pronounce_args);
}


# get the pronunciation from the website
sub get_dic_web {
  my $self = shift;

  my $corpus = shift;
  &LogiosLog::fail("Need corpus") if !$corpus;

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
