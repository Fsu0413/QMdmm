// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 6.5

Item {
    anchors.fill: parent

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

                source: "../assets/btn.jpg"
                text: qsTr("Start game")

                onClicked: rootItem.startGameButtonClicked()
            }

            Button {
                height: parent.height / 3
                width: parent.width

                source: "../assets/btn.jpg"
                text: qsTr("Configuration")

                onClicked: rootItem.configureButtonClicked()
            }

            Button {
                height: parent.height / 3
                width: parent.width

                source: "../assets/btn.jpg"
                text: qsTr("About")

                onClicked: rootItem.aboutButtonClicked()
            }
        }
    }
}
