# SPDX-License-Identifier: AGPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

project(QMdmm
    VERSION 0.5.0
    LANGUAGES CXX
)

option(BUILD_SHARED_LIBS "Build shared libraries" true)

set(CMAKE_CXX_STANDARD 11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(AutoGenerateHeader)

if (NOT WIN32)
    find_package(PkgConfig QUIET)
    if (PKG_CONFIG_FOUND)
        pkg_search_module(JSONCPP QUIET IMPORTED_TARGET GLOBAL jsoncpp)
    endif()
endif()

add_subdirectory(QMdmmCore)
add_subdirectory(QMdmmServer)
