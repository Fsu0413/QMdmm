// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 6.5

Item {
    id: scene
    anchors.fill: parent

    signal startGameClicked
    signal configureClicked
    signal aboutClicked

    Item {
        y: 512
        height: 384

        x: parent.width / 8 * 3
        width: parent.width / 4

        Grid {
            anchors.fill: parent
            columns: 1

            Button {
                height: parent.height / 3
                width: parent.width

                source: "../assets/btn.png"
                text: qsTr("Start game")

                onClicked: scene.startGameClicked()
            }

            Button {
                height: parent.height / 3
                width: parent.width

                source: "../assets/btn.png"
                text: qsTr("Configuration")

                onClicked: scene.configureClicked()
            }

            Button {
                height: parent.height / 3
                width: parent.width

                source: "../assets/btn.png"
                text: qsTr("About")

                onClicked: scene.aboutClicked()
            }
        }
    }
}
