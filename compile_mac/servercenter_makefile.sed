#!/bin/sed -f
# servercenter_makefile.sed
/^ServerCenter_LDADD/ c\
ServerCenter_LDADD = $(ALLLIBS) /usr/local/lib/libevent_core.a /usr/local/lib/libevent_pthreads.a

/^ServerCenter_DEFINES/ c\
ServerCenter_DEFINES = -D_DEBUG -D_CONSOLE -D__LINUX__ -w -std=c++0x

/^ServerCenter_INCLUDES/ s/Win32-Code\/nmake/include/g

/^ServerCenter_INCLUDES/ s/Win32-Code/include/g

/^ServerCenter_CFLAGS/ a\
export ServerCenter_INCLUDES
