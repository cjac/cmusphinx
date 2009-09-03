#!/bin/sh

if [ $# != 1 ]; then
	echo "Usage: $0 MEETING"
	exit 1
fi
meeting=$1

./trans_select/selectICSI_trans.pl -removemixture y \
	-removenoise y -removeoverlap n \
	-incontrolfile transcripts.corr/$meeting.mrt.am.ctl \
	-intranswosil transcripts.corr/$meeting.mrt.am.wosil.trans \
	-outcontrolfile $meeting/$meeting.mrt.ctl.out \
	-outtranswosil $meeting/$meeting.mrt.am.wosil.trans.out \
	-outtranswsil $meeting/$meeting.mrt.am.wsil.trans.out \
	-outtransremoved $meeting/$meeting.mrt.am.wosil.trans.removed

./icsi-partition.pl $meeting/$meeting.mrt.ctl.out \
	$meeting/$meeting.mrt.am.wsil.trans.out $meeting
