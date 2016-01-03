#!/bin/bash

#必须在CenterHubLibrary工程文件目录执行此脚本！！！

../compile/runscp ../compile/convert.scp CenterHubLibrary
../compile/centerhub_makefile.sed Makefile.gen > Makefile.am
autoscan
../compile/centerhub_configure.sed configure.scan > configure.ac
touch Makefile.common.in
libtoolize
aclocal
autoheader
automake -a -c --foreign
autoconf
./configure --prefix=/home/lanxiang/centerhub
make -j4