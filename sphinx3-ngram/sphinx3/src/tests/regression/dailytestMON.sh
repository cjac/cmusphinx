# The file format is:
#
# /path/to/test/location  target
#
# where the path is to the location where the test is located
# (e.g. ti46, rm1 etc) and target is a target for a single test
# (as opposed to a target that points to other targets)
#
./src/tests/performance/ti46 ti46-quick-lp
./src/tests/performance/ti46 ti46-quick-decode
./src/tests/performance/ti46 ti46-quick-decany
./src/tests/performance/ti46 ti46-lp
./src/tests/performance/ti46 ti46-decode
./src/tests/performance/ti46 ti46-decany
