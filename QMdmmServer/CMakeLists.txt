# SPDX-License-Identifier: AGPL-3.0-or-later

set(QMDMMSERVER_PCH
    src/qmdmmserverglobal.h
)

set(QMDMMSERVER_HEADERS
    ${QMDMMSERVER_PCH}
    src/qmdmmserver.h
    src/qmdmmagent.h
    src/qmdmmclient.h
    src/qmdmmlogicrunner.h
    src/qmdmmsocket.h
)

set(QMDMMSERVER_PRIVATE_HEADERS
    src/qmdmmserver_p.h
    src/qmdmmagent_p.h
    src/qmdmmclient_p.h
    src/qmdmmlogicrunner_p.h
    src/qmdmmsocket_p.h
)

set(QMDMMSERVER_SOURCES
    src/qmdmmserver.cpp
    src/qmdmmagent.cpp
    src/qmdmmclient.cpp
    src/qmdmmlogicrunner.cpp
    src/qmdmmsocket.cpp
)

add_custom_target(QMdmmServer)
auto_generate_header_file_for_target(QMdmmServer "${QMDMMSERVER_HEADERS}")
auto_generate_header_file_for_target_private(QMdmmServer "${QMDMMSERVER_PRIVATE_HEADERS}")
add_library(QMdmmServerPrivate INTERFACE)
target_include_directories(QMdmmServerPrivate
    INTERFACE "${CMAKE_BINARY_DIR}/build/include/QMdmmServer/private/${QMdmm_VERSION}"
)

if (Qt6_FOUND)
    qt6_add_library(QMdmmServer6 ${QMDMMSERVER_HEADERS} ${QMDMMSERVER_SOURCES} ${QMDMMSERVER_PRIVATE_HEADERS})
    add_dependencies(QMdmmServer QMdmmServer6)

    set_target_properties(QMdmmServer6 PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        VERSION "${QMdmm_VERSION}"
    )

    target_precompile_headers(QMdmmServer6
        PRIVATE ${QMDMMSERVER_PCH}
    )

    target_compile_definitions(QMdmmServer6
        PUBLIC QT_NO_CAST_FROM_ASCII
    )

    if (BUILD_SHARED_LIBS)
        target_compile_definitions(QMdmmServer6
            PRIVATE QMDMMSERVER_LIBRARY
        )
    else()
        target_compile_definitions(QMdmmServer6
            PUBLIC QMDMM_STATIC
        )
    endif()

    target_include_directories(QMdmmServer6
        INTERFACE "${CMAKE_BINARY_DIR}/build/include/QMdmmServer"
        PRIVATE src
    )

    target_link_libraries(QMdmmServer6
        PUBLIC QMdmmCore6
               Qt6::Network
               Qt6::WebSockets
    )

    install(TARGETS QMdmmServer6
        RUNTIME
        ARCHIVE
        LIBRARY
    )

    add_library(QMdmmServerPrivate6 INTERFACE)
    target_link_libraries(QMdmmServerPrivate6
        INTERFACE QMdmmServer6
                  QMdmmServerPrivate
    )
endif()

if (Qt5_FOUND)
    add_library(QMdmmServer5 ${QMDMMSERVER_HEADERS} ${QMDMMSERVER_SOURCES} ${QMDMMSERVER_PRIVATE_HEADERS})
    add_dependencies(QMdmmServer QMdmmServer5)

    set_target_properties(QMdmmServer5 PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        VERSION "${QMdmm_VERSION}"
    )

    target_precompile_headers(QMdmmServer5
        PRIVATE ${QMDMMSERVER_PCH}
    )

    target_compile_definitions(QMdmmServer5
        PUBLIC QT_NO_CAST_FROM_ASCII
    )

    if (BUILD_SHARED_LIBS)
        target_compile_definitions(QMdmmServer5
            PRIVATE QMDMMSERVER_LIBRARY
        )
    else()
        target_compile_definitions(QMdmmServer5
            PUBLIC QMDMM_STATIC
        )
    endif()

    target_include_directories(QMdmmServer5
        INTERFACE "${CMAKE_BINARY_DIR}/build/include/QMdmmServer"
        PRIVATE src
    )

    target_link_libraries(QMdmmServer5
        PUBLIC QMdmmCore5
               Qt5::Network
               Qt5::WebSockets
    )

    install(TARGETS QMdmmServer5
        RUNTIME
        ARCHIVE
        LIBRARY
    )

    add_library(QMdmmServerPrivate5 INTERFACE)
    target_link_libraries(QMdmmServerPrivate5
        INTERFACE QMdmmServer5
                  QMdmmServerPrivate
    )
endif()
