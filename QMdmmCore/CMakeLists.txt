# SPDX-License-Identifier: AGPL-3.0-or-later

set(QMDMMCORE_HEADERS
    src/qmdmmcoreglobal.h

    src/qmdmmprotocol.h

    src/qmdmmroom.h
    src/qmdmmplayer.h

    src/qmdmmlogic.h
    src/qmdmmagent.h
    src/qmdmmstonescissorscloth.h
)

set(QMDMMCORE_SOURCES
    src/qmdmmcore.cpp
    src/qmdmmprotocol.cpp

    src/qmdmmroom.cpp
    src/qmdmmplayer.cpp

    src/qmdmmlogic.cpp
    src/qmdmmagent.cpp
    src/qmdmmstonescissorscloth.cpp
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

target_link_libraries(QMdmmCore
    PUBLIC QMdmm::JsonCpp
)

#includetarget.path = /include/QMdmmCore/
#includetarget.files = $$OUT_PWD/../dist/include/QMdmmCore/*

#target.path = /lib/
#dlltarget.path = /bin/
#INSTALLS += target dlltarget includetarget

#QMAKE_CLEAN += $$system_path($$OUT_PWD/../dist/include/QMdmmCore/.timestamp)
