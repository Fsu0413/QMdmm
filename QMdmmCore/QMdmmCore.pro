include($$_PRO_FILE_PWD_/../QMdmm.pri)

CONFIG -= qt

TARGET = QMdmmCore
TEMPLATE = lib

!staticlib: DEFINES += QMDMMCORE_LIBRARY

CONFIG += precompiled_header

PRECOMPILED_HEADER = qmdmmcoreglobal.h

SOURCES += qmdmmcore.cpp \
    qmdmmroom.cpp \
    qmdmmplayer.cpp \
    qmdmmstonescissorscloth.cpp \
    qmdmmprotocol.cpp

QMDMM_HEADERS +=\
    qmdmmroom.h \
    qmdmmplayer.h \
    qmdmmcoreglobal.h \
    qmdmmprotocol.h \
    qmdmmstonescissorscloth.h

HEADERS += $$QMDMM_HEADERS

DESTDIR = $$OUT_PWD/../dist/lib
DLLDESTDIR = $$OUT_PWD/../dist/bin

!system_jsoncpp {
    include($$PWD/3rdparty/jsoncpp/jsoncpp.pri)
}

generateHeaders.target = $$system_path($$OUT_PWD/../dist/include/QMdmmCore/.timestamp)
!build_pass: mkpath($$OUT_PWD/../dist/include/QMdmmCore)

contains(QMAKE_HOST.os, "Windows"): generateHeaders.commands = cscript $$system_path($$PWD/../tools/AutoGenerateHeader.vbs) -o $$system_path($$OUT_PWD/../dist/include/QMdmmCore) -f $$system_path($$PWD/)
else: generateHeaders.commands = $$PWD/../tools/AutoGenerateHeader.sh -o $$OUT_PWD/../dist/include/QMdmmCore -f $$PWD/

HEADERS_ABSOLUTE =
for (header, QMDMM_HEADERS): HEADERS_ABSOLUTE += $$system_path($$absolute_path($$header))

generateHeaders.depends = $$HEADERS_ABSOLUTE

QMAKE_EXTRA_TARGETS += generateHeaders
PRE_TARGETDEPS += $$generateHeaders.target

includetarget.path = /include/QMdmmCore/
includetarget.files = $$OUT_PWD/../dist/include/QMdmmCore/*

target.path = /lib/
dlltarget.path = /bin/
INSTALLS += target dlltarget includetarget
