# SPDX-License-Identifier: AGPL-3.0-or-later

# Networked local-game playability regression test.
#
# tst_qmdmm_smoke spins up an in-process server + two bot clients, plays a full
# game to GAME OVER, and asserts the networked loop actually completes. It is
# always built (so the binary is always available) and, when BUILD_TESTING is
# enabled, registered as a CTest test so `ctest` can verify the game stays
# playable after future changes.

add_custom_target(QMdmm_SmokeTest)
if (BUILD_TESTING)
    set_target_properties(QMdmm_SmokeTest PROPERTIES
        EXCLUDE_FROM_ALL false
    )
endif()

if (Qt6_FOUND)
    add_executable(tst_qmdmm_smoke6 EXCLUDE_FROM_ALL
        main.cpp
    )
    add_dependencies("QMdmm_SmokeTest" tst_qmdmm_smoke6)
    target_link_libraries(tst_qmdmm_smoke6 PRIVATE QMdmmCore6 QMdmmNetworking6)

    # Register as a CTest test (consistent with the other tests in this project,
    # which are gated behind BUILD_TESTING).
    #
    # Two cases cover the playerNumPerRoom boundary: the default (2, the minimum
    # valid room) and the high end (8 players) so a room-size regression at either
    # end of the supported 2..9 range is caught.
    if (BUILD_TESTING)
        add_test(NAME tst_qmdmm_smoke6 COMMAND $<TARGET_FILE:tst_qmdmm_smoke6>)
        add_test(NAME tst_qmdmm_smoke6_8players COMMAND $<TARGET_FILE:tst_qmdmm_smoke6> 8)
    endif()
endif()
