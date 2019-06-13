#!/bin/sed -f
# admintool_configure.sed
/AC_CONFIG_HEADER/ a\
AM_INIT_AUTOMAKE(AdminTool,1.0)\
AC_PROG_RANLIB(RANLIB)

/Checks for libraries/ a\
AC_CHECK_LIB([m], [floor])\
AC_CHECK_LIB([pthread], [pthread_create])
