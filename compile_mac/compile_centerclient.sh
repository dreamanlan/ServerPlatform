#!/bin/bash

../compile_mac/runscp ../compile_mac/convert.scp CenterClientLibrary
sed -f ../compile_mac/centerclient_makefile.sed Makefile.gen > Makefile.am
autoscan
sed -f ../compile_mac/centerclient_configure.sed configure.scan > configure.ac
touch Makefile.common.in
libtoolize
aclocal
autoheader
automake -a -c --foreign
autoconf
./configure
make -j4