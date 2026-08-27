# SPDX-License-Identifier: AGPL-3.0-or-later
# GUI smoke test (Qt Quick Test). Exercises the QMdmmGameClient bridge (the
# C++/QML glue used by the "local game" mode) and the QML scenes' interaction
# wiring at runtime, where the GUI currently has only static verification.
#
# QMdmmGameClient lives in the QMdmmGui *executable*, not a linkable library, so
# its source is compiled straight into this test target.

add_custom_target(QMdmmGui_Test)
if (BUILD_TESTING)
    set_target_properties(QMdmmGui_Test PROPERTIES EXCLUDE_FROM_ALL false)
endif()

add_executable(tst_qmdmmgui6 EXCLUDE_FROM_ALL
    tst_qmdmmgui.cpp
    ../src/gameclient.h
    ../src/gameclient.cpp
)
add_dependencies(QMdmmGui_Test tst_qmdmmgui6)

target_link_libraries(tst_qmdmmgui6
    PUBLIC QMdmm6::Core
           QMdmm6::Networking
           Qt6::Qml
           Qt6::Quick
)
if (BUILD_TESTING)
    target_link_libraries(tst_qmdmmgui6 PUBLIC Qt6::QuickTest)
endif()

set_target_properties(tst_qmdmmgui6 PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# QMdmmGameClient is declared in src/, next to the target's own test entry point.
target_include_directories(tst_qmdmmgui6 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src")

# Qt Quick Test discovers the tst_*.qml cases in this directory.
target_compile_definitions(tst_qmdmmgui6 PRIVATE
    "QUICK_TEST_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\""
)

add_test(NAME "QMdmmGui - gui smoke - Qt6" COMMAND tst_qmdmmgui6)

# Headless runners need a renderer and QPA platform that do not depend on a
# GPU or a display. Linux has no display; macOS Intel runners stall in the
# cocoa platform on startup. The offscreen QPA platform + software renderer
# sidesteps both, so use it on every Unix runner.
if (UNIX)
    set_tests_properties("QMdmmGui - gui smoke - Qt6" PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software"
    )
endif()
