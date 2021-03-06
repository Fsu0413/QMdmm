# SPDX-License-Identifier: AGPL-3.0-or-later

include($$_PRO_FILE_PWD_/../QMdmm.pri)

CONFIG -= qt

TARGET = QMdmmServer
TEMPLATE = lib

!staticlib: DEFINES += QMDMMSERVER_LIBRARY

CONFIG += precompiled_header

PRECOMPILED_HEADER = qmdmmserverglobal.h

SOURCES += \
    qmdmmserver.cpp \
    qmdmmserverplayer.cpp \
    qmdmmserverroom.cpp \
    qmdmmserversocket.cpp

HEADERS += \
    qmdmmserverglobal.h \
    qmdmmserver.h \
    qmdmmserverplayer.h \
    qmdmmserverroom.h \
    qmdmmserversocket.h

DESTDIR = $$OUT_PWD/../dist/lib
DLLDESTDIR = $$OUT_PWD/../dist/bin

LIBS += -lQMdmmCore

linux-* {
    QMAKE_CFLAGS += -pthread
    QMAKE_CXXFLAGS += -pthread
    QMAKE_LFLAGS += -pthread
}

generateHeaders.target = $$system_path($$OUT_PWD/../dist/include/QMdmmServer/.timestamp)
!build_pass: mkpath($$OUT_PWD/../dist/include/QMdmmServer)

contains(QMAKE_HOST.os, "Windows"): generateHeaders.commands = cscript $$system_path($$PWD/../tools/AutoGenerateHeader.vbs) -o $$system_path($$OUT_PWD/../dist/include/QMdmmServer) -f $$system_path($$PWD/)
else: generateHeaders.commands = $$PWD/../tools/AutoGenerateHeader.sh -o $$OUT_PWD/../dist/include/QMdmmServer -f $$PWD/

HEADERS_ABSOLUTE =
for (header, HEADERS): HEADERS_ABSOLUTE += $$system_path($$absolute_path($$header))

generateHeaders.depends = $$HEADERS_ABSOLUTE

QMAKE_EXTRA_TARGETS += generateHeaders
PRE_TARGETDEPS += $$generateHeaders.target

includetarget.path = /include/QMdmmServer/
includetarget.files = $$OUT_PWD/../dist/include/QMdmmServer/*

target.path = /lib/
dlltarget.path = /bin/
INSTALLS += target dlltarget includetarget

QMAKE_CLEAN += $$system_path($$OUT_PWD/../dist/include/QMdmmServer/.timestamp)
