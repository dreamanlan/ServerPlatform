#!/bin/bash

#������CenterHubLibrary�����ļ�Ŀ¼ִ�д˽ű�������

../compile_linux/runscp ../compile_linux/convert.scp CenterHubLibrary
../compile_linux/centerhub_makefile.sed Makefile.gen > Makefile.am
autoscan
../compile_linux/centerhub_configure.sed configure.scan > configure.ac
touch Makefile.common.in
libtoolize
aclocal
autoheader
automake -a -c --foreign
autoconf
./configure --prefix=/home/lanxiang/centerhub
make -j4