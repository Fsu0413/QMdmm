# SPDX-License-Identifier: AGPL-3.0-or-later
# QMdmmBot unit tests (QtTest).
#
# Bot lives in the QMdmmBot *executable*, not in a linkable library, so its
# sources are compiled straight into this test target -- the same approach the
# QMdmmGui smoke test takes for QMdmmGameClient. The style subclasses come along
# because Bot::createBot() references them.

add_custom_target(QMdmmBot_Test)
if (BUILD_TESTING)
    set_target_properties(QMdmmBot_Test PROPERTIES EXCLUDE_FROM_ALL false)
endif()

add_executable(tst_qmdmmbot6 EXCLUDE_FROM_ALL
    tst_qmdmmbot.cpp
    test.cpp
    test.h
    ../src/bot.h
    ../src/bot.cpp
    ../src/knifepreferredbot.cpp
    ../src/horsepreferredbot.cpp
    ../src/rlbot.cpp
)
add_dependencies(QMdmmBot_Test tst_qmdmmbot6)

target_link_libraries(tst_qmdmmbot6
    PUBLIC QMdmm6::Core
           QMdmm6::Networking
)
if (BUILD_TESTING)
    target_link_libraries(tst_qmdmmbot6 PUBLIC Qt6::Test)
endif()

set_target_properties(tst_qmdmmbot6 PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# bot.h is declared in src/, next to the sources it belongs to.
target_include_directories(tst_qmdmmbot6 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src")

add_test(NAME "QMdmmBot - bot - Qt6" COMMAND tst_qmdmmbot6)
