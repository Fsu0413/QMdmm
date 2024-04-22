# SPDX-License-Identifier: AGPL-3.0-or-later

set(QMDMMCORE_HEADERS
    src/qmdmmcoreglobal.h

    src/qmdmmroom.h
    src/qmdmmplayer.h
    src/qmdmmcoreglobal.h
    src/qmdmmprotocol.h
    src/qmdmmstonescissorscloth.h
)

set(QMDMMCORE_SOURCES
    src/qmdmmcore.cpp
    src/qmdmmroom.cpp
    src/qmdmmplayer.cpp
    src/qmdmmstonescissorscloth.cpp
    src/qmdmmprotocol.cpp
)

add_library(QMdmmCore
    ${QMDMMCORE_HEADERS}
    ${QMDMMCORE_SOURCES}
)

target_precompile_headers(QMdmmCore
    PRIVATE src/qmdmmcoreglobal.h
)

auto_generate_header_file_for_target(QMdmmCore "${QMDMMCORE_HEADERS}")

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
    INTERFACE "${CMAKE_BINARY_DIR}/build/include"
    PRIVATE src
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

#includetarget.path = /include/QMdmmCore/
#includetarget.files = $$OUT_PWD/../dist/include/QMdmmCore/*

#target.path = /lib/
#dlltarget.path = /bin/
#INSTALLS += target dlltarget includetarget

#QMAKE_CLEAN += $$system_path($$OUT_PWD/../dist/include/QMdmmCore/.timestamp)
