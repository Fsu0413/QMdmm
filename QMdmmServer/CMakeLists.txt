# SPDX-License-Identifier: AGPL-3.0-or-later

set(QMDMMSERVER_HEADERS
    qmdmmserverglobal.h

    qmdmmserver.h
    qmdmmserverplayer.h
    qmdmmserverroom.h
    qmdmmserversocket.h
)

set(QMDMMSERVER_SOURCES
    qmdmmserver.cpp
    qmdmmserverplayer.cpp
    qmdmmserverroom.cpp
    qmdmmserversocket.cpp
)

find_classes_in_header_files("${QMDMMSERVER_HEADERS}" "include/QMdmmServer" QMDMMSERVER_GENERATED_HEADERS)

add_library(QMdmmServer
    ${QMDMMSERVER_HEADERS}
    ${QMDMMSERVER_SOURCES}
    ${QMDMMSERVER_GENERATED_HEADERS}
)

target_precompile_headers(QMdmmServer
    PRIVATE qmdmmserverglobal.h
)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(QMdmmServer
        PRIVATE QMDMMSERVER_LIBRARY
    )
else()
    target_compile_definitions(QMdmmServer
        PUBLIC QMDMM_STATIC
    )
endif()

if (ANDROID OR IOS)
    target_compile_definitions(QMdmmServer
        PUBLIC MOBILE_DEVICES
    )
endif()

if (CMAKE_DL_LIBS)
    target_link_libraries(QMdmmServer
        PUBLIC ${CMAKE_DL_LIBS}
    )
endif()

target_link_libraries(QMdmmServer
    PUBLIC QMdmmCore
)

#!win32-msvc* {
#    # we use gcc/clang on unix-like systems and mingw
#    QMAKE_CFLAGS += -Wpointer-to-int-cast
#    QMAKE_CXXFLAGS += -Wc++11-compat
#    *-g++: QMAKE_CXXFLAGS += -Wzero-as-null-pointer-constant
#    mac {
#        # macOS and iOS does not use GNU ld
#        QMAKE_LFLAGS += -Wl,-undefined -Wl,error
#    } else {
#        # treat any other linker as GNU ld temporily
#        QMAKE_LFLAGS += -Wl,--no-undefined
#    }
#}

#generateHeaders.target = $$system_path($$OUT_PWD/../dist/include/QMdmmServer/.timestamp)
#!build_pass: mkpath($$OUT_PWD/../dist/include/QMdmmServer)

#contains(QMAKE_HOST.os, "Windows"): generateHeaders.commands = cscript $$system_path($$PWD/../tools/AutoGenerateHeader.vbs) -o $$system_path($$OUT_PWD/../dist/include/QMdmmServer) -f $$system_path($$PWD/)
#else: generateHeaders.commands = $$PWD/../tools/AutoGenerateHeader.sh -o $$OUT_PWD/../dist/include/QMdmmServer -f $$PWD/

#HEADERS_ABSOLUTE =
#for (header, HEADERS): HEADERS_ABSOLUTE += $$system_path($$absolute_path($$header))

#generateHeaders.depends = $$HEADERS_ABSOLUTE

#QMAKE_EXTRA_TARGETS += generateHeaders
#PRE_TARGETDEPS += $$generateHeaders.target

#includetarget.path = /include/QMdmmServer/
#includetarget.files = $$OUT_PWD/../dist/include/QMdmmServer/*

#target.path = /lib/
#dlltarget.path = /bin/
#INSTALLS += target dlltarget includetarget

#QMAKE_CLEAN += $$system_path($$OUT_PWD/../dist/include/QMdmmServer/.timestamp)
