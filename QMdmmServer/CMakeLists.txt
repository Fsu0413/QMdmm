# SPDX-License-Identifier: AGPL-3.0-or-later

set(QMDMMSERVER_SOURCES
    src/main.cpp
    src/config.cpp
    src/config.h
)

if (Qt6_FOUND)    
    if (ANDROID)
        # TODO: investigate the *CORRECT* method not using qt6_add_executable in Android platform
        # This was meant to be built as an executable run by QProcess
        # instead of a MODULE loaded from the Qt for Android entry point
        # Also, due to qt6_add_executable creates a MODULE, all Qt executable run inside Termux breaks build
        add_executable(QMdmmServer6 ${QMDMMSERVER_SOURCES})
    else()
        qt6_add_executable(QMdmmServer6 ${QMDMMSERVER_SOURCES})
    endif()

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
        DESTINATION "${CMAKE_INSTALL_BINDIR}"
        RUNTIME
        LIBRARY
    )

    if (ANDROID)
        # TODO: check if it is right
        qt6_finalize_target(QMdmmServer6)
    endif()
endif()
