# SPDX-License-Identifier: AGPL-3.0-or-later

add_custom_target(QMdmmCore_Test)
if (BUILD_TESTING)
    set_target_properties(QMdmmCore_Test PROPERTIES
        EXCLUDE_FROM_ALL false
    )
endif()

function(add_qmdmmcore_test)
    if (Qt6_FOUND)
        add_executable("tst_qmdmmcore6" EXCLUDE_FROM_ALL ${ARGN} test.cpp test.h)
        add_dependencies(QMdmmCore_Test "tst_qmdmmcore6")
        target_link_libraries("tst_qmdmmcore6" PUBLIC QMdmmCore6)
        if (BUILD_TESTING)
            target_link_libraries("tst_qmdmmcore6" PUBLIC Qt6::Test)
        endif()
        set_target_properties("tst_qmdmmcore6" PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)
        add_test(NAME "QMdmmCore - Qt6" COMMAND "tst_qmdmmcore6")
    endif()
endfunction()

add_qmdmmcore_test(
    tst_qmdmmcore.cpp
    tst_qmdmmlogic.cpp
    tst_qmdmmplayer.cpp
    tst_qmdmmroom.cpp
    tst_qmdmmprotocol.cpp
    # tst_qmdmmdebug.cpp
)
