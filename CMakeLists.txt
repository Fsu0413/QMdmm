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

find_package(Qt6      COMPONENTS Core Network )
find_package(Qt5 5.15 COMPONENTS Core Network)

if (NOT (Qt5_FOUND OR Qt6_FOUND))
    message(FATAL_ERROR "at least one of Qt 5 and Qt 6 is needed")
endif()

if (Qt6_FOUND)
    if (Qt6_VERSION VERSION_GREATER_EQUAL 6.3.0)
        qt6_standard_project_setup()
        # we don't need them to be set by default
        unset(CMAKE_AUTOMOC)
        unset(CMAKE_AUTOUIC)
        unset(CMAKE_AUTORCC)
    endif()
    if (QT_KNOWN_POLICY_QTP0003)
        # Consider the BUILD_SHARED_LIBS value when creating Qt libraries.
        qt6_policy(SET QTP0003 NEW)
    endif()
endif()

# modify the default output location

include(GNUInstallDirs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/${CMAKE_INSTALL_LIBDIR}")

if (WIN32)
    set(CMAKE_SHARED_MODULE_PREFIX "")
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    if (NOT MSVC)
        # match behavoir of qt6_add_library
        set(CMAKE_IMPORT_LIBRARY_SUFFIX ".a")
    endif()
endif()

add_subdirectory(3rdparty)
add_subdirectory(QMdmmCore)
add_subdirectory(QMdmmServer)
