#!/bin/sh

set -e
cd sphinxbase
./autogen.sh
make
make distcheck
cd ../pocketsphinx
./autogen.sh
make
make distcheck DISTCHECK_CONFIGURE_FLAGS=--with-sphinxbase=`pwd`/../sphinxbase
