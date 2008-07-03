#!/bin/sh

set -e

platform=${1:-i386-linux}

echo "0.5:"
./wsj1_test5k.sh $platform-0.5-simple ../$platform/src/programs/pocketsphinx_batch
./wsj1_test5k_fast.sh $platform-0.5-fast ../$platform/src/programs/pocketsphinx_batch

echo "0.4.1:"
./wsj1_test5k.sh $platform-0.4-simple ../../../pocketsphinx-0.4/pocketsphinx/$platform/src/programs/pocketsphinx_batch
./wsj1_test5k_fast.sh $platform-0.4-fast ../../../pocketsphinx-0.4/pocketsphinx/$platform/src/programs/pocketsphinx_batch
