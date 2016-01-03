#!/bin/sed -f
# servercenter_makefile.sed
/^ServerCenter_LDADD/ c\
ServerCenter_LDADD = $(ALLLIBS) /home/lanxiang/libevent/lib/libevent_core.a /home/lanxiang/libevent/lib/libevent_pthreads.a /usr/lib64/libcurses.a -lrt

/^ServerCenter_DEFINES/ c\
ServerCenter_DEFINES = -D_DEBUG -D_CONSOLE -D__LINUX__ -w -std=c++0x

/^ServerCenter_INCLUDES/ s/WIN32-Code/include/g

/^ServerCenter_CFLAGS/ a export ServerCenter_INCLUDES
