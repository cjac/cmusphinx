#!/bin/bash -x

if test $[$# < 1]  = 1
then
    echo "$0 <Sourceforge username>"
    exit 1
fi

#Written by Arthur Chan 20041207
if test $1 = "archan" 
    then 
    echo "Arthur Chan ! Your user name in Sourceforge is not archan! Do you have any long-term memory!?"
    exit 1
fi

#This script is run in developer side to initiate the chain of compilation.
username=$1

# Arthur : To be replaced by global symbol
sphinx_ver="sphinx3-0.5"

#Package name 
package_name="sphinx3"

#Package extension
package_ext="tar.gz"

#Distribution full name
distfn="${sphinx_ver}.${package_ext}"

# cf shell server address
cfshserver=cf-shell.sf.net
cfsh_address=${username}@${cfshserver}

# Mail header common strings
headstring="${package_name} Compile farm Compilation "

# The local compilation space
root=./sphinxDailyCF.$$

# The local log
outfile=test.out

# The log analysis log
analysislog=analysis.out

# Define a path, just in case
export PATH="/usr/local/bin:/bin:/usr/bin"

# remote logdir
logdir="logCompile"


# Try to find an executable that can send mail
# Default to sendmail
MAILX=sendmail

# Try to find mhmail
TMPMAIL=`which mhmail 2> /dev/null`
if test z${TMPMAIL} == z; then
# If we failed, try mailx
    TMPMAIL=`which mailx 2> /dev/null`
    if test z${TMPMAIL} == z; then
# If we failed again, try mail
	TMPMAIL=`which mail 2> /dev/null`
    fi
fi

# If we found one of the above, use it. Otherwise, keep sendmail
if test z${TMPMAIL} != z; then MAILX=${TMPMAIL};fi

S3DISTLIST='archan@cs.cmu.edu egouvea@cs.cmu.edu dhuggins@cs.cmu.edu yitao@cs.cmu.edu scotts+@pitt.edu '


mkdir $root
pushd $root 

## cvs 
cvs -d:ext:${username}@cvs.sf.net:/cvsroot/cmusphinx co ${package_name} >> $outfile 2>&1

pushd ${package_name} 

sh ./autogen.sh >> $outfile 2>&1
sh ./autogen.sh >> $outfile 2>&1 

if ! make distcheck >> $outfile 2>&1 ;
 then 
    ${MAILX} -s "${headstring} failed, Cause: make distcheck failed." ${S3DISTLIST} < $outfile 
    exit
 fi

#clean up the previous setup
ssh ${cfsh_address} rm -r ${package_name} ${logdir} ${distfn} >> $outfile 2>&1

if ! scp ${distfn} ${cfsh_address}:.  >> $outfile 2>&1 ;
 then 
    ${MAILX} -s "${headstring} failed, Cause: scp failed." ${S3DISTLIST} < $outfile 
    exit
 fi


if ! ssh ${cfsh_address} tar zxvf ${distfn}  >> $outfile 2>&1
 then 
    ${MAILX} -s "${headstring} failed, Cause: fail to remotely control tarring." ${S3DISTLIST} < $outfile 
    exit
 fi

if ! ssh ${cfsh_address} mv ${sphinx_ver} ${package_name}/ >> $outfile 2>&1
 then
    ${MAILX} -s "${headstring} failed, Cause: fail to remotely control moving" ${S3DISTLIST} < $outfile 
    exit
 fi

if ! ssh ${cfsh_address} mkdir ${logdir} >> $outfile 2>&1
 then
    ${MAILX} -s "${headstring} failed, Cause: fail to remotely mkdir" ${S3DISTLIST} < $outfile 
    exit
 fi

if ! ssh ${cfsh_address} ./compileFarmRunTest.sh  >> $outfile 4>&1
 then
    ${MAILX} -s "${headstring} failed, Cause: fail to start compileFarmRunTest.sh" ${S3DISTLIST} < $outfile 
    exit
 fi

if ! ssh ${cfsh_address} tar zcvf log.tgz ${logdir} 2>&1 
then
    ${MAILX} -s "${headstring} failed, Cause: Remote taring of ${logdir} failed" ${S3DISTLIST} < $outfile 
    exit
fi

popd

if ! scp ${cfsh_address}:./log.tgz .  >> $outfile 2>&1
 then
    ${MAILX} -s "${headstring} failed, Cause: Cannot copy ${logdir}" ${S3DISTLIST} < $outfile 
    exit
 fi

if ! ssh ${cfsh_address} rm -r ${distfn} ${package_name}/ >> $outfile 2>&1
 then
    ${MAILX} -s "${headstring} failed, Cause: Cannot clean up" ${S3DISTLIST} < $outfile 
    exit
 fi

tmp=`date --iso-8601`
tar zxvf ./log.tgz 
cp -r ${logdir} ./log${tmp}

pushd ./log${tmp}
echo "Sphinx 3 CF compilation report" > $analysislog
for i in `find . -name  "*.log" -maxdepth 1`
do 
    echo "Platform $i" >> $analysislog
    grep PASS $i >> $analysislog
    grep FAIL $i >> $analysislog
    echo "`grep PASS $i |wc -l` PASSES"  >> $analysislog 
    echo "`grep FAIL $i |wc -l` FAILS" >> $analysislog
done
${MAILX} -s "${headstring} Compilation completed. analysis " ${S3DISTLIST} < $analysislog
${MAILX} -s "${headstring} Compilation completed " ${S3DISTLIST} < $outfile
popd

popd

chmod -R 777 $root/${package_name} 
/bin/rm -rf $root/${package_name}









