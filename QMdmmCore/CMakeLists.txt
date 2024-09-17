# SPDX-License-Identifier: AGPL-3.0-or-later

set(QMDMMCORE_HEADERS
    src/qmdmmcoreglobal.h

    src/qmdmmprotocol.h

    src/qmdmmroom.h
    src/qmdmmplayer.h
    src/qmdmmlogic.h
)

set(QMDMMCORE_SOURCES
    src/qmdmmcore.cpp
    src/qmdmmprotocol.cpp

    src/qmdmmroom.cpp
    src/qmdmmplayer.cpp
    src/qmdmmlogic.cpp
)

add_custom_target(QMdmmCore)
auto_generate_header_file_for_target(QMdmmCore "${QMDMMCORE_HEADERS}")

if (Qt6_FOUND)
    qt6_add_library(QMdmmCore6 ${QMDMMCORE_HEADERS} ${QMDMMCORE_SOURCES})
    add_dependencies(QMdmmCore QMdmmCore6)

    set_target_properties(QMdmmCore6 PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        VERSION "${QMdmm_VERSION}"
    )

    target_compile_definitions(QMdmmCore6
        PUBLIC "QT_NO_CAST_FROM_ASCII"
               "QT_DISABLE_DEPRECATED_BEFORE=0x70000"
               "QMDMM_VERSION=\"${QMdmm_VERSION}\""
    )

    if (BUILD_SHARED_LIBS)
        target_compile_definitions(QMdmmCore6
            PRIVATE QMDMMCORE_LIBRARY
        )
    else()
        target_compile_definitions(QMdmmCore6
            PUBLIC QMDMM_STATIC
        )
    endif()

    target_include_directories(QMdmmCore6
        INTERFACE "${CMAKE_BINARY_DIR}/build/include"
        PRIVATE src
    )

    target_link_libraries(QMdmmCore6
        PUBLIC Qt6::Core
    )

    install(TARGETS QMdmmCore6
        RUNTIME
        ARCHIVE
        LIBRARY
    )
endif()

if (Qt5_FOUND)
    add_library(QMdmmCore5 ${QMDMMCORE_HEADERS} ${QMDMMCORE_SOURCES})
    add_dependencies(QMdmmCore QMdmmCore5)

    set_target_properties(QMdmmCore5 PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        VERSION "${QMdmm_VERSION}"
    )

    target_compile_definitions(QMdmmCore5
        PUBLIC "QT_NO_CAST_FROM_ASCII"
               "QT_DISABLE_DEPRECATED_BEFORE=0x70000"
               "QMDMM_VERSION=\"${QMdmm_VERSION}\""
    )

    if (BUILD_SHARED_LIBS)
        target_compile_definitions(QMdmmCore5
            PRIVATE QMDMMCORE_LIBRARY
        )
    else()
        target_compile_definitions(QMdmmCore5
            PUBLIC QMDMM_STATIC
        )
    endif()

    target_include_directories(QMdmmCore5
        INTERFACE "${CMAKE_BINARY_DIR}/build/include"
        PRIVATE src
    )

    target_link_libraries(QMdmmCore5
        PUBLIC Qt5::Core
    )

    install(TARGETS QMdmmCore5
        RUNTIME
        ARCHIVE
        LIBRARY
    )
endif()
