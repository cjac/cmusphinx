#!/bin/bash

username=$1

# The repository address
address=${username}@cvs.sf.net

# Define a path, just in case
export PATH="/usr/local/bin:/bin:/usr/bin"

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


# Define the variables for compilation root, mailing lists
root=/tmp/sphinxCompilation.$$
S2LIST='archan egouvea yitao dhuggins'
S3LIST='archan egouvea yitao dhuggins'
S4LIST='cmusphinx-commits@lists.sourceforge.net'
STLIST='archan egouvea yitao dhuggins'

# Create root and move there
mkdir $root
cd $root

# Output file that will get sent in case of failure
outfile=$root/test.out

# CVS requires us to define this
export CVS_RSH=ssh

# Fresh download of sphinx2
cvs -d:ext:${address}:/cvsroot/cmusphinx co sphinx2 > $outfile 2>&1

# Configure it
pushd sphinx2 >> $outfile 2>&1

./autogen.sh >> $outfile 2>&1 
./autogen.sh >> $outfile 2>&1 

# Compile and run test, and verify if both were successful
if ! make all test >> $outfile 2>&1 ;
 then ${MAILX} -s "sphinx2 compilation failed" ${S2LIST} < $outfile
 elif ! (grep BESTPATH $outfile | grep -q 'GO FORWARD TEN METERS');
 then ${MAILX} -s "Sphinx2 test failed" ${S2LIST} < $outfile;
 else ${MAILX} -s "sphinx2 compilation or test succeeded" ${S2LIST} < $outfile
fi

popd >> $outfile 2>&1

# Fresh download of sphinx3
cvs -d:ext:${address}:/cvsroot/cmusphinx co sphinx3 > $outfile 2>&1

# Configure it
pushd sphinx3 >> $outfile 2>&1

./autogen.sh >> $outfile 2>&1 
./autogen.sh >> $outfile 2>&1 

# Compile and run test, and verify that all tests ran successfully
if ! make all test-full >> $outfile 2>&1 ;
 then ${MAILX} -s "sphinx3 compilation failed" ${S3LIST} < $outfile;
 elif ! (grep FWDVIT $outfile | grep -q 'P I T T S B U R G H');
 then ${MAILX} -s "Sphinx3 test failed" ${S3LIST} < $outfile;
 elif ! (grep BSTPTH $outfile | grep -q 'P I T T S B U R G H');
 then ${MAILX} -s "Sphinx3 test failed" ${S3LIST} < $outfile;
 elif grep -q FAILED $outfile;
 then ${MAILX} -s "Sphinx3 test failed" ${S3LIST} < $outfile;
 else ${MAILX} -s "sphinx3 test succeeded" ${S3LIST} < $outfile;
fi

popd >> $outfile 2>&1

# Fresh download of SphinxTrain
cvs -d:ext:${address}:/cvsroot/cmusphinx co SphinxTrain > $outfile 2>&1

# Configure it
pushd SphinxTrain >> $outfile 2>&1
./configure >> $outfile 2>&1

# Coompile and make sure it's successful
if ! make all >> $outfile 2>&1 ;
 then ${MAILX} -s "SphinxTrain compilation failed" ${STLIST} < $outfile;
 else ${MAILX} -s "SphinxTrain compilation succeeded" ${STLIST} < $outfile;
fi

popd >> $outfile 2>&1

# Remove what we created
cd /tmp
/bin/rm -rf $root



