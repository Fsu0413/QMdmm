# SPDX-License-Identifier: AGPL-3.0-or-later
# Networked local-game playability regression test.
#
# qmdmm_smoke spins up an in-process server + two bot clients, plays a full
# game to GAME OVER, and asserts the networked loop actually completes. It is
# always built (so the binary is always available) and, when BUILD_TESTING is
# enabled, registered as a CTest test so `ctest` can verify the game stays
# playable after future changes.
cmake_minimum_required(VERSION 3.19)

add_executable(qmdmm_smoke main.cpp)

target_link_libraries(qmdmm_smoke PRIVATE QMdmmCore6 QMdmmNetworking6)
target_compile_features(qmdmm_smoke PRIVATE cxx_std_20)

# Make sure the executable can find the in-tree dylibs at runtime.
set_target_properties(qmdmm_smoke PROPERTIES
    BUILD_RPATH_USE_ORIGIN true
    BUILD_RPATH "${CMAKE_CURRENT_BINARY_DIR}/../build/lib"
)

# Register as a CTest test (consistent with the other tests in this project,
# which are gated behind BUILD_TESTING).
#
# Two cases cover the playerNumPerRoom boundary: the default (2, the minimum
# valid room) and the high end (8 players) so a room-size regression at either
# end of the supported 2..9 range is caught.
if (BUILD_TESTING)
    add_test(NAME qmdmm_smoke COMMAND $<TARGET_FILE:qmdmm_smoke>)
    add_test(NAME qmdmm_smoke_8players COMMAND $<TARGET_FILE:qmdmm_smoke> 8)
endif()
