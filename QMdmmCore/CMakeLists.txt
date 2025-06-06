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
    src/qmdmmsettings.h
)

set(QMDMMCORE_PRIVATE_HEADERS
    src/qmdmmroom_p.h
    src/qmdmmplayer_p.h
    src/qmdmmlogic_p.h
    src/qmdmmdebug_p.h
    src/qmdmmsettings_p.h
)

set(QMDMMCORE_SOURCES
    src/qmdmmcore.cpp
    src/qmdmmprotocol.cpp
    src/qmdmmroom.cpp
    src/qmdmmplayer.cpp
    src/qmdmmlogic.cpp
    src/qmdmmdebug.cpp
    src/qmdmmsettings.cpp
)

set(QMDMMCORE_PRIVATE_SOURCES
    src/qmdmmdebug_p.cpp
    src/qmdmmlogic_p.cpp
    src/qmdmmplayer_p.cpp
    src/qmdmmsettings_p.cpp
)

set(QMDMMCORE_DOC_FILES ${QMDMMCORE_HEADERS} ${QMDMMCORE_SOURCES} PARENT_SCOPE)

if (UNIX AND ( ( CMAKE_INSTALL_PREFIX STREQUAL "/usr" ) OR ( CMAKE_INSTALL_PREFIX STREQUAL "/" ) ) )
    set(QMDMM_CONFIGURATION_PREFIX "/etc/${PROJECT_NAME}")
    set(QMDMM_RUNTIME_DATA_PREFIX "/var/${PROJECT_NAME}")
else()
    set(QMDMM_CONFIGURATION_PREFIX "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}")
    set(QMDMM_RUNTIME_DATA_PREFIX "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LOCALSTATEDIR}/${PROJECT_NAME}")
endif()

add_library(QMdmmCore INTERFACE)
auto_generate_header_file_for_target(QMdmmCore "${QMDMMCORE_HEADERS}")
install(FILES ${QMDMMCORE_HEADERS}
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/QMdmmCore"
    COMPONENT "dev"
)
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
              src
)
if (NOT MSVC)
    target_compile_options(QMdmmCore
        INTERFACE "$<$<CONFIG:Debug>:--coverage>"
    )
    target_link_options(QMdmmCore
        INTERFACE "$<$<CONFIG:Debug>:--coverage>"
    )
else()
    if ( ( Debug IN LISTS CMAKE_CONFIGURATION_TYPES ) OR ( CMAKE_BUILD_TYPE STREQUAL Debug ) )
        message(AUTHOR_WARNING "Coverage generation is not supported in MSVC")
    endif()
endif()
if (NOT MSVC)
    target_compile_options(QMdmmCore
        INTERFACE -Wall -Wextra
    )
else()
    target_compile_options(QMdmmCore
        INTERFACE -W4 -wd4251
    )
endif()
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
target_compile_definitions(QMdmmCore
    INTERFACE QMDMM_CONFIGURATION_PREFIX="${QMDMM_CONFIGURATION_PREFIX}"
              QMDMM_RUNTIME_DATA_PREFIX="${QMDMM_RUNTIME_DATA_PREFIX}"
)
if (QMDMM_EXPORT_PRIVATE)
    if (BUILD_SHARED_LIBS)
        target_compile_definitions(QMdmmCore
            INTERFACE QMDMM_NEED_EXPORT_PRIVATE
        )
    endif()
    add_library(QMdmmCorePrivate INTERFACE)
    target_sources(QMdmmCorePrivate PRIVATE ${QMDMMCORE_PRIVATE_HEADERS})
    install(FILES ${QMDMMCORE_PRIVATE_HEADERS}
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/QMdmmCore/private/${PROJECT_VERSION}"
        COMPONENT "private-dev"
    )
    target_link_libraries(QMdmmCorePrivate
        INTERFACE QMdmmCore
    )
endif()

if (MSVC)
    target_compile_options(QMdmmCore
        INTERFACE "-W4"
                  "-wd4251"
                  "-we4596"
    )
else()
    target_compile_options(QMdmmCore
        INTERFACE "-Wall"
                  "-Wextra"
    )
endif()

if (Qt6_FOUND)
    qt6_add_library(QMdmmCore6 ${QMDMMCORE_HEADERS} ${QMDMMCORE_SOURCES} ${QMDMMCORE_PRIVATE_HEADERS} ${QMDMMCORE_PRIVATE_SOURCES})
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
    )

    target_link_libraries(QMdmmCore6
        PUBLIC QMdmmCore
               Qt6::Core
    )

    install(TARGETS QMdmmCore6
        COMPONENT "6"
        RUNTIME
        LIBRARY
    )

    install(TARGETS QMdmmCore6
        COMPONENT "dev"
        ARCHIVE
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

add_subdirectory(test)
