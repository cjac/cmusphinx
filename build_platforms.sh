#!/bin/sh

set -e

TOP=`pwd`
BUILD=${1:-i386-linux}

cd "$TOP"/sphinxbase
make distclean || true
cd "$TOP"/pocketsphinx
make distclean || true
cd "$TOP"

if command -v bfin-uclinux-gcc >/dev/null 2>&1; then
    rm -rf "$TOP"/sphinxbase/bfin-uclinux "$TOP"/pocketsphinx/bfin-uclinux
    mkdir "$TOP"/sphinxbase/bfin-uclinux "$TOP"/pocketsphinx/bfin-uclinux

    cd "$TOP"/sphinxbase/bfin-uclinux
    configure=configure
    test -x ../configure || configure=autogen.sh
    ../$configure --enable-fixed --without-lapack --host=bfin-uclinux --build=$BUILD
    make -j3

    cd "$TOP"/pocketsphinx/bfin-uclinux
    configure=configure
    test -x ../configure || configure=autogen.sh
    ../$configure --with-sphinxbase="$TOP"/sphinxbase \
	--with-sphinxbase-build="$TOP"/sphinxbase/bfin-uclinux \
	--host=bfin-uclinux --build=$BUILD
    make -j3
fi

cd "$TOP"
rm -rf "$TOP"/sphinxbase/$BUILD "$TOP"/pocketsphinx/$BUILD
mkdir "$TOP"/sphinxbase/$BUILD "$TOP"/pocketsphinx/$BUILD

cd "$TOP"/sphinxbase/$BUILD
configure=configure
test -x ../configure || configure=autogen.sh
../$configure
make -j3

cd "$TOP"/pocketsphinx/$BUILD
configure=configure
test -x ../configure || configure=autogen.sh
../$configure --with-sphinxbase="$TOP"/sphinxbase \
    --with-sphinxbase-build="$TOP"/sphinxbase/$BUILD
make -j3
