linux����ע�����

1�����ȱ���libevent��
./configure --disable-shared --prefix=��װĿ¼
make
make install

2��linux����Ҫ����libevent_core.a,libevent_pthreads.a
��admintool_makefile.sed���޸ģ��ٶ�libevent��װĿ¼Ϊ/home/myself:
AdminTool_LDADD = $(ALLLIBS) /home/myself/lib/libevent_core.a /home/myself/lib/libevent_pthreads.a /usr/lib64/libcurses.a -lrt
����-lrt������ϣ���������Ӳ���clock_gettime
��������ģ���õ���stdint.hͷ�ļ��������c++0x�ģ���Ҫ�޸ı���ѡ�����-std=c++0x
AdminTool_DEFINES = -D_DEBUG -D_CONSOLE -D__LINUX__ -w -std=c++0x

3��AdminTool����curses�����ʹ�þ�̬������ncurses
��admintool_makefile.sed���޸ģ���2��д��

4��CenterClientLibrary��Ҫ���ɶ�̬�⣬Ŀǰ��̬������libevent������ʱ�о��棬������-fPIC����ѡ�����libevent����Ȼ���ܻ������⣨����������Ҫ�ĳɶ�̬���ӣ���