#!/bin/bash

#必须在ServerCenter工程文件目录执行此脚本！！！

../compile_linux/runscp ../compile_linux/convert.scp ServerCenter
../compile_linux/servercenter_makefile.sed Makefile.gen > Makefile.am
autoscan
../compile_linux/servercenter_configure.sed configure.scan > configure.ac
touch Makefile.common.in
aclocal
autoheader
automake -a -c --foreign
autoconf
./configure
make -j4