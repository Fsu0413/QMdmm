# SPDX-License-Identifier: AGPL-3.0-or-later

add_custom_target(QMdmmNetworking_Test)
if (BUILD_TESTING)
    set_target_properties(QMdmmNetworking_Test PROPERTIES
        EXCLUDE_FROM_ALL false
    )
endif()

function(add_qmdmmnetworking_test TEST_SOURCE)
    string(MAKE_C_IDENTIFIER "${TEST_SOURCE}" TEST_TARGET_EXECUTABLE)

    if (Qt6_FOUND)
        add_executable("tst_qmdmmnetworking_${TEST_TARGET_EXECUTABLE}6" EXCLUDE_FROM_ALL ${TEST_SOURCE} test.cpp test.h)
        add_dependencies(QMdmmNetworking_Test "tst_qmdmmnetworking_${TEST_TARGET_EXECUTABLE}6")
        target_link_libraries("tst_qmdmmnetworking_${TEST_TARGET_EXECUTABLE}6" PUBLIC QMdmm6::Networking)
        if (QMDMM_EXPORT_PRIVATE)
            target_link_libraries("tst_qmdmmnetworking_${TEST_TARGET_EXECUTABLE}6" PUBLIC QMdmm6::NetworkingPrivate)
        endif()
        if (BUILD_TESTING)
            target_link_libraries("tst_qmdmmnetworking_${TEST_TARGET_EXECUTABLE}6" PUBLIC Qt6::Test)
        endif()
        set_target_properties("tst_qmdmmnetworking_${TEST_TARGET_EXECUTABLE}6" PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)
        add_test(NAME "QMdmmNetworking - ${TEST_SOURCE} - Qt6" COMMAND "tst_qmdmmnetworking_${TEST_TARGET_EXECUTABLE}6")
    endif()
endfunction()

add_qmdmmnetworking_test(tst_qmdmmnetworking.cpp)
