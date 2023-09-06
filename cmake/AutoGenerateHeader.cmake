# SPDX-License-Identifier: AGPL-3.0-or-later

function(find_classes_in_header_file FILE_NAME OUTPUT_VAR)
    set(OUTPUT)
    set(CLASS_REGEX "^(class|struct) +[A-Z]*_EXPORT +([A-Za-z0-9_]+)( .*)?")
    file(STRINGS "${FILE_NAME}" FILE_MATCHED_LINES REGEX "${CLASS_REGEX}")
    foreach (MATCHED IN LISTS FILE_MATCHED_LINES)
        string(REGEX MATCH "${CLASS_REGEX}" MATCHED_2 "${MATCHED}")
        if (NOT MATCHED_2)
            message(FATAL_ERROR "Unexpected error: ${MATCHED} ${MATCHED_2} ${CMAKE_MATCH_2} ${FILE_MATCHED_LINES}")
        endif()
        list(APPEND OUTPUT ${CMAKE_MATCH_2})
    endforeach()
    set("${OUTPUT_VAR}" "${OUTPUT}" PARENT_SCOPE)
endfunction()

# TODO: use DEPFILE
function(find_classes_in_header_files FILE_PATHS INCLUDE_PREFIX OUTPUT_VAR)
    set(OUTPUT)
    foreach (FILE_PATH IN LISTS FILE_PATHS)
        get_filename_component(FILE_NAME "${FILE_PATH}" NAME)
        find_classes_in_header_file("${FILE_PATH}" out)
        foreach (FILE_CLASS IN LISTS out)
            set(INCLUDE_FILE "${INCLUDE_PREFIX}/${FILE_CLASS}")
            list(APPEND OUTPUT "${INCLUDE_FILE}")
            add_custom_command(OUTPUT "${INCLUDE_FILE}"
                               COMMAND "${CMAKE_COMMAND}" -P "${AUTO_GENERATE_HEADER_CMAKE_FILE}" "${CMAKE_CURRENT_BINARY_DIR}/${INCLUDE_PREFIX}" "${FILE_NAME}" "${FILE_CLASS}"
                               DEPENDS "${FILE_PATH}" "${AUTO_GENERATE_HEADER_CMAKE_FILE}"
            )
        endforeach()
    endforeach()
    set("${OUTPUT_VAR}" "${OUTPUT}" PARENT_SCOPE)
endfunction()

function(generate_class_name_header BINARY_DIR ORIGINAL_FILE_NAME CLASS_NAME)
    file(WRITE "${BINARY_DIR}/${CLASS_NAME}"
         "#include \"${ORIGINAL_FILE_NAME}\"
"
    )
endfunction()

if (DEFINED PROJECT_VERSION)
    # include mode, only function call is used
    set(AUTO_GENERATE_HEADER_CMAKE_FILE "${CMAKE_CURRENT_LIST_FILE}")
else()
    # generate mode
    set(START_ARGN)
    foreach (I RANGE ${CMAKE_ARGC})
        if (CMAKE_ARGV${I} STREQUAL "-P")
            math(EXPR START_ARGN "${I} + 2")
            break()
        endif()
    endforeach()
    math(EXPR START_ARGN2 "${START_ARGN} + 1")
    math(EXPR START_ARGN3 "${START_ARGN2} + 1")
    generate_class_name_header("${CMAKE_ARGV${START_ARGN}}" "${CMAKE_ARGV${START_ARGN2}}" "${CMAKE_ARGV${START_ARGN3}}")
endif()
