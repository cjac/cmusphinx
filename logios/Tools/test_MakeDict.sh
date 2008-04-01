#!/usr/bin/sh
#
# test MakeDict for correct installation under linux and cygwin
# this is a quick test and may not be completely robust
# [20080401] (air)
#

if [ -z `which perl` ] ; then
echo -n "You need to have the perl interpreter installed to use MakeDict... "
exit 0 ;
else
echo "Perl installed"
echo -n "Testing for perl modules... " ;
fi

cat >_test_MakeDict_.pl <<EOF
sub catch { my \$sig = shift; 
print "failed on \$sig\n"; exit 1; }
\$SIG{QUIT} = 'catch'; 
\$SIG{HUP} = 'catch'; 
require Config;
require File::Spec;
require HTML::Template;
EOF

if [ `perl _test_MakeDict_.pl` ] ; then
rm -f _test_MakeDict_.pl
exit 0;
fi
rm -f _test_MakeDict_.pl
echo "all modules available"

echo "test MakeDict... "
PGM=MakeDict/make_pronunciation.pl
perl $PGM -tools . -resources MakeDict/test \
    -words example -handdict hand.dict -dict example.dict
X=`diff --strip-trailing-cr MakeDict/test/example.dict MakeDict/test/example.dic.test`
Y=`diff --strip-trailing-cr MakeDict/test/pronunciation.log MakeDict/test/pronunciation.log.test`
if  [ -n "$X" -o -n "$Y" ]; then
echo "FAILED";
else
echo 'SUCCEEDED';
fi
##
