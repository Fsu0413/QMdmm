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

add_custom_target(QMdmmCore)
auto_generate_header_file_for_target(QMdmmCore "${QMDMMCORE_HEADERS}")
if (QMDMM_EXPORT_PRIVATE)
    auto_generate_header_file_for_target_private(QMdmmCore "${QMDMMCORE_PRIVATE_HEADERS}")
    add_library(QMdmmCorePrivate INTERFACE)
    target_include_directories(QMdmmCorePrivate
        INTERFACE "${CMAKE_BINARY_DIR}/build/include/QMdmmCore/private/${QMdmm_VERSION}"
    )
endif()

if (Qt6_FOUND)
    qt6_add_library(QMdmmCore6 ${QMDMMCORE_HEADERS} ${QMDMMCORE_SOURCES} ${QMDMMCORE_PRIVATE_HEADERS})
    add_library(QMdmm6::Core ALIAS QMdmmCore6)
    add_dependencies(QMdmmCore QMdmmCore6)

    set_target_properties(QMdmmCore6 PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        VERSION "${QMdmm_VERSION}"
    )

    target_precompile_headers(QMdmmCore6
        PRIVATE ${QMDMMCORE_PCH}
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
        if (QMDMM_EXPORT_PRIVATE)
            target_compile_definitions(QMdmmCore6
                PUBLIC QMDMM_NEED_EXPORT_PRIVATE
            )
        endif()
    else()
        target_compile_definitions(QMdmmCore6
            PUBLIC QMDMM_STATIC
        )
    endif()

    target_include_directories(QMdmmCore6
        PUBLIC "${CMAKE_BINARY_DIR}/build/include"
        INTERFACE "${CMAKE_BINARY_DIR}/build/include/QMdmmCore"
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
    add_dependencies(QMdmmCore QMdmmCore5)

    set_target_properties(QMdmmCore5 PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        VERSION "${QMdmm_VERSION}"
    )

    target_precompile_headers(QMdmmCore5
        PRIVATE ${QMDMMCORE_PCH}
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
        if (QMDMM_EXPORT_PRIVATE)
            target_compile_definitions(QMdmmCore5
                PUBLIC QMDMM_NEED_EXPORT_PRIVATE
            )
        endif()
    else()
        target_compile_definitions(QMdmmCore5
            PUBLIC QMDMM_STATIC
        )
    endif()

    target_include_directories(QMdmmCore5
        PUBLIC "${CMAKE_BINARY_DIR}/build/include"
        INTERFACE "${CMAKE_BINARY_DIR}/build/include/QMdmmCore"
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

    if (QMDMM_EXPORT_PRIVATE)
        add_library(QMdmmCorePrivate5 INTERFACE)
        add_library(QMdmm5::CorePrivate ALIAS QMdmmCorePrivate5)
        target_link_libraries(QMdmmCorePrivate5
            INTERFACE QMdmm5::Core
                      QMdmmCorePrivate
        )
    endif()
endif()
