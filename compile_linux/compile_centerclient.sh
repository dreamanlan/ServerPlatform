#!/bin/bash

#������CenterClientLibrary�����ļ�Ŀ¼ִ�д˽ű�������

../compile_linux/runscp ../compile_linux/convert.scp CenterClientLibrary
../compile_linux/centerclient_makefile.sed Makefile.gen > Makefile.am
autoscan
../compile_linux/centerclient_configure.sed configure.scan > configure.ac
touch Makefile.common.in
libtoolize
aclocal
autoheader
automake -a -c --foreign
autoconf
./configure --prefix=/home/lanxiang/centerclient
make -j4