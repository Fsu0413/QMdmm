# SPDX-License-Identifier: AGPL-3.0-or-later
#
# QMdmm 6 - helper functions (hand-written).
#
# Installed verbatim and include()d by QMdmm6Config.cmake before the imported
# targets are created, so the functions below are available to consumers that
# want to build their own convenience API on top of the package.

# qmdmm6_add_versionless_targets()
#
# Creates the generation-free convenience aliases:
#
#   QMdmm::Core        ->  QMdmm6::Core
#   QMdmm::Networking  ->  QMdmm6::Networking
#
# Mirrors Qt, where Qt::Core is an alias for whichever Qt6::Core / Qt7::Core was
# found first. Two QMdmm generations cannot both own the alias, so the first one
# found wins and the second one says so on the status line - deliberately not a
# FATAL_ERROR, because failing here would break a perfectly innocent
# find_package() call of the second generation.
function(qmdmm6_add_versionless_targets)
    foreach (_qmdmm6_component IN LISTS QMdmm6_KNOWN_COMPONENTS)
        set(_qmdmm6_versioned "${QMdmm6_COMPONENT_TARGET_${_qmdmm6_component}}")
        set(_qmdmm6_versionless "QMdmm::${_qmdmm6_component}")

        if (NOT TARGET "${_qmdmm6_versioned}")
            continue()
        endif()

        if (TARGET "${_qmdmm6_versionless}")
            get_target_property(_qmdmm6_claimed "${_qmdmm6_versionless}" QMDMM_VERSIONLESS_FOR)
            if (NOT "${_qmdmm6_claimed}" STREQUAL "${_qmdmm6_versioned}")
                message(STATUS
                    "QMdmm: ${_qmdmm6_versionless} is already provided by "
                    "${_qmdmm6_claimed}; keeping it and skipping QMdmm 6")
            endif()
            continue()
        endif()

        # An INTERFACE IMPORTED target rather than add_library(ALIAS): targets
        # from install(EXPORT) are directory-scoped (not GLOBAL), and CMake only
        # allows an ALIAS of a GLOBAL imported target.
        add_library("${_qmdmm6_versionless}" INTERFACE IMPORTED)
        set_target_properties("${_qmdmm6_versionless}" PROPERTIES
            INTERFACE_LINK_LIBRARIES "${_qmdmm6_versioned}"
            QMDMM_VERSIONLESS_FOR "${_qmdmm6_versioned}"
        )
    endforeach()
endfunction()
