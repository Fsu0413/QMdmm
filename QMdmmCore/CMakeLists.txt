# SPDX-License-Identifier: AGPL-3.0-or-later

set(QMDMMCORE_HEADERS
    qmdmmcoreglobal.h

    qmdmmroom.h
    qmdmmplayer.h
    qmdmmcoreglobal.h
    qmdmmprotocol.h
    qmdmmstonescissorscloth.h
)

set(QMDMMCORE_SOURCES
    qmdmmcore.cpp
    qmdmmroom.cpp
    qmdmmplayer.cpp
    qmdmmstonescissorscloth.cpp
    qmdmmprotocol.cpp
)

find_classes_in_header_files("${QMDMMCORE_HEADERS}" "include/QMdmmCore" QMDMMCORE_GENERATED_HEADERS)

add_library(QMdmmCore
    ${QMDMMCORE_HEADERS}
    ${QMDMMCORE_SOURCES}
    ${QMDMMCORE_GENERATED_HEADERS}
)

target_precompile_headers(QMdmmCore
    PRIVATE qmdmmcoreglobal.h
)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(QMdmmCore
        PRIVATE QMDMMCORE_LIBRARY
    )
else()
    target_compile_definitions(QMdmmCore
        PUBLIC QMDMM_STATIC
    )
endif()

if (ANDROID OR IOS)
    target_compile_definitions(QMdmmCore
        PUBLIC MOBILE_DEVICES
    )
endif()

target_include_directories(QMdmmCore
    INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/include"
    PUBLIC .
)

if (JSONCPP_FOUND)
    target_link_libraries(QMdmmCore
        PUBLIC PkgConfig::JSONCPP
    )
else()
    add_library(QMdmmInternalJsonCppLibrary
        3rdparty/jsoncpp/json_reader.cpp
        3rdparty/jsoncpp/json_value.cpp
        3rdparty/jsoncpp/json_writer.cpp
        3rdparty/jsoncpp/json/config.h
        3rdparty/jsoncpp/json/features.h
        3rdparty/jsoncpp/json/forwards.h
        3rdparty/jsoncpp/json/json.h
        3rdparty/jsoncpp/json/reader.h
        3rdparty/jsoncpp/json/value.h
        3rdparty/jsoncpp/json/version.h
        3rdparty/jsoncpp/json/writer.h
        3rdparty/jsoncpp/json/allocator.h
        3rdparty/jsoncpp/json/assertions.h
        3rdparty/jsoncpp/json/autolink.h
        3rdparty/jsoncpp/json_tool.h
    )
    target_include_directories(QMdmmInternalJsonCppLibrary
        PUBLIC 3rdparty/jsoncpp
    )
    if (BUILD_SHARED_LIBS)
        target_compile_definitions(QMdmmInternalJsonCppLibrary
            PRIVATE JSON_DLL_BUILD
        )
    endif()
    target_link_libraries(QMdmmCore
        PUBLIC QMdmmInternalJsonCppLibrary
    )
endif()

#generateHeaders.target = $$system_path($$OUT_PWD/../dist/include/QMdmmCore/.timestamp)
#!build_pass: mkpath($$OUT_PWD/../dist/include/QMdmmCore)

#contains(QMAKE_HOST.os, "Windows"): generateHeaders.commands = cscript $$system_path($$PWD/../tools/AutoGenerateHeader.vbs) -o $$system_path($$OUT_PWD/../dist/include/QMdmmCore) -f $$system_path($$PWD/)
#else: generateHeaders.commands = $$PWD/../tools/AutoGenerateHeader.sh -o $$OUT_PWD/../dist/include/QMdmmCore -f $$PWD/

#HEADERS_ABSOLUTE =
#for (header, QMDMM_HEADERS): HEADERS_ABSOLUTE += $$system_path($$absolute_path($$header))

#generateHeaders.depends = $$HEADERS_ABSOLUTE

#QMAKE_EXTRA_TARGETS += generateHeaders
#PRE_TARGETDEPS += $$generateHeaders.target

#includetarget.path = /include/QMdmmCore/
#includetarget.files = $$OUT_PWD/../dist/include/QMdmmCore/*

#target.path = /lib/
#dlltarget.path = /bin/
#INSTALLS += target dlltarget includetarget

#QMAKE_CLEAN += $$system_path($$OUT_PWD/../dist/include/QMdmmCore/.timestamp)
