#!/bin/sh -x

time ../src/phoenix2corpus 100000 forms.txt MeetingLine.gra > /tmp/corptest
head /tmp/corptest
rm -f /tmp/corptest

