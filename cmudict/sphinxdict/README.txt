Converted versions of cmudict; suitable as input to trainer and decoder
-----------------------------------------------------------------------
[20080219] (air)

The Sphinx-compatible version of the dictionary, compiled from the 
corresponding version of cmudict. For human browsing, please use the 
uncompiled version.

To produce a new version of the SPHINX dictionary do (in cmudict/):

perl ./scripts/make_baseform.pl <cmudict.VERSION> sphinxdict/cmudict.VERSION_SPHINX_40
Stress removal results in some duplicate pronunciations being removed.
This is noted in the STDERR output.

The following are convenience scripts:
CompileDictionary.bat
CompileDictionary.sh

