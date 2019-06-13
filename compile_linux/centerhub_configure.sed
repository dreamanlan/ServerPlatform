#!/bin/sed -f
# centerhub_configure.sed
/AC_CONFIG_HEADER/ a\
AM_INIT_AUTOMAKE(CenterHubLibrary,1.0) \
AC_CONFIG_MACRO_DIR([m4]) \
AC_PROG_LIBTOOL

/Checks for libraries/ a\
AC_CHECK_LIB([m], [floor])\
AC_CHECK_LIB([pthread], [pthread_create])
