#!/bin/sh

# Clean thoroughly, just in case
(cd sphinx3; make -k distclean)

# Cycle through all machines
for file in `grep ^1 /etc/compilefarm-hosts  | grep - | awk '{print $NF}'`;
do
# Run test for each machine
ssh $file ./runtest.sh > $file.log 2>&1
done


