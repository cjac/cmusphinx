#!/bin/sh

# Clean thoroughly, just in case
(cd sphinx3; make -k distclean)

mkdir ./logCompile
# Cycle through all machines
for file in `grep ^1 /etc/compilefarm-hosts  | grep - | awk '{print $NF}'`;
do
# Run test for each machine
ssh $file ./runTest.sh > $file.log 2>&1
mv $file.log ./logCompile 
done





