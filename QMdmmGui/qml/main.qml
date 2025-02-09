// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 6.5

Item {
    anchors.fill: parent

    Item {
        visible: true

        height: 1024
        width: Math.max(800, parent.width * 1024 / Math.max(0.1, parent.height))

        transformOrigin: Item.TopLeft
        scale: parent.height / 1024

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
    }
}
