#!/bin/bash

../compile_mac/runscp ../compile_mac/convert.scp CenterHubLibrary
sed -f ../compile_mac/centerhub_makefile.sed Makefile.gen > Makefile.am
autoscan
sed -f ../compile_mac/centerhub_configure.sed configure.scan > configure.ac
touch Makefile.common.in
libtoolize
aclocal
autoheader
automake -a -c --foreign
autoconf
./configure
make -j4