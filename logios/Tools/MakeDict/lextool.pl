#
# Copyright (C) 2007-2008 Carnegie Mellon University
# CGI script for the LexTool web service
# Developed by Alex Rudnicky and Thomas Harris

# [20080326] (air) Release version.
#

# DEPENDS ON THE MakeDict FOLDER STRUCTURE
BEGIN {  push @INC, "./lib", "MakeDict/lib"; }

use strict;
use HTML::Template;
use File::Spec;
use Pronounce;

use CGI::Simple;
$CGI::Simple::DISABLE_UPLOADS = 0;
my $cgi = CGI::Simple->new;

sub handler { my $sig = @_;
	      print STDERR "lmtool caught a SIG$sig -- dying\n";
	      exit(0);
	    }
# foreach (qw(XCPU KILL TERM STOP)) { $SIG{$_} = \&handler; }
foreach (qw(KILL TERM STOP)) { $SIG{$_} = \&handler; }



&LogiosLog::logios_log(-1);  # don't open a logging file
$| = 0;  # autoflush
  print "Content-Type: text/html\012\012";

# NOTE: these paths are arranged as per READ_ME file
my $tools = File::Spec->rel2abs(File::Spec->curdir());

my $where = File::Spec->rel2abs # where stuff will end up
  (File::Spec->catdir('..','..',"htdocs","tools","product"));
my $outdir = time . "_" . sprintf "%05d", $$;  # time and pid
srand (time () ^ ($$ + ($$ << 15)));  # reseed the rand'er
my $base = sprintf "%04d", int(rand 10000) +1;  # get a rand
my $workdir = File::Spec->catdir($where,$outdir);
mkdir($workdir,0766);  # make a unique dirname for the current session

my $stattempl = HTML::Template->new(filename => File::Spec->catfile($tools,'template','status'));
my $rslttempl = HTML::Template->new(filename => File::Spec->catfile($tools,'template','result'));
my $errotempl = HTML::Template->new(filename => File::Spec->catfile($tools,'template','error'));

my $word = $cgi->param('singleword');
if ( defined $word ) {
  # do a single-word lookup
  # ( but not yet... )
}

# else, it's an upload invocation
if ( not defined $cgi->param('wordfile') or $cgi->param('wordfile') eq "") {
  $errotempl->param(reason => "you did not specify an input file");
  print $errotempl->output;
  exit(0);
}
my $okw = $cgi->upload( $cgi->param('wordfile'),File::Spec->catfile($workdir,"$base.word"));
if (not defined $okw or defined $cgi->cgi_error()) {
  $errotempl->param(reason => "could not upload word file".$cgi->cgi_error());
  print $errotempl->output;
  exit;
}
# this is an optional upload...
my $okh = $cgi->upload( $cgi->param('handfile'),File::Spec->catfile($workdir,"$base.hand"));

# generate the dictionary
&Pronounce::make_dict($tools,$workdir,"$base.word","$base.hand","$base.dict","$base.pron.log");

# compose the results page ****
$rslttempl->param(server => "http://ramps.speech.cs.cmu.edu",
		  path => "tools/product/$outdir",
		  file => "$base.dict"
		 );

print $rslttempl->output;


sub lextool_death {
  print "Content-type: text/html\012\012", $errotempl->output;
  die "LEXTOOL died due to malformed input\n";
}


#
