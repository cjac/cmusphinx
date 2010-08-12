#!/usr/bin/python
#
# Converts HTK models to Sphinx 3 models.
#
# @author W.J. Maaskant


from sys import exit

from htk_converter import *

# Parameters init.
if len(sys.argv) < 4:
	print "Usage: %s HMMDEFS TIEDLIST OUT_PATH\n" % (sys.argv[0])
	exit(1)
hmmdefs = sys.argv[1]
tiedlist = sys.argv[2]
prefix = sys.argv[3]

# Perform the conversion.
try:
	conv = HtkConverter()
	conv.load(hmmdefs, tiedlist)
	# conv.show() # Uncomment to show the loaded models. Save the output to a file, or else you won't be able to inspect the results.
	conv.writeS3(prefix)
except HtkConverterError, e:
	print e
