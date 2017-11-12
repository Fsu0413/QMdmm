include($$_PRO_FILE_PWD_/../QMdmm.pri)

CONFIG -= qt

TARGET = QMdmmServer
TEMPLATE = lib

!staticlib: DEFINES += QMDMMSERVER_LIBRARY

CONFIG += precompiled_header

PRECOMPILED_HEADER = qmdmmserverglobal.h

SOURCES += \
    qmdmmserver.cpp \
    qmdmmsocket.cpp \
    qmdmmserverplayer.cpp \
    qmdmmprotocol.cpp \
    qmdmmserverroom.cpp

HEADERS += \
    qmdmmserverglobal.h \
    qmdmmserver.h \
    qmdmmsocket.h \
    qmdmmserverplayer.h \
    qmdmmprotocol.h \
    qmdmmserverroom.h

DESTDIR = $$OUT_PWD/../dist/lib
DLLDESTDIR = $$OUT_PWD/../dist/bin

LIBS += -lQMdmmCore

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
