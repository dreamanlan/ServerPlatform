#!/bin/bash

#必须在ServerCenter工程文件目录执行此脚本！！！

../compile/runscp ../compile/convert.scp ServerCenter
../compile/servercenter_makefile.sed Makefile.gen > Makefile.am
autoscan
../compile/servercenter_configure.sed configure.scan > configure.ac
touch Makefile.common.in
aclocal
autoheader
automake -a -c --foreign
autoconf
./configure
make -j4