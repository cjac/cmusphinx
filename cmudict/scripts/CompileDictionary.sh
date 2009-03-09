#!sh
# [20080422] (air) Compile cmudict into SPHINX_40 form
# make_baseforms.pl removes stress marks and eliminates resulting duplicates

perl scripts/make_baseform.pl cmudict.0.7a sphinxdict/cmudict.0.7a_SPHINX_40

#
