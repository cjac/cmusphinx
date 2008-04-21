Converted versions of cmudict; suitable as input to trainer and decoder
-----------------------------------------------------------------------
[20080219] (air)

The Sphinx-compatible version of the dictionary, compiled from the 
corresponding version of cmudict.

To produce a new version of the SPHINX dictionary do:

perl ./make_baseform.pl ../<cmudict.VERSION> cmudict.VERSION_SPHINX_40
Stress removal results in some duplicate pronunciations being removed.
This is noted in the STDERR output.

