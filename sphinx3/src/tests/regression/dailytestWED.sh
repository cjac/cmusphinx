# The file format is:
#
# /path/to/test/location  target
#
# where the path is to the location where the test is located
# (e.g. ti46, rm1 etc) and target is a target for a single test
# (as opposed to a target that points to other targets)
#
./src/tests/performance/rm1 rm1_flat_unigram-lp
./src/tests/performance/rm1 rm1_unigram-lp
./src/tests/performance/rm1 rm1_bigram-lp
./src/tests/performance/rm1 rm1_trigram-lp 
./src/tests/performance/rm1 rm1_flat_unigram_quick-lp
./src/tests/performance/rm1 rm1_unigram_quick-lp
./src/tests/performance/rm1 rm1_bigram_quick-lp
./src/tests/performance/rm1 rm1_trigram_quick-lp
