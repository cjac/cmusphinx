#!/usr/bin/sh

# 
# compile the CMUCUSLMTk into native Windows executables using the Cygwin environment
# You should run this in a Cygwin shell

echo "Results will appear in $PWD"

if  [ -e Makefile ]; then
    make clean
fi

./autogen.sh --prefix $PWD --exec-prefix $PWD \
    --bindir $PWD/bin --libdir $PWD/lib \
    CFLAGS='-mno-cygwin'

#
