#!/bin/bash

../compile_mac/runscp ../compile_mac/convert.scp ServerCenter
sed -f ../compile_mac/servercenter_makefile.sed Makefile.gen > Makefile.am
autoscan
sed -f ../compile_mac/servercenter_configure.sed configure.scan > configure.ac
touch Makefile.common.in
aclocal
autoheader
automake -a -c --foreign
autoconf
./configure
make -j4