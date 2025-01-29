# SPDX-License-Identifier: AGPL-3.0-or-later

set(QMDMMNETWORKING_PCH
    src/qmdmmnetworkingglobal.h
)

set(QMDMMNETWORKING_HEADERS
    ${QMDMMNETWORKING_PCH}
    src/qmdmmserver.h
    src/qmdmmagent.h
    src/qmdmmclient.h
    src/qmdmmlogicrunner.h
    src/qmdmmsocket.h
)

set(QMDMMNETWORKING_PRIVATE_HEADERS
    src/qmdmmserver_p.h
    src/qmdmmagent_p.h
    src/qmdmmclient_p.h
    src/qmdmmlogicrunner_p.h
    src/qmdmmsocket_p.h
)

set(QMDMMNETWORKING_SOURCES
    src/qmdmmserver.cpp
    src/qmdmmagent.cpp
    src/qmdmmclient.cpp
    src/qmdmmlogicrunner.cpp
    src/qmdmmsocket.cpp
)

set(QMDMMNETWORKING_PRIVATE_SOURCES
    src/qmdmmclient_p.cpp
)

set(QMDMMNETWORKING_DOC_FILES ${QMDMMNETWORKING_HEADERS} ${QMDMMNETWORKING_SOURCES} PARENT_SCOPE)

add_library(QMdmmNetworking INTERFACE)
auto_generate_header_file_for_target(QMdmmNetworking "${QMDMMNETWORKING_HEADERS}")
install(FILES ${QMDMMNETWORKING_HEADERS}
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/QMdmmNetworking"
    COMPONENT "dev"
)
target_include_directories(QMdmmNetworking
    INTERFACE src
)
if (QMDMM_EXPORT_PRIVATE)
    add_library(QMdmmNetworkingPrivate INTERFACE)
    target_sources(QMdmmNetworkingPrivate PRIVATE ${QMDMMNETWORKING_PRIVATE_HEADERS})
    install(FILES ${QMDMMNETWORKING_PRIVATE_HEADERS}
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/QMdmmNetworking/private/${PROJECT_VERSION}"
        COMPONENT "private-dev"
    )
    target_link_libraries(QMdmmNetworkingPrivate
        INTERFACE QMdmmNetworking
    )
endif()

if (Qt6_FOUND)
    qt6_add_library(QMdmmNetworking6 ${QMDMMNETWORKING_HEADERS} ${QMDMMNETWORKING_SOURCES} ${QMDMMNETWORKING_PRIVATE_HEADERS} ${QMDMMNETWORKING_PRIVATE_SOURCES})
    add_library(QMdmm6::Networking ALIAS QMdmmNetworking6)

    set_target_properties(QMdmmNetworking6 PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        VERSION "${QMdmm_VERSION}"
    )

    target_precompile_headers(QMdmmNetworking6
        PRIVATE ${QMDMMNETWORKING_PCH}
    )

    if (BUILD_SHARED_LIBS)
        target_compile_definitions(QMdmmNetworking6
            PRIVATE QMDMMNETWORKING_LIBRARY
        )
    endif()

    target_include_directories(QMdmmNetworking6
        INTERFACE "${CMAKE_BINARY_DIR}/build/include/QMdmmNetworking"
    )

    target_link_libraries(QMdmmNetworking6
        PUBLIC QMdmm6::Core
               QMdmmNetworking
               Qt6::Network
               Qt6::WebSockets
    )

    install(TARGETS QMdmmNetworking6
        COMPONENT "6"
        RUNTIME
        LIBRARY
    )

    install(TARGETS QMdmmNetworking6
        COMPONENT "dev"
        ARCHIVE
    )

    if (QMDMM_EXPORT_PRIVATE)
        add_library(QMdmmNetworkingPrivate6 INTERFACE)
        add_library(QMdmm6::NetworkingPrivate ALIAS QMdmmNetworkingPrivate6)
        target_link_libraries(QMdmmNetworkingPrivate6
            INTERFACE QMdmm6::Networking
                      QMdmmNetworkingPrivate
        )
    endif()
endif()

if (Qt5_FOUND)
    add_library(QMdmmNetworking5 ${QMDMMNETWORKING_HEADERS} ${QMDMMNETWORKING_SOURCES} ${QMDMMNETWORKING_PRIVATE_HEADERS} ${QMDMMNETWORKING_PRIVATE_SOURCES})
    add_library(QMdmm5::Networking ALIAS QMdmmNetworking5)

    set_target_properties(QMdmmNetworking5 PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        VERSION "${QMdmm_VERSION}"
    )

    target_precompile_headers(QMdmmNetworking5
        PRIVATE ${QMDMMNETWORKING_PCH}
    )

    if (BUILD_SHARED_LIBS)
        target_compile_definitions(QMdmmNetworking5
            PRIVATE QMDMMNETWORKING_LIBRARY
        )
    endif()

    target_include_directories(QMdmmNetworking5
        INTERFACE "${CMAKE_BINARY_DIR}/build/include/QMdmmNetworking"
    )

    target_link_libraries(QMdmmNetworking5
        PUBLIC QMdmm5::Core
               QMdmmNetworking
               Qt5::Network
               Qt5::WebSockets
    )

    install(TARGETS QMdmmNetworking5
        COMPONENT "5"
        RUNTIME
        LIBRARY
    )

    install(TARGETS QMdmmNetworking5
        COMPONENT "dev"
        ARCHIVE
    )

    if (QMDMM_EXPORT_PRIVATE)
        add_library(QMdmmNetworkingPrivate5 INTERFACE)
        add_library(QMdmm5::NetworkingPrivate ALIAS QMdmmNetworkingPrivate5)
        target_link_libraries(QMdmmNetworkingPrivate5
            INTERFACE QMdmm5::Networking
                      QMdmmNetworkingPrivate
        )
    endif()
endif()
