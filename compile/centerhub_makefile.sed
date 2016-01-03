#!/bin/sed -f
# centerhub_makefile.sed
/^libCenterHubLibrary_la_SOURCES/ a\
include_Headers=CenterHub.h

/^libCenterHubLibrary_la_LIBADD/ c\
libCenterHubLibrary_la_LIBADD = $(ALLLIBS) /home/lanxiang/libevent/lib/libevent_core.a /home/lanxiang/libevent/lib/libevent_pthreads.a /usr/lib64/libcurses.a -lrt

/^libCenterHubLibrary_la_DEFINES/ c\
libCenterHubLibrary_la_DEFINES = -D_DEBUG -D__LINUX__ -w -std=c++0x

/^libCenterHubLibrary_la_INCLUDES/ s/WIN32-Code/include/g

/^libCenterHubLibrary_la_CFLAGS/ a export libCenterHubLibrary_la_INCLUDES
