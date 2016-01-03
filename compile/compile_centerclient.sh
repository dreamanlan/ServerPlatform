#!/bin/bash

#必须在CenterClientLibrary工程文件目录执行此脚本！！！

../compile/runscp ../compile/convert.scp CenterClientLibrary
../compile/centerclient_makefile.sed Makefile.gen > Makefile.am
autoscan
../compile/centerclient_configure.sed configure.scan > configure.ac
touch Makefile.common.in
libtoolize
aclocal
autoheader
automake -a -c --foreign
autoconf
./configure --prefix=/home/lanxiang/centerclient
make -j4