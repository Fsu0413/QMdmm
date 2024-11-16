
find_package(Doxygen)

if (DOXYGEN_FOUND)
    set(QMDMM_DOC_SOURCE_FILES)

    foreach (i IN LISTS QMDMMCORE_DOC_FILES)
        list(APPEND QMDMM_DOC_SOURCE_FILES "../QMdmmCore/${i}")
    endforeach()

    foreach (i IN LISTS QMDMMNETWORKING_DOC_FILES)
        # uncomment when Core is ready
        # list(APPEND QMDMM_DOC_SOURCE_FILES "../QMdmmNetworking/${i}")
    endforeach()

    set(DOXYGEN_PROJECT_NAME "${PROJECT_NAME}")
    set(DOXYGEN_PROJECT_NUMBER "${PROJECT_VERSION}")
    set(DOXYGEN_PROJECT_BRIEF "${PROJECT_DESCRIPTION}")
    set(DOXYGEN_CREATE_SUBDIRS YES)
    set(DOXYGEN_CASE_SENSE_NAMES YES)
    set(DOXYGEN_FULL_PATH_NAMES NO)
    set(DOXYGEN_SOURCE_BROWSER NO)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_LATEX NO)
    set(DOXYGEN_GENERATE_RTF YES)
    set(DOXYGEN_GENERATE_MAN YES)
    set(DOXYGEN_PREDEFINED
        DOXYGEN
        Q_DOC
        Q_CLANG_DOC
        "_declspec(x)= "
        "__attribute__(x)= "
        "Q_ENUM(x)="
        "Q_ENUM_NS(x)="
    )
    set(DOXYGEN_SORT_MEMBER_DOCS NO)
    set(DOXYGEN_MACRO_EXPANSION YES)
    set(DOXYGEN_EXTRACT_PRIV_VIRTUAL YES)
    set(DOXYGEN_WARN_IF_UNDOC_ENUM_VAL YES)

    doxygen_add_docs(
        "${PROJECT_NAME}_docs"
        ${QMDMM_DOC_SOURCE_FILES}
        QMdmmCore/group.cpp
        ALL
        USE_STAMP_FILE
    )
endif()
