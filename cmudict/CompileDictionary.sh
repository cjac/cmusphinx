#!/usr/bin/sh
# [20080422] (air) Compile cmudict into SPHINX_40 form

# make_baseforms.pl simply removes stress marks
perl bin/make_baseform.pl cmudict.0.7a sphinxdict/cmudict.0.7a_SPHINX_40

#
