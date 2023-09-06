
add_library(QMdmmCore
    qmdmmcoreglobal.h

    qmdmmroom.h
    qmdmmplayer.h
    qmdmmcoreglobal.h
    qmdmmprotocol.h
    qmdmmstonescissorscloth.h
    qmdmmcore.cpp
    qmdmmroom.cpp
    qmdmmplayer.cpp
    qmdmmstonescissorscloth.cpp
    qmdmmprotocol.cpp
)

target_precompile_headers(QMdmmCore
    PRIVATE qmdmmcoreglobal.h
)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(QMdmmCore
        PRIVATE QMDMMCORE_LIBRARY
    )
else()
    target_compile_definitions(QMdmmCore
        PUBLIC QMDMM_STATIC
    )
endif()

if (ANDROID OR IOS)
    target_compile_definitions(QMdmmCore
        PUBLIC MOBILE_DEVICES
    )
endif()

if (JSONCPP_FOUND)
    target_link_libraries(QMdmmCore
        PUBLIC PkgConfig::JSONCPP
    )
else()
    add_library(QMdmmInternalJsonCppLibrary
        3rdparty/jsoncpp/json_reader.cpp
        3rdparty/jsoncpp/json_value.cpp
        3rdparty/jsoncpp/json_writer.cpp
        3rdparty/jsoncpp/json/config.h
        3rdparty/jsoncpp/json/features.h
        3rdparty/jsoncpp/json/forwards.h
        3rdparty/jsoncpp/json/json.h
        3rdparty/jsoncpp/json/reader.h
        3rdparty/jsoncpp/json/value.h
        3rdparty/jsoncpp/json/version.h
        3rdparty/jsoncpp/json/writer.h
        3rdparty/jsoncpp/json/allocator.h
        3rdparty/jsoncpp/json/assertions.h
        3rdparty/jsoncpp/json/autolink.h
        3rdparty/jsoncpp/json_tool.h
    )
    target_include_directories(QMdmmInternalJsonCppLibrary
        PUBLIC 3rdparty/jsoncpp
    )
    if (BUILD_SHARED_LIBS)
        target_compile_definitions(QMdmmInternalJsonCppLibrary
            PRIVATE JSON_DLL_BUILD
        )
    endif()
    target_link_libraries(QMdmmCore
        PUBLIC QMdmmInternalJsonCppLibrary
    )

#    generateHeadersJson.target = $$system_path($$OUT_PWD/../dist/include/json/.timestamp)
#    !build_pass: mkpath($$OUT_PWD/../dist/include/json)

#    contains(QMAKE_HOST.os, "Windows"): generateHeadersJson.commands = cscript $$system_path($$PWD/../../../tools/AutoGenerateHeader.vbs) -o $$system_path($$OUT_PWD/../dist/include/json) -f $$system_path($$PWD/json/)
#    else: generateHeadersJson.commands = $$PWD/../../../tools/AutoGenerateHeader.sh -o $$OUT_PWD/../dist/include/json -f $$PWD/json/

#    JSON_HEADERS_ABSOLUTE =
#    for (header, JSON_HEADERS): JSON_HEADERS_ABSOLUTE += $$system_path($$absolute_path($$header))

#    generateHeadersJson.depends = $$JSON_HEADERS_ABSOLUTE

#    QMAKE_EXTRA_TARGETS += generateHeadersJson
#    PRE_TARGETDEPS += $$generateHeadersJson.target

#    DEFINES += JSON_DLL_BUILD

#    win32-msvc*: QMAKE_CXXFLAGS += -wd4275

#    QMAKE_CLEAN += $$system_path($$OUT_PWD/../dist/include/json/.timestamp)
endif()

#generateHeaders.target = $$system_path($$OUT_PWD/../dist/include/QMdmmCore/.timestamp)
#!build_pass: mkpath($$OUT_PWD/../dist/include/QMdmmCore)

#contains(QMAKE_HOST.os, "Windows"): generateHeaders.commands = cscript $$system_path($$PWD/../tools/AutoGenerateHeader.vbs) -o $$system_path($$OUT_PWD/../dist/include/QMdmmCore) -f $$system_path($$PWD/)
#else: generateHeaders.commands = $$PWD/../tools/AutoGenerateHeader.sh -o $$OUT_PWD/../dist/include/QMdmmCore -f $$PWD/

#HEADERS_ABSOLUTE =
#for (header, QMDMM_HEADERS): HEADERS_ABSOLUTE += $$system_path($$absolute_path($$header))

#generateHeaders.depends = $$HEADERS_ABSOLUTE

#QMAKE_EXTRA_TARGETS += generateHeaders
#PRE_TARGETDEPS += $$generateHeaders.target

#includetarget.path = /include/QMdmmCore/
#includetarget.files = $$OUT_PWD/../dist/include/QMdmmCore/*

#target.path = /lib/
#dlltarget.path = /bin/
#INSTALLS += target dlltarget includetarget

#QMAKE_CLEAN += $$system_path($$OUT_PWD/../dist/include/QMdmmCore/.timestamp)
