SOURCES += \
    $$PWD/json_reader.cpp \
    $$PWD/json_value.cpp \
    $$PWD/json_writer.cpp

JSON_HEADERS += \
    $$PWD/json/config.h \
    $$PWD/json/features.h \
    $$PWD/json/forwards.h \
    $$PWD/json/json.h \
    $$PWD/json/reader.h \
    $$PWD/json/value.h \
    $$PWD/json/version.h \
    $$PWD/json/writer.h \
    $$PWD/json/allocator.h \
    $$PWD/json/assertions.h \
    $$PWD/json/autolink.h \
    $$PWD/json_tool.h

HEADERS += $$JSON_HEADERS

INCLUDEPATH += $$PWD

generateHeadersJson.target = $$system_path($$OUT_PWD/../dist/include/json/.timestamp)
!build_pass: mkpath($$OUT_PWD/../dist/include/json)

contains(QMAKE_HOST.os, "Windows"): generateHeadersJson.commands = cscript $$system_path($$PWD/../../../tools/AutoGenerateHeader.vbs) -o $$system_path($$OUT_PWD/../dist/include/json) -f $$system_path($$PWD/json/)
else: generateHeadersJson.commands = $$PWD/../../../tools/AutoGenerateHeader.sh -o $$OUT_PWD/../dist/include/json -f $$PWD/json/

JSON_HEADERS_ABSOLUTE =
for (header, JSON_HEADERS): JSON_HEADERS_ABSOLUTE += $$system_path($$absolute_path($$header))

generateHeadersJson.depends = $$JSON_HEADERS_ABSOLUTE

QMAKE_EXTRA_TARGETS += generateHeadersJson
PRE_TARGETDEPS += $$generateHeadersJson.target

DEFINES += JSON_DLL_BUILD

win32-msvc*: QMAKE_CXXFLAGS += -wd4275

QMAKE_CLEAN += $$system_path($$OUT_PWD/../dist/include/json/.timestamp)
