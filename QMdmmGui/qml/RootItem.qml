// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 6.5

Image {
    id: rootItem

    anchors.fill: parent

    source: "../assets/1.jpg"

    // Component.onCompleted: cppif.testSlot()
    StartScene {
        anchors.fill: parent

        onStartGameClicked: cppif.testSlot()
        onConfigureClicked: cppif.testSlot()
        onAboutClicked: cppif.testSlot()
    }

    Window {
        id: resizableWindow
        visible: false
    }
}
