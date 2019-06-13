#!/bin/sed -f
# centerclient_makefile.sed
/^libCenterClientLibrary_la_SOURCES/ a\
include_Headers=CenterClient.h

/^libCenterClientLibrary_la_LIBADD/ c\
libCenterClientLibrary_la_LIBADD = $(ALLLIBS) /usr/local/lib/libevent_core.a /usr/local/lib/libevent_pthreads.a

/^libCenterClientLibrary_la_DEFINES/ c\
libCenterClientLibrary_la_DEFINES = -D_DEBUG -D__LINUX__ -w -std=c++0x

/^libCenterClientLibrary_la_INCLUDES/ s/WIN32-Code\/nmake/include/g

/^libCenterClientLibrary_la_INCLUDES/ s/WIN32-Code/include/g

/^libCenterClientLibrary_la_CFLAGS/ a\
export libCenterClientLibrary_la_INCLUDES
