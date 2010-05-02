#!/bin/sh
#
# test MakeDict for correct installation under linux and cygwin
# this is a quicky test and may not be completely robust
#
# [20080401] (air)
# depending on your cygwin installation filetype (ie, dos/unix),
# you may need to fix this script as follows (svn is a bit clueless):
#
#   sed -e 's/\r//' test_MakeDict.sh > cygw_MakeDict.sh
#


echo -n "Looking for perl... "
if [ -z `which perl` ] ; then
echo "missing. please fix PATH or install perl."
exit 0 ;
else
echo "perl installed, great!"
fi

echo -n "Testing for perl modules... " ;
cat >_test_MakeDict_.pl <<EOF
sub catch { exit ; } 
\$SIG{QUIT} = 'catch'; \$SIG{HUP} = 'catch'; 
require Config; require File::Spec; require HTML::Template;
print "ok";
EOF

if [ -z `perl _test_MakeDict_.pl` ] ; then
rm -f _test_MakeDict_.pl
exit 0;
fi
rm -f _test_MakeDict_.pl
echo "all necessary modules available!"

echo -n "test MakeDict... "
rm -f test/example.dict test/pronunciation.log
PGM=make_pronunciation.pl
if perl $PGM -tools .. -dictdir test \
    -words example -handdict hand.dict -dict example.dict
then
    echo " $PGM failed"
    exit
fi

X=`diff --strip-trailing-cr test/example.dict test/example.dic.test`
Y=`diff --strip-trailing-cr test/pronunciation.log test/pronunciation.log.test`
if  [ -n "$X" -o -n "$Y" ]; then
echo "FAILED";
else
echo ''; echo 'SUCCEEDED';
fi

##
