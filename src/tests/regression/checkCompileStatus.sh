#!/bin/bash

username=$1

# The repository address
address=${username}@cvs.sf.net

# Define a path, just in case
export PATH="/usr/local/bin:/bin:/usr/bin:/usr/ccs/bin"

# Try to find an executable that can send mail
# Default to sendmail
MAILX=sendmail

# Hack. 'which' on Solaris (at least the SCS facilitized ones) sends
# output to standard output *no matter what*, instead of sending it to
# stderr when an error occurs. Therefore, we can't just redirect
# stderr to /dev/null when doing, say, 'which mhmail'. Therefore, we
# have to do this: try to find the executable using 'which'. Count the
# number of items in the output. If it's only one, 'which' returned the
# executable location. If it's more than one, it is an error
# message. The bash test returns false if it has zero arguments, and
# true if it has one true argument. Therefore, when we test the number
# of items using awk, we output 1 (true) if the incoming string has
# more than 1 item, and nothing otherwise.

# Try to find mhmail
TMPMAIL=`which mhmail`
if test `echo ${TMPMAIL} | awk '{if (NF > 1) print 1}'`; then
# If we failed, try mailx
    TMPMAIL=`which mailx`
    if test `echo ${TMPMAIL} | awk '{if (NF > 1) print 1}'`; then
# If we failed again, try mail
	TMPMAIL=`which mail`
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

# Try to find gmake, supposedly the GNU make. autogen.sh is too smart
# for our own good though: it defines MAKE internally in the Makefile,
# so beware of how you name this variable

GMAKE=`which gmake`
if test `echo ${GMAKE} | awk '{if (NF > 1) print 1}'`; then
# If we failed, try make
    GMAKE=`which make`
    if test `echo ${GMAKE} | awk '{if (NF > 1) print 1}'`; then
# If we failed again, bail out: we cannot make the project!
    ${MAILX} -s "Make not found in system `hostname`" ${S3LIST} < /dev/null
# Exit with non zero value
    exit 1;
    fi
fi

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
if ! ${GMAKE} all test >> $outfile 2>&1 ;
 then ${MAILX} -s "sphinx2 compilation failed" ${S2LIST} < $outfile
 elif ! (grep BESTPATH $outfile | grep 'GO FORWARD TEN METERS' > /dev/null);
 then ${MAILX} -s "Sphinx2 test failed" ${S2LIST} < $outfile;
 else ${MAILX} -s "sphinx2 compilation and test succeeded" ${S2LIST} < $outfile
fi

popd >> $outfile 2>&1

/bin/rm -rf sphinx2

# Fresh download of sphinx3
cvs -d:ext:${address}:/cvsroot/cmusphinx co sphinx3 > $outfile 2>&1

# Configure it
pushd sphinx3 >> $outfile 2>&1

./autogen.sh >> $outfile 2>&1 
./autogen.sh >> $outfile 2>&1 

# Compile and run test, and verify that all tests ran successfully
if ! ${GMAKE} all test-full >> $outfile 2>&1 ;
 then ${MAILX} -s "sphinx3 compilation failed" ${S3LIST} < $outfile;
 elif ! (grep FWDVIT $outfile | grep 'P I T T S B U R G H' > /dev/null);
 then ${MAILX} -s "Sphinx3 test failed" ${S3LIST} < $outfile;
 elif ! (grep BSTPTH $outfile | grep 'P I T T S B U R G H' > /dev/null);
 then ${MAILX} -s "Sphinx3 test failed" ${S3LIST} < $outfile;
 elif grep FAILED $outfile > /dev/null;
 then ${MAILX} -s "Sphinx3 test failed" ${S3LIST} < $outfile;
 else ${MAILX} -s "sphinx3 compilation and test succeeded" ${S3LIST} < $outfile;
fi

popd >> $outfile 2>&1

/bin/rm -rf sphinx3

# Fresh download of SphinxTrain
cvs -d:ext:${address}:/cvsroot/cmusphinx co SphinxTrain > $outfile 2>&1

# Configure it
pushd SphinxTrain >> $outfile 2>&1
./configure >> $outfile 2>&1

# Compile and make sure it's successful
if ! ${GMAKE} all >> $outfile 2>&1 ;
 then ${MAILX} -s "SphinxTrain compilation failed" ${STLIST} < $outfile;
 else ${MAILX} -s "SphinxTrain compilation succeeded" ${STLIST} < $outfile;
fi

popd >> $outfile 2>&1

# Remove what we created
cd /tmp
/bin/rm -rf $root



