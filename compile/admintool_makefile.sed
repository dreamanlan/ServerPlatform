#!/bin/sed -f
# admintool_makefile.sed
/^AdminTool_LDADD/ c\
AdminTool_LDADD = $(ALLLIBS) /home/lanxiang/libevent/lib/libevent_core.a /home/lanxiang/libevent/lib/libevent_pthreads.a /usr/lib64/libcurses.a -lrt

/^AdminTool_DEFINES/ c\
AdminTool_DEFINES = -D_DEBUG -D_CONSOLE -D__LINUX__ -w -std=c++0x

/^AdminTool_INCLUDES/ s/WIN32-Code/include/g

/^AdminTool_CFLAGS/ a export AdminTool_INCLUDES
