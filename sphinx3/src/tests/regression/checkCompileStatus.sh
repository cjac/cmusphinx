#!/bin/bash -x

export PATH="/usr/local/bin:/bin:/usr/bin"

root=/tmp/sphinx3.$$
LIST='egouvea archan yitao'

mkdir $root
cd $root

export CVS_RSH=/usr/local/bin/ssh

/usr/local/bin/cvs -d:ext:cvs.sf.net:/cvsroot/cmusphinx co sphinx3

cd sphinx3

./autogen.sh
./autogen.sh

if ! make all test > test.out 2>&1;
 then mhmail -s "Sphinx3 compilation failed" ${LIST} < test.out;
 elif ! grep 'FWDVIT: P I T T S B U R G H' test.out;
 then mhmail -s "Sphinx3 test failed" ${LIST} < test.out;
 fi

cd /tmp
/bin/rm -rf $root
