# SPDX-License-Identifier: AGPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

project(QMdmm
    VERSION 0.0.1
    LANGUAGES CXX
)

include(.FsEnv.cmake OPTIONAL)

option(BUILD_SHARED_LIBS "Build shared libraries" true)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(AutoGenerateHeader)

add_subdirectory(3rdparty)
add_subdirectory(QMdmmCore)
add_subdirectory(QMdmmServer)
