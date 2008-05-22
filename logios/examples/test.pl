#!/usr/bin/perl

use File::Spec;

use strict;

my $PERL = 'perl';
my $LOGIOS_ROOT = File::Spec->rel2abs(File::Spec->updir);
my $RESOURCES = File::Spec->catdir($LOGIOS_ROOT, 'examples', 'Resources');
my $PROJECT = 'MeetingLineDomain';
my $INSTANCE = 'MeetingLine';

my $MAKELANGUAGE = File::Spec->catfile($LOGIOS_ROOT, 'Tools', 'MakeLanguage', 'make_language.pl');

print stderr "Executing MakeLanguage$/";
exec "$PERL $MAKELANGUAGE"
    ." --logios $LOGIOS_ROOT"
    ." --resources $RESOURCES"
    ." --project $PROJECT"
    ." --instance $INSTANCE"
    ." --logfile $INSTANCE.log";

