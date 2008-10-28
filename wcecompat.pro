TEMPLATE = lib
TARGET = wcecompat
DESTDIR = lib

DEPENDPATH += . include src include/sys
INCLUDEPATH += . include src include/sys

CONFIG += staticlib
CONFIG -= shared
QT=

LIBS += -lcorelibc -lcoredll /NODEFAULTLIB:LIBC.LIB

# Input
HEADERS += include/assert.h \
           include/conio.h \
           include/ctype.h \
           include/errno.h \
           include/fcntl.h \
           include/float.h \
           include/io.h \
           include/limits.h \
           include/malloc.h \
           include/math.h \
           include/memory.h \
           include/process.h \
           include/setjmp.h \
           include/signal.h \
           include/stdarg.h \
           include/stddef.h \
           include/stdio.h \
           include/stdlib.h \
           include/string.h \
           include/time.h \
           include/winsock_extras.h \
           src/args.h \
           src/ChildData.h \
           src/internal.h \
           src/pipe.h \
           src/redir.h \
           src/ts_string.h \
           include/sys/stat.h \
           include/sys/timeb.h \
           include/sys/types.h
SOURCES += src/args.cpp \
           src/assert.cpp \
           src/ChildData.cpp \
           src/env.cpp \
           src/errno.cpp \
           src/io.cpp \
           src/pipe.cpp \
           src/process.cpp \
           src/redir.cpp \
           src/stat.cpp \
           src/stdio_extras.cpp \
           src/stdlib_extras.cpp \
           src/string_extras.cpp \
           src/time.cpp \
           src/timeb.cpp \
           src/ts_string.cpp \
           src/winmain.cpp \
           src/winsock_extras.cpp

QMAKE_POST_LINK = copy $$DESTDIR\\$${TARGET}.lib $$DESTDIR\\$${TARGET}ex.lib

