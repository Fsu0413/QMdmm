// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 6.5

Image {
    id: rootItem

    signal startGameButtonClicked()
    signal configureButtonClicked()
    signal aboutButtonClicked()

    anchors.fill: parent

    source: "../assets/1.jpg"

    // Component.onCompleted: cppif.testSlot()
    StartScene{
        anchors.fill: parent
    }

    Window {
        id: resizableWindow
        visible: false
    }
}
