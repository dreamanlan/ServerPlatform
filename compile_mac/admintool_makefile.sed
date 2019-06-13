#!/bin/sed -f
# admintool_makefile.sed
/^AdminTool_LDADD/ c\
AdminTool_LDADD = $(ALLLIBS) /usr/local/lib/libevent_core.a /usr/local/lib/libevent_pthreads.a

/^AdminTool_DEFINES/ c\
AdminTool_DEFINES = -D_DEBUG -D_CONSOLE -D__LINUX__ -w

/^AdminTool_INCLUDES/ s/WIN32-Code\/nmake/include/g

/^AdminTool_INCLUDES/ s/WIN32-Code/include/g

/^AdminTool_CFLAGS/ a\
export AdminTool_INCLUDES
