#!/bin/sh

set -e

TOP=`pwd`

cd "$TOP"/sphinxbase
make distclean || true
autoreconf -i
cd "$TOP"/pocketsphinx
make distclean || true
autoreconf -i
cd "$TOP"

rm -rf "$TOP"/sphinxbase/bfin-uclinux "$TOP"/pocketsphinx/bfin-uclinux
mkdir "$TOP"/sphinxbase/bfin-uclinux "$TOP"/pocketsphinx/bfin-uclinux

cd "$TOP"/sphinxbase/bfin-uclinux
../configure --enable-fixed --without-lapack --host=bfin-uclinux --build=i686-linux
make -j3

cd "$TOP"/pocketsphinx/bfin-uclinux
../configure --with-sphinxbase="$TOP"/sphinxbase \
    --with-sphinxbase-build="$TOP"/sphinxbase/bfin-uclinux \
    --host=bfin-uclinux --build=i686-linux
make -j3

cd "$TOP"
rm -rf "$TOP"/sphinxbase/i386-linux "$TOP"/pocketsphinx/i386-linux
mkdir "$TOP"/sphinxbase/i386-linux "$TOP"/pocketsphinx/i386-linux

cd "$TOP"/sphinxbase/i386-linux
../configure
make -j3

cd "$TOP"/pocketsphinx/i386-linux
../configure --with-sphinxbase="$TOP"/sphinxbase \
    --with-sphinxbase-build="$TOP"/sphinxbase/i386-linux
make -j3
