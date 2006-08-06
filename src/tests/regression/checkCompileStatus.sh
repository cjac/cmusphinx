#!/bin/bash

# Define a path, just in case
export PATH="/usr/local/bin:/bin:/usr/bin:/usr/ccs/bin"

# Try to find an executable that can send mail
# Default to sendmail
MAILX=sendmail

# Since we're running under bash, don't bother using "which", which
# doesn't work on Solaris anyway.  Use "command" instead which is a
# handy builtin.

# Try to find mhmail
TMPMAIL=`command -v mhmail`
if test z"$TMPMAIL" = z; then
# If we failed, try mailx
    TMPMAIL=`command -v mailx`
    if test z"$TMPMAIL" = z; then
# If we failed again, try mail
	TMPMAIL=`command -v mail`
    fi
fi

# If we found one of the above, use it. Otherwise, keep sendmail
if test z${TMPMAIL} != z; then MAILX=${TMPMAIL};fi

# Define the variables for compilation root, mailing lists
root=/tmp/sphinxCompilation.$$
SBLIST='archan egouvea yitao dhuggins'
PSLIST='archan egouvea yitao dhuggins'
S2LIST='archan egouvea yitao dhuggins'
S3LIST='archan egouvea yitao dhuggins'
S4LIST='cmusphinx-commits@lists.sourceforge.net'
STLIST='archan egouvea yitao dhuggins'

# Try to find gmake, supposedly the GNU make. autogen.sh is too smart
# for our own good though: it defines MAKE internally in the Makefile,
# so beware of how you name this variable

GMAKE=`command -v gmake`
if test z"$GMAKE" = z; then
# If we failed, try make
    GMAKE=`command -v make`
    if test z"$GMAKE" = z; then
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

# Fresh download of sphinxbase
svn co https://svn.sourceforge.net/svnroot/cmusphinx/trunk/sphinxbase > $outfile 2>&1

# Configure it
pushd sphinxbase >> $outfile 2>&1

./autogen.sh >> $outfile 2>&1 
./autogen.sh >> $outfile 2>&1 

# Compile and run test, and verify if both were successful
if ! ${GMAKE} distcheck >> $outfile 2>&1 ;
 then ${MAILX} -s "sphinxbase compilation failed" ${SBLIST} < $outfile
 else ${MAILX} -s "sphinxbase compilation and test succeeded" ${SBLIST} < $outfile
fi

# sphinxbase is needed for everything else, so build it in-place, and don't remove it
if ! ${GMAKE}  >> $outfile 2>&1 ;
 then ${MAILX} -s "sphinxbase compilation failed" ${SBLIST} < $outfile
fi
# Make everything writable so it can be deleted later
chmod -R +w .
popd >> $outfile 2>&1

# Fresh download of pocketsphinx
svn co https://svn.sourceforge.net/svnroot/cmusphinx/trunk/pocketsphinx > $outfile 2>&1

# Configure it
pushd pocketsphinx >> $outfile 2>&1

./autogen.sh >> $outfile 2>&1 
./autogen.sh >> $outfile 2>&1 

# Compile and run test, and verify if both were successful
# Note that distcheck builds in a subdirectory so we need to give it
# an (absolute) path to sphinxbase.
if ! ${GMAKE} distcheck DISTCHECK_CONFIGURE_FLAGS=--with-sphinxbase=`pwd`/../sphinxbase >> $outfile 2>&1 ;
 then ${MAILX} -s "pocketsphinx compilation failed" ${PSLIST} < $outfile
 elif ! (grep BESTPATH $outfile | grep 'GO FORWARD TEN METERS' > /dev/null);
 then ${MAILX} -s "pocketsphinx test failed" ${PSLIST} < $outfile;
 else ${MAILX} -s "pocketsphinx compilation and test succeeded" ${PSLIST} < $outfile
fi

popd >> $outfile 2>&1

chmod -R 755 pocketsphinx
/bin/rm -rf pocketsphinx

# Fresh download of sphinx2
svn co https://svn.sourceforge.net/svnroot/cmusphinx/trunk/sphinx2 > $outfile 2>&1

# Configure it
pushd sphinx2 >> $outfile 2>&1

./autogen.sh >> $outfile 2>&1 
./autogen.sh >> $outfile 2>&1 

# Compile and run test, and verify if both were successful
if ! ${GMAKE} distcheck >> $outfile 2>&1 ;
 then ${MAILX} -s "sphinx2 compilation failed" ${S2LIST} < $outfile
 elif ! (grep BESTPATH $outfile | grep 'GO FORWARD TEN METERS' > /dev/null);
 then ${MAILX} -s "sphinx2 test failed" ${S2LIST} < $outfile;
 else ${MAILX} -s "sphinx2 compilation and test succeeded" ${S2LIST} < $outfile
fi

popd >> $outfile 2>&1

chmod -R 755 sphinx2
/bin/rm -rf sphinx2

# Fresh download of sphinx3
svn co https://svn.sourceforge.net/svnroot/cmusphinx/trunk/sphinx3 > $outfile 2>&1

# Configure it
pushd sphinx3 >> $outfile 2>&1

./autogen.sh >> $outfile 2>&1 
./autogen.sh >> $outfile 2>&1 

# Compile and run test, and verify that all tests ran successfully
if ! ${GMAKE} distcheck DISTCHECK_CONFIGURE_FLAGS=--with-sphinxbase=`pwd`/../sphinxbase >> $outfile 2>&1 ;
 then ${MAILX} -s "sphinx3 compilation failed" ${S3LIST} < $outfile;
 elif grep FAILED $outfile > /dev/null;
 then ${MAILX} -s "Sphinx3 test failed" ${S3LIST} < $outfile;
 else ${MAILX} -s "sphinx3 compilation and test succeeded" ${S3LIST} < $outfile;
fi

popd >> $outfile 2>&1

# Check coding style, but send message only if script executed
# successfully. The script will fail, for example, if indent doesn't
# exist, or doesn't have the same options, in the machine.
./src/tests/regression/checkStyle.sh > $outfile 2>&1 && ${MAILX} -s "Coding style" ${S3LIST} < $outfile

chmod -R 755 sphinx3
/bin/rm -rf sphinx3

# Fresh download of SphinxTrain
svn co https://svn.sourceforge.net/svnroot/cmusphinx/trunk/SphinxTrain > $outfile 2>&1


# Configure it
pushd SphinxTrain >> $outfile 2>&1
./configure >> $outfile 2>&1

# Compile and make sure it's successful
if ! ${GMAKE} all >> $outfile 2>&1 ;
 then ${MAILX} -s "SphinxTrain compilation failed" ${STLIST} < $outfile;
 else ${MAILX} -s "SphinxTrain compilation succeeded" ${STLIST} < $outfile;
fi

popd >> $outfile 2>&1

chmod -R 755 SphinxTrain
/bin/rm -rf SphinxTrain

# Remove what we created
cd /tmp
/bin/rm -rf $root



