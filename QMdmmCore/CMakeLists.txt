# SPDX-License-Identifier: AGPL-3.0-or-later

set(QMDMMCORE_PCH
    src/qmdmmcoreglobal.h
)

set(QMDMMCORE_HEADERS
    ${QMDMMCORE_PCH}
    src/qmdmmprotocol.h
    src/qmdmmroom.h
    src/qmdmmplayer.h
    src/qmdmmlogic.h
    src/qmdmmdebug.h
)

set(QMDMMCORE_PRIVATE_HEADERS
    src/qmdmmroom_p.h
    src/qmdmmplayer_p.h
    src/qmdmmlogic_p.h
    src/qmdmmdebug_p.h
)

set(QMDMMCORE_SOURCES
    src/qmdmmcore.cpp
    src/qmdmmprotocol.cpp
    src/qmdmmroom.cpp
    src/qmdmmplayer.cpp
    src/qmdmmlogic.cpp
    src/qmdmmdebug.cpp
)

set(QMDMMCORE_DOC_FILES ${QMDMMCORE_HEADERS} ${QMDMMCORE_SOURCES} PARENT_SCOPE)

add_library(QMdmmCore INTERFACE)
auto_generate_header_file_for_target(QMdmmCore "${QMDMMCORE_HEADERS}")
if (NOT BUILD_SHARED_LIBS)
    target_compile_definitions(QMdmmCore
        INTERFACE QMDMM_STATIC
    )
endif()
target_compile_definitions(QMdmmCore
    INTERFACE "QT_NO_CAST_FROM_ASCII"
              "QT_DISABLE_DEPRECATED_BEFORE=0x70000"
              "QMDMM_VERSION=\"${QMdmm_VERSION}\""
)
target_include_directories(QMdmmCore
    INTERFACE "${CMAKE_BINARY_DIR}/build/include"
)
get_target_property(QMDMMCORE_GENERATED_FILES QMdmmCore SOURCES)
if (NOT QMDMMCORE_GENERATED_FILES)
    unset(QMDMMCORE_GENERATED_FILES)
endif()

# https://doc.qt.io/qt-6/cmake-qt5-and-qt6-compatibility.html#unicode-support-in-windows
if (WIN32)
    target_compile_definitions(QMdmmCore
        INTERFACE UNICODE
                  _UNICODE
    )
endif()
if (QMDMM_EXPORT_PRIVATE)
    if (BUILD_SHARED_LIBS)
        target_compile_definitions(QMdmmCore
            INTERFACE QMDMM_NEED_EXPORT_PRIVATE
        )
    endif()
    add_library(QMdmmCorePrivate INTERFACE)
    auto_generate_header_file_for_target_private(QMdmmCore "${QMDMMCORE_PRIVATE_HEADERS}")
    target_include_directories(QMdmmCorePrivate
        INTERFACE "${CMAKE_BINARY_DIR}/build/include/QMdmmCore/private/${QMdmm_VERSION}"
    )
    target_link_libraries(QMdmmCorePrivate
        INTERFACE QMdmmCore
    )
endif()

if (MSVC)
    target_compile_options(QMdmmCore
        INTERFACE "-W4"
                  "-wd4251"
    )
else()
    target_compile_options(QMdmmCore
        INTERFACE "-Wall"
                  "-Wextra"
    )
endif()

if (Qt6_FOUND)
    qt6_add_library(QMdmmCore6 ${QMDMMCORE_HEADERS} ${QMDMMCORE_SOURCES} ${QMDMMCORE_PRIVATE_HEADERS})
    add_library(QMdmm6::Core ALIAS QMdmmCore6)

    set_target_properties(QMdmmCore6 PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        VERSION "${QMdmm_VERSION}"
    )

    target_precompile_headers(QMdmmCore6
        PRIVATE ${QMDMMCORE_PCH}
    )

    if (BUILD_SHARED_LIBS)
        target_compile_definitions(QMdmmCore6
            PRIVATE QMDMMCORE_LIBRARY
        )
    endif()

    target_include_directories(QMdmmCore6
        INTERFACE "${CMAKE_BINARY_DIR}/build/include/QMdmmCore"
        PRIVATE src
    )

    target_link_libraries(QMdmmCore6
        PUBLIC QMdmmCore
               Qt6::Core
    )

    install(TARGETS QMdmmCore6
        RUNTIME
        ARCHIVE
        LIBRARY
    )

    if (QMDMM_EXPORT_PRIVATE)
        add_library(QMdmmCorePrivate6 INTERFACE)
        add_library(QMdmm6::CorePrivate ALIAS QMdmmCorePrivate6)
        target_link_libraries(QMdmmCorePrivate6
            INTERFACE QMdmm6::Core
                      QMdmmCorePrivate
        )
    endif()
endif()

if (Qt5_FOUND)
    add_library(QMdmmCore5 ${QMDMMCORE_HEADERS} ${QMDMMCORE_SOURCES} ${QMDMMCORE_PRIVATE_HEADERS})
    add_library(QMdmm5::Core ALIAS QMdmmCore5)

    set_target_properties(QMdmmCore5 PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        VERSION "${QMdmm_VERSION}"
    )

    target_precompile_headers(QMdmmCore5
        PRIVATE ${QMDMMCORE_PCH}
    )

    if (BUILD_SHARED_LIBS)
        target_compile_definitions(QMdmmCore5
            PRIVATE QMDMMCORE_LIBRARY
        )
    endif()

    target_include_directories(QMdmmCore5
        INTERFACE "${CMAKE_BINARY_DIR}/build/include/QMdmmCore"
        PRIVATE src
    )

    target_link_libraries(QMdmmCore5
        PUBLIC QMdmmCore
               Qt5::Core
    )

    install(TARGETS QMdmmCore5
        RUNTIME
        ARCHIVE
        LIBRARY
    )

    if (QMDMM_EXPORT_PRIVATE)
        add_library(QMdmmCorePrivate5 INTERFACE)
        add_library(QMdmm5::CorePrivate ALIAS QMdmmCorePrivate5)
        target_link_libraries(QMdmmCorePrivate5
            INTERFACE QMdmm5::Core
                      QMdmmCorePrivate
        )
    endif()
endif()
