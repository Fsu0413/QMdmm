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

option(BUILD_TESTING "Build test binaries" false)

option(QMDMM_EXPORT_PRIVATE "Export private symbols, copy headers of private classes" false)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED true)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN true)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(GNUInstallDirs)
include(AutoGenerateHeader)

set(QT6_EXTRA_COMPONENTS_NEEDED)

if (BUILD_TESTING)
    list(APPEND QT6_EXTRA_COMPONENTS_NEEDED Test)
endif()

# Note: Qt suggest users use versionless targets and functions, but if Qt 6 and Qt 5 are mixed in one CMakeLists.txt, it won't be an option and versioned ones should be used.
# There are definitions to disable the versionless targets and functions, but!! But!! BUT!!!!! They DIDN'T WORK in Qt 6.2 / 5.15 combination!!!
# Currently we are requiring Qt 6.5 and things work
set(QT_NO_CREATE_VERSIONLESS_FUNCTIONS 1)
set(QT_NO_CREATE_VERSIONLESS_TARGETS 1)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Network WebSockets Gui Qml Quick ${QT6_EXTRA_COMPONENTS_NEEDED})

if (Qt6_FOUND)
    qt6_standard_project_setup(REQUIRES 6.5)
    # we don't need them to be set by default
    unset(CMAKE_AUTOMOC)
    unset(CMAKE_AUTOUIC)
    unset(CMAKE_AUTORCC)
    if (QT_KNOWN_POLICY_QTP0003)
        # Consider the BUILD_SHARED_LIBS value when creating Qt libraries.
        qt6_policy(SET QTP0003 NEW)
    endif()
    if (QT_KNOWN_POLICY_QTP0004)
        # Extra directories with QML files in a QML module need extra qmldir files.
        # Migrate as long as possible!
        qt6_policy(SET QTP0004 OLD)
    endif()
endif()

# modify the default output location
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

include(CTest)

add_subdirectory(3rdparty)
add_subdirectory(QMdmmCore)
add_subdirectory(QMdmmNetworking)
add_subdirectory(QMdmmServer)
if (Qt6_FOUND)
    add_subdirectory(QMdmmGui)
endif()
add_subdirectory(doc) # MAKE SURE this is the last one in the add_subdirectory list

set(CPACK_GENERATOR 7Z TGZ TXZ TZST ZIP)
set(CPACK_DEP_COMPONENT_INSTALL ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_SOURCE_GENERATOR 7Z TGZ TXZ TZST ZIP)
set(CPACK_SOURCE_IGNORE_FILES
    [[\\.git/]]
    [[\\.DS_Store]]
    [[.*~$]]
    [[.*\\.autosave$]]
    [[build/]]
    [[/\\.FsEnv\\.cmake$]]
    [[CMakeLists\\.txt\\.user]]
)
set(CPACK_RPM_PACKAGE_LICENSE "AGPLv3")
set(CPACK_PACKAGE_CONTACT "Frank Su <fsu0413@vip.qq.com>")

if (CMAKE_SYSTEM_NAME MATCHES "[Ll]inux")
    # TODO: add DEB / RPM package dependencies
    list(APPEND CPACK_GENERATOR RPM DEB)
endif()

include(CPack)
cpack_add_component("6")
cpack_add_component("dev" DEPENDS "5" "6")
cpack_add_component("private-dev" DEPENDS "dev")
cpack_add_component("doc" DEPENDS "dev")
