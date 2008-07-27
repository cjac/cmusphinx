# Logios uitility
# [20080727] (air)
# Create a Resources tree (for Olympus-style knowledge base),
# or, at least, check that all the bits are there.
# Ensures that full tree is there; note that the compiler dies anyway if no grammar defined
# NB: must be run at the root of the project tree (but not really meant to be user-modifiable)

if [ ! -e Resources ]; then mkdir Resources; fi
if [ ! -e Resources/DecoderConfig ]; then mkdir Resources/DecoderConfig; fi
if [ ! -e Resources/DecoderConfig/Dictionary ]; then mkdir Resources/DecoderConfig/Dictionary; fi
if [ ! -e Resources/DecoderConfig/LanguageModel ]; then mkdir Resources/DecoderConfig/LanguageModel; fi
if [ ! -e Resources/DecoderConfig/LanguageModel/temp ]; then mkdir Resources/DecoderConfig/LanguageModel/TEMP; fi
if [ ! -e Resources/Grammar ]; then mkdir Resources/Grammar; fi
if [ ! -e Resources/Grammar/GRAMMAR ]; then mkdir Resources/Grammar/GRAMMAR; fi

#

