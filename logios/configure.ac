#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.58)
AC_INIT(FULL-PACKAGE-NAME, VERSION, BUG-REPORT-ADDRESS)
AM_INIT_AUTOMAKE
AC_PROG_RANLIB
AC_CONFIG_SRCDIR([Tools/cfg2ngram/src/cfg2wngram.cpp])
AC_CONFIG_HEADER([config.h])

# Native Windows compilation
AC_ARG_ENABLE(mingw,
	[--enable-mingw	  Compile for native Windows execution],
	[LDFLAGS="-mno-cygwin -L/lib/mingw -L/lib/gcc/i686-pc-mingw32/3.4.4 $(LDFLAGS)"
	 CC="gcc -mno-cygwin"
	]
)

# Checks for programs.
AC_PROG_CXX
# tk: I don't think we need plain c
#AC_PROG_CC

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST

# Checks for library functions.
AC_HEADER_STDC

AC_OUTPUT([Makefile \
           Tools/Makefile \
	   Tools/cfg2ngram/Makefile \
           Tools/cfg2ngram/src/Makefile \
           Tools/cfg2ngram/src/pcfgsrc/Makefile \
           Tools/MakeGra/Makefile \
           Tools/MakeGra/phoenix_v0/Makefile \
           Tools/MakeGra/phoenix_v0/ParserLib/Makefile])
