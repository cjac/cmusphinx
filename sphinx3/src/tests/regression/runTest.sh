#!/bin/bash

# This script is usually run by compileFarmRunTest.sh

# Define paths, just in case
export PATH=/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/local/sbin
export LD_LIBRARY_PATH=/usr/lib:/usr/local/lib

# goto sphinx3 :-)
pushd sphinx3
# Create a machine-specific directory
mkdir `hostname`
# Run test from the machine-specific directory
cd `hostname`
# Do the dance: configure, make etc
../configure
make all test-full
# Copy the log files to a safe location that won't de destroyed by a clean
mkdir log
cp -p *.out log/
cp -p pittsburgh* log/
# Clean thoroughly the binaries, libraries, etc, in preparation for 
# another test
make distclean
popd

# Store all the information from a particular host.
mv sphinx3/`hostname` logCompile/ 

