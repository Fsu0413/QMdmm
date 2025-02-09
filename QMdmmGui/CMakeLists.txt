# SPDX-License-Identifier: AGPL-3.0-or-later

# This GUI will be Qt 6 only
# Qt 5 and Qt 6 QML CMake API is incompatible (qt6_add_qml_module) so ...

set(QMDMMGUI_PCH
    src/qmdmmgui.h
)

set(QMDMMGUI_SOURCES
    ${QMDMMGUI_PCH}
    src/main.cpp
    src/cppif.h
)

set(QMDMMGUI_QML_FILES
    qml/main.qml
    qml/Button.qml
    qml/StartScene.qml
    qml/Window.qml
)
set(QMDMMGUI_RESOURCES
    assets/1.jpg
    assets/btn.jpg
)

qt6_add_executable(QMdmm6 WIN32 MACOSX_BUNDLE ${QMDMMGUI_SOURCES})

set_target_properties(QMdmm6 PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
    VERSION "${QMdmm_VERSION}"
)
target_precompile_headers(QMdmm6
    PRIVATE "${QMDMMGUI_PCH}"
)
target_link_libraries(QMdmm6
    PUBLIC QMdmmNetworking6
           QMdmmCore6
           Qt6::Quick
           Qt6::Qml
           Qt6::Gui
           Qt6::Core
)
qt6_add_qml_module(QMdmm6
    URI QMdmm.Gui
    VERSION ${QMdmm_VERSION}
    QML_FILES ${QMDMMGUI_QML_FILES}
    RESOURCES ${QMDMMGUI_RESOURCES}
)
install(TARGETS QMdmm6
    COMPONENT "6"
    DESTINATION "${CMAKE_INSTALL_BINDIR}"
    RUNTIME
    BUNDLE
)
