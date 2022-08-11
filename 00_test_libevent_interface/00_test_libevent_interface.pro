TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c

#libevent编译出来安装的库
LIBS += /usr/local/lib/libevent.so
