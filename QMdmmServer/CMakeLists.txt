# SPDX-License-Identifier: AGPL-3.0-or-later

set(QMDMMSERVER_SOURCES
    src/main.cpp
    src/config.cpp
    src/config.h
)

if (Qt6_FOUND)
    qt6_add_executable(QMdmmServer6 ${QMDMMSERVER_SOURCES})

    set_target_properties(QMdmmServer6 PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        VERSION "${QMdmm_VERSION}"
    )

    target_link_libraries(QMdmmServer6
        PUBLIC QMdmm6::Core
               QMdmm6::Networking
    )

    install(TARGETS QMdmmServer6
        COMPONENT "6"
        RUNTIME
        LIBRARY
    )
endif()
