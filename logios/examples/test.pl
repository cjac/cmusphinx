#!/usr/bin/perl

use File::Spec;
use File::Path;
use Getopt::Long;

use strict;

my $PERL = 'perl';
my $LOGIOS_ROOT = File::Spec->rel2abs(File::Spec->updir);
my $RESOURCES = File::Spec->catdir($LOGIOS_ROOT, 'examples', 'Resources');
my $DICTIONARYDIR = File::Spec->catdir($RESOURCES, 'DecoderConfig', 'Dictionary');
my $LMDIR = File::Spec->catdir($RESOURCES, 'DecoderConfig', 'LanguageModel');
my $GRAMMARDIR = File::Spec->catdir($RESOURCES, 'Grammar');
my $PROJECT = 'MeetingLineDomain';
my $INSTANCE = 'MeetingLine';

my $MAKELANGUAGE = File::Spec->catfile($LOGIOS_ROOT, 'Tools', 'MakeLanguage', 'make_language.pl');

my $target = shift;
&clean; #clean by default
exit if $target eq 'clean';
&build;
print "Press ENTER to exit.$/"; <STDIN>;
exit;

sub clean {
    print STDERR "Cleaning up Resources$/";
    my @deltargets = (glob(File::Spec->catfile($DICTIONARYDIR, '*')),
		      glob(File::Spec->catfile($LMDIR, '*')),
		      map(File::Spec->catfile($GRAMMARDIR, $_),
			  'forms', 'frames', 'log', 'logios.log', 'nets',
			  map("$INSTANCE$_", 
			      '.corpus', '.ctl', '.gra', '.probdef', '.token', '.words', 
			      '_abs.gra', '_flat.gra')));
    rmtree(\@deltargets, 1, 1);
}

sub build {
    print stderr "Executing MakeLanguage$/";
    system "$PERL $MAKELANGUAGE"
	." --logios $LOGIOS_ROOT"
	." --resources $RESOURCES"
	." --project $PROJECT"
	." --instance $INSTANCE"
	." --logfile $INSTANCE.log";
}
