#!/bin/sh
# Try to find an executable that can send mail
export PATH=/usr/local/bin:/bin/:/usr/bin/:~robust/archive/third_party_packages/NIST_scoring_tools/sctk/linux/bin:$PATH

# Try, in this order, mhmail. mailx, mail, and mailto. If all fail,
# safe guard by using sendmail.

MAILX=sendmail

# Try to find mhmail
TMPMAIL=`which mhmail 2> /dev/null`
if test z${TMPMAIL} == z; then
# If we failed, try mailx
    TMPMAIL=`which mailx 2> /dev/null`
    if test z${TMPMAIL} == z; then
# If we failed again, try mail
	TMPMAIL=`which mail 2> /dev/null`
	if test z${TMPMAIL} == z; then
# If we failed again, try mailto
	    TMPMAIL=`which mailto 2> /dev/null`
	fi
    fi
fi

# If we found one of the above, use it. Otherwise, keep sendmail
if test z${TMPMAIL} != z; then MAILX=${TMPMAIL}; fi

if [ -n "$PBS_ENVIRONMENT" ]; then
echo "This job was submitted by user $PBS_O_LOGNAME"
echo "This job was submitted to host $PBS_O_HOST"
echo "This job was submitted to queue $PBS_O_QUEUE"
echo "PBS working directory $PBS_O_WORKDIR"
echo "PBS job id $PBS_JOBID"
echo "PBS job name $PBS_JOBNAME"
echo "PBS environment $PBS_ENVIRONMENT"
echo "This script is running on `hostname`"
fi

S3REGTESTLIST='archan@cs.cmu.edu egouvea@cs.cmu.edu yitao@cs.cmu.edu dhuggins@cs.cmu.edu sclee@scanscout.com ricky.houghton@gmail.com'

#Run test, assume we're at the top level (sphinx3 directory). 

testdate=`date -I`

if ! make perf-quick > perf-quick.log 2>&1 ;
 then
    ${MAILX} -s "Quick Performance Test failed on date:$testdate, machine:`hostname`, dir:`pwd`" ${S3REGTESTLIST} < perf-quick.log
    exit
 fi

if ! make perf-std > perf-std.log 2>&1 ;
 then
    ${MAILX} -s "Standard Performance Test failed on date:$testdate, machine:`hostname`, dir:`pwd`" ${S3REGTESTLIST} < perf-std.log 
    exit
 fi

# Send all results to standard out. If the job is running via the
# queue, it will be stored in a file, and it's up to the caller to
# save it.

cat ./perf-std.log ./perf-quick.log ./src/tests/performance/*/*.sys ./src/tests/performance/*/*.raw

# Send analysis results.
cat ./src/tests/performance/*/*.sys | ${MAILX} -s "Results of S3 Standard Regression Test on date:$testdate, machine:`hostname`, dir:`pwd` " ${S3REGTESTLIST} 
