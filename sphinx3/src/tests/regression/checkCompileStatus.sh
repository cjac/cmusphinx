#!/bin/bash -x

username=$1

export PATH="/usr/local/bin:/bin:/usr/bin"

root=/tmp/sphinxCompilation.$$
S2LIST='archan egouvea yitao dhuggins'
S3LIST='archan egouvea yitao dhuggins'
S4LIST='archan egouvea yitao dhuggins'
STLIST='archan egouvea yitao dhuggins'

mkdir $root
cd $root

export CVS_RSH=/usr/local/bin/ssh

/usr/local/bin/cvs -d:ext:${username}@cvs.sf.net:/cvsroot/cmusphinx co sphinx2 >> ./test.out

pushd sphinx2 
./autogen.sh >> ./test.out 2>&1 
./autogen.sh >> ./test.out 2>&1 

if ! make all test >> test.out 2>&1 ;
 then mhmail -s "sphinx2 compilation or test failed" ${S2LIST} <test.out
 else mhmail -s "sphinx2 compilation or test succeeded" ${S2LIST} <test.out
 fi

popd

/usr/local/bin/cvs -d:ext:${username}@cvs.sf.net:/cvsroot/cmusphinx co sphinx3 >> ./test.out

pushd sphinx3

./autogen.sh >> ./test.out 2>&1 
./autogen.sh >> ./test.out 2>&1 

if ! make all test-full >> test.out 2>&1 ;
 then mhmail -s "sphinx3 compilation failed" ${S3LIST} < test.out;
 elif ! (grep FWDVIT ../test.out | grep -q 'P I T T S B U R G H');
 then mhmail -s "Sphinx3 test failed" ${S3LIST} < ../test.out;
 elif ! (grep BSTPTH ../test.out | grep -q 'P I T T S B U R G H');
 then mhmail -s "Sphinx3 test failed" ${S3LIST} < ../test.out;
 elif grep -q FAILED ../test.out;
 then mhmail -s "Sphinx3 test failed" ${S3LIST} < ../test.out;
 else mhmail -s "sphinx3 test succeeded" ${S3LIST} < test.out;
 fi

popd 

/usr/local/bin/cvs -d:ext:${username}@cvs.sf.net:/cvsroot/cmusphinx co SphinxTrain >> ./test.out

pushd SphinxTrain
./configure >> ./test
if ! make all >> test.out 2>&1 ;
 then mhmail -s "SphinxTrain compilation failed" ${STLIST} < test.out;
 else mhmail -s "SphinxTrain compilation succeeded" ${STLIST} < test.out;
 fi
popd

cd /tmp
/bin/rm -rf $root



