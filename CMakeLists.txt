# SPDX-License-Identifier: AGPL-3.0-or-later

cmake_minimum_required(VERSION 3.19)

if (DEFINED PROJECT_NAME)
    message(FATAL_ERROR "Please build this project separately. add_subdirectory won't be supported.")
endif()

if (POLICY CMP0177)
    # no influence here
    cmake_policy(SET CMP0177 NEW)
endif()

project(QMdmm
    VERSION 0.0.1
    LANGUAGES CXX
    DESCRIPTION "A game which is played in primary / high school"
)

include(.FsEnv.cmake OPTIONAL)

option(BUILD_SHARED_LIBS "Build shared libraries" true)
if (BUILD_SHARED_LIBS)
    set(CMAKE_POSITION_INDEPENDENT_CODE true)
endif()

option(QMDMM_EXPORT_PRIVATE "Export private symbols, copy headers of private classes" false)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN true)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(AutoGenerateHeader)

# Note: Qt suggest users use versionless targets and functions, but if Qt 6 and Qt 5 are mixed in one CMakeLists.txt, it won't be an option and versioned ones should be used.
# There are definitions to disable the versionless targets and functions, but!! But!! BUT!!!!! They DIDN'T WORK in Qt 6.2 / 5.15 combination!!!
# set(QT_NO_CREATE_VERSIONLESS_FUNCTIONS 1)
# set(QT_NO_CREATE_VERSIONLESS_TARGETS 1)
# It works in later Qt 6 / 5.15 combinations, but be careful to check which version since they are supported (probably Qt 6.5)

# find_package Qt6 must be called before Qt5 - QTBUG-97097
find_package(Qt6 6.2  COMPONENTS Core Network WebSockets)
find_package(Qt5 5.15 COMPONENTS Core Network WebSockets)

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
        # match behavior of qt6_add_library
        set(CMAKE_IMPORT_LIBRARY_SUFFIX ".a")
    endif()
endif()

add_subdirectory(3rdparty)
add_subdirectory(QMdmmCore)
add_subdirectory(QMdmmNetworking)
add_subdirectory(QMdmmServer)
add_subdirectory(doc) # MAKE SURE this is the last one in the add_subdirectory list

set(CPACK_GENERATOR 7Z TGZ TXZ TZST ZIP)
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_SOURCE_GENERATOR 7Z TGZ TXZ TZST ZIP)
set(CPACK_SOURCE_IGNORE_FILES
    [[\\.git/]]
    [[\\.DS_Store/]]
    [[.*~$]]
    [[.*\\.autosave$]]
    [[build/]]
)
set(CPACK_RPM_PACKAGE_LICENSE "AGPLv3")

if (CMAKE_SYSTEM_NAME MATCHES "[Ll]inux")
    list(APPEND CPACK_GENERATOR RPM DEB)
endif()

include(CPack)
cpack_add_component("5")
cpack_add_component("6")
cpack_add_component("dev" DEPENDS "5" "6")
cpack_add_component("private-dev" DEPENDS "dev")
cpack_add_component("doc" DEPENDS "dev")
