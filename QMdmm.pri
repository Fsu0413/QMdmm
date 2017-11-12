
CONFIG += warn_on c++11
win32: CONFIG += skip_target_version_ext

VERSION = 0.5.0

DEFINES += QT_NO_CAST_FROM_ASCII

android || ios {
    DEFINES += MOBILE_DEVICES
}
staticlib: DEFINES += QMDMM_STATIC

!isEmpty($$(PREFIX)) {
    CONFIG += install_build
}

!win32-msvc* {
    # we use gcc/clang on unix-like systems and mingw
    QMAKE_CFLAGS += -Wpointer-to-int-cast
    QMAKE_CXXFLAGS += -Wc++11-compat
    *-g++: QMAKE_CXXFLAGS += -Wzero-as-null-pointer-constant
    mac:QMAKE_LFLAGS += -Wl,-undefined -Wl,error
    else:QMAKE_LFLAGS += -Wl,--no-undefined
}

LIBS += -L$$OUT_PWD/../dist/lib
INCLUDEPATH += $$OUT_PWD/../dist/include

