AC_PREREQ([2.65])
AC_INIT([consensus], 0.1)
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([src/Clustering.cc])

CXXFLAGS=${CXXFLAGS:--g -O3 -ffast-math -Wall}

AC_PROG_CXX
AC_PROG_CC

AC_FUNC_ALLOCA
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])

AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

AC_CHECK_FUNCS([mkdir strchr])
AC_CONFIG_FILES([Makefile
                 include/Makefile
                 src/Makefile])
AC_OUTPUT
