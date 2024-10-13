# SPDX-License-Identifier: AGPL-3.0-or-later

set(QMDMMSERVER_SOURCES
    src/main.cpp
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
        RUNTIME
        ARCHIVE
        LIBRARY
    )
endif()

if (Qt5_FOUND)
    add_executable(QMdmmServer5 ${QMDMMSERVER_SOURCES})

    set_target_properties(QMdmmServer5 PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        VERSION "${QMdmm_VERSION}"
    )

    target_link_libraries(QMdmmServer5
        PUBLIC QMdmm5::Core
               QMdmm5::Networking
    )

    install(TARGETS QMdmmServer5
        RUNTIME
        ARCHIVE
        LIBRARY
    )
endif()
