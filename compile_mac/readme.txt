linux编译注意事项：

1、首先编译libevent：
./configure --disable-shared --prefix=安装目录
make
make install

2、linux下需要连接libevent_core.a,libevent_pthreads.a
在admintool_makefile.sed里修改，假定libevent安装目录为/home/myself:
AdminTool_LDADD = $(ALLLIBS) /home/myself/lib/libevent_core.a /home/myself/lib/libevent_pthreads.a /usr/lib64/libcurses.a -lrt
最后的-lrt必须加上，否则会链接不上clock_gettime
由于网络模块用到了stdint.h头文件，这个是c++0x的，需要修改编译选项加上-std=c++0x
AdminTool_DEFINES = -D_DEBUG -D_CONSOLE -D__LINUX__ -w -std=c++0x

3、AdminTool依赖curses，最好使用静态库连接ncurses
在admintool_makefile.sed里修改，见2中写法

4、CenterClientLibrary需要生成动态库，目前静态链接了libevent，编译时有警告，采用了-fPIC编译选项编译libevent，仍然可能会有问题（再有问题需要改成动态链接）。