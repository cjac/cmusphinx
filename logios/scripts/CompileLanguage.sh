#!/usr/bin/bash
# Compile a language knowledge base from a grammar
# [20080707] (air)

#   ::  ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  ::::::
#   ::  FIRST: Copy this script into your Project root folder                             ::::::
here=$PWD

#   ::  SECOND: Change the following line to point to your Logios installation            ::::::
#   ::  or have the environment variable set appropriately                                ::::::
LOGIOS_ROOT=

# this next line accomodates using a native-installed perl under cygwin
# by turning the path into DOS style
if [ -n ${here#/cygdrive/} ]; then here=`cygpath -m -a $here`; fi
if [ -z "$LOGIOS_ROOT" ]; then LOGIOS_ROOT=$here; fi  # defaults to current folder

#   ::  THIRD: (for Olympus) create your project grammar in Resources\Grammar\GRAMMAR     ::::::
#   ::  (the following script creates the folder tree)                                    ::::::
#   ::  If this is not for Olympus, you'll need to specify --inpath and --outpath, below  ::::::

#   ::  CHANGE THESE LABELS AS PER YOUR PROJECT; OUTPUT FILES WILL BE NAMED ACCORDINGLY   ::::::

PROJECT="MeetingLineDomain"
INSTANCE="MeetingLine"
OLYMPUS=do_this

#   ::  ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  ::::::


#   :: Compile language knowledge bases from the grammar
#   :: --olympus and --resources flag must be used together, in which case the Resources/ tree is used

if [ -n "$OLYMPUS" ]; 
then
    # make the Olumpus-style tree, if it doesn't exist
    $LOGIOS_ROOT/scripts/makeTree.sh

    RESOURCES=$here/Resources
    perl $LOGIOS_ROOT/Tools/MakeLanguage/make_language.pl \
	--logios $LOGIOS_ROOT \
	--olympus \
	--resources $RESOURCES \
	--project $PROJECT --instance $INSTANCE \
	--logfile $INSTANCE.log
else
#   :: otherwise --inpath and --outpath give you control over actual locations
    INPATH=$here/Resources/Grammar/GRAMMAR
    OUTPATH=$here/Resources/Grammar/GRAMMAR
    perl $LOGIOS_ROOT/Tools/MakeLanguage/make_language.pl \
	--logios $LOGIOS_ROOT \
	--inpath $INPATH --outpath $OUTPATH \
	--project $PROJECT --instance $INSTANCE \
	--logfile $INSTANCE.log
fi

#

