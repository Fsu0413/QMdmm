# SPDX-License-Identifier: AGPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

project(QMdmm
    VERSION 0.0.1
    LANGUAGES CXX
)

include(.FsEnv.cmake OPTIONAL)

option(BUILD_SHARED_LIBS "Build shared libraries" true)
if (BUILD_SHARED_LIBS)
    set(CMAKE_POSITION_INDEPENDENT_CODE true)
endif()

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(AutoGenerateHeader)

find_package(Qt6 COMPONENTS Core)
find_package(Qt5 COMPONENTS Core)

if (NOT (Qt5_FOUND OR Qt6_FOUND))
    message(FATAL_ERROR "at least one of Qt 5 and Qt 6 is needed")
endif()

add_subdirectory(3rdparty)
add_subdirectory(QMdmmCore)
