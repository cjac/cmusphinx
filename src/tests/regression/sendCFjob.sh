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

# The repository address
address=${username}@cvs.sf.net

# Mail header common strings
headstring="${package_name} Compile farm Compilation "

# cf server
cfserver=cf-shell.sf.net

# The local compilation space
root=./sphinxDailyCF.$$

# The local log
outfile=test.out

# Define a path, just in case
export PATH="/usr/local/bin:/bin:/usr/bin"

# remote logdir
logdir="logCompile"

#User path
#ugly hacks to get the Sourceforge compile farm directory structure
#e.g. for username arthchan2003 , the path will be
# /home/users/a/ar/arthchan2003/ . 

firstlevel=`echo $username |sed "s/./& /g" |cut -d" " -f1`
secondlevel=`echo $username |sed "s/../& /g" |cut -d" " -f1`
userpath=/home/users/${firstlevel}/${secondlevel}/${username}

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

S3DISTLIST='archan'


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

if ! scp ${sphinx_ver}.${package_ext} ${username}@${cfserver}:${userpath} >> $outfile 2>&1 ;
 then 
    ${MAILX} -s "${headstring} failed, Cause: scp failed." ${S3DISTLIST} < $outfile 
    exit
 fi

if ! ssh ${username}@${cfserver} tar zxvf ${sphinx_ver}.${package_ext}  >> $outfile 2>&1
 then 
    ${MAILX} -s "${headstring} failed, Cause: fail to remotely control tarring." ${S3DISTLIST} < $outfile 
    exit
 fi

if ! ssh ${username}@${cfserver} mv ${sphinx_ver} ${package_name}/ >> $outfile 2>&1
 then
    ${MAILX} -s "${headstring} failed, Cause: fail to remotely control moving" ${S3DISTLIST} < $outfile 
    exit
 fi

if ! ssh ${username}@${cfserver} ./compileFarmRunTest.sh  >> $outfile 2>&1
 then
    ${MAILX} -s "${headstring} failed, Cause: fail to start compileFarmRunTest.sh" ${S3DISTLIST} < $outfile 
    exit
 fi


tmp=`date --iso-8601=minutes`

if ! scp -r ${username}@${cfserver}:${userpath}/${logdir} ./log${tmp}  >> $outfile 2>&1
 then
    ${MAILX} -s "${headstring} failed, Cause: Cannot copy ${logdir}" ${S3DISTLIST} < $outfile 
    exit
 fi

if ! ssh ${username}@${cfserver} rm -r ${sphinx_ver}.${package_ext} ${package_name}/ logCompile >> $outfile 2>&1
 then
    ${MAILX} -s "${headstring} failed, Cause: Cannot clean up" ${S3DISTLIST} < $outfile 
    exit
 fi

pushd ./log${tmp}

if grep FAIL ./*/*.log >> $outfile 2>&1 
 then
    $(MAILX} -s "${headstring} failed, Please take a look at ./log${tmp}"
    exit
 else
    ${MAILX} -s "${headstring} succeeded." < $outfile
 fi

popd

popd

popd

chmod -R 777 $root
/bin/rm -rf $root









