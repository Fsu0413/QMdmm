
find_package(jsoncpp QUIET GLOBAL)

add_library(QMdmmInternalJsonCppLibrary EXCLUDE_FROM_ALL
    upstream/src/lib_json/json_reader.cpp
    upstream/src/lib_json/json_tool.h
    upstream/src/lib_json/json_value.cpp
    upstream/src/lib_json/json_valueiterator.inl
    upstream/src/lib_json/json_writer.cpp
    upstream/include/json/allocator.h
    upstream/include/json/assertions.h
    upstream/include/json/config.h
    upstream/include/json/forwards.h
    upstream/include/json/json.h
    upstream/include/json/json_features.h
    upstream/include/json/reader.h
    upstream/include/json/value.h
    upstream/include/json/version.h
    upstream/include/json/writer.h
)
target_include_directories(QMdmmInternalJsonCppLibrary
    PUBLIC upstream/include
)
if (BUILD_SHARED_LIBS)
    target_compile_definitions(QMdmmInternalJsonCppLibrary
        PRIVATE JSON_DLL_BUILD
    )
endif()

if (jsoncpp_FOUND)
    add_library(QMdmm::JsonCpp ALIAS jsoncpp_lib)
else()
    add_library(QMdmm::JsonCpp ALIAS QMdmmInternalJsonCppLibrary)
endif()
