#!sh
# [20080422] (air) Compile cmudict into SPHINX_40 form
# make_baseforms.pl removes stress marks and eliminates resulting duplicates

DICTIONARY=cmudict.0.7a


echo -n "Compiling $DICTIONARY..."
perl ./scripts/make_baseform.pl $DICTIONARY sphinxdict/${DICTIONARY}_SPHINX_40

echo ""
echo -n "Testing sphinx dictionary... "
./scripts/test_dict.pl -p sphinxdict/SphinxPhones_40 sphinxdict/${DICTIONARY}_SPHINX_40

#
