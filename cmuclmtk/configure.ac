#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.58)
AC_INIT([cmucuslmtk], [1.0], [air+logios@cs.cmu.edu])
AM_INIT_AUTOMAKE([cmucuslmtk],[1.0])
AM_INIT_AUTOMAKE([-Wall -Werror])
AC_CONFIG_SRCDIR([src/liblmest/disc_meth.h])
AC_CONFIG_HEADER([config.h])

# PUT EVETHING IN THIS DIRECTORY
prefix=`pwd`

dnl Optionally add support to 32 bits LM
dnl May be better to make this as default and do a --enable-16bits for small systems
AC_ARG_ENABLE(32bits,
           [  --enable-32bits         Enable 32 bit LM support],
           [CFLAGS="$CFLAGS -DTHIRTYTWOBITS"]
)
AC_ARG_ENABLE(mingw,
	[  --enable-mingw	  Compile for native Windows execution],
	[LDFLAGS="-mno-cygwin -L/lib/mingw -L/lib/gcc/i686-pc-mingw32/3.4.4 $LDFLAGS"
	 CC='gcc -mno-cygwin'
	]
)

AC_PROG_CC
AC_PROG_RANLIB

AC_PROG_INSTALL

AC_CHECK_TOOL(AR,ar)
dnl  libs/mips_swap.h has the endian-related stuff
AC_C_BIGENDIAN
AC_CHECK_LIB(m,log)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h strings.h sys/time.h])

#AC_CONFIG_SUBDIRS(src/expat)
AC_CONFIG_FILES([Makefile
		src/Makefile
		test/Makefile
		src/liblmest/Makefile
		src/libs/Makefile
		src/programs/Makefile
		])
AC_OUTPUT
