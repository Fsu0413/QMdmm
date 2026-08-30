// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 6.5

import "."

Item {
    id: card

    property string displayName
    property var player
    property bool you

    height: 300
    width: 280

    Rectangle {
        anchors.fill: parent
        border.color: you ? "#7c4" : "#555"
        border.width: you ? 3 : 1
        color: you ? "#223322" : "#222222"
        radius: 10
    }

    Text {
        id: nameText

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 10
        color: "white"
        elide: Text.ElideRight
        font.pixelSize: 28
        horizontalAlignment: Text.AlignHCenter
        text: displayName + (you ? qsTr(" (you)") : "")
        width: parent.width - 20
    }

    // HP bar
    Rectangle {
        id: hpBack

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: nameText.bottom
        anchors.topMargin: 14
        color: "#000"
        height: 26
        radius: 4
        width: parent.width - 40

        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.top: parent.top
            color: player.hp > player.maxHp * 0.3 ? "#4caf50" : "#e53935"
            radius: 4
            width: parent.width * Math.max(0, Math.min(1, player.hp / Math.max(1, player.maxHp)))
        }

        Text {
            anchors.centerIn: parent
            color: "white"
            font.pixelSize: 18
            text: player.hp + " / " + player.maxHp
        }
    }

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: hpBack.bottom
        anchors.topMargin: 16
        spacing: 12

        Image {
            fillMode: Image.PreserveAspectFit
            height: 48
            source: "../assets/knife.png"
            visible: player.hasKnife
            width: 48
        }

        Image {
            fillMode: Image.PreserveAspectFit
            height: 48
            source: "../assets/horse.png"
            visible: player.hasHorse
            width: 48
        }
    }

    Text {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#ddd"
        font.pixelSize: 20
        text: qsTr("Place: %1").arg(game.placeName(player.place))
    }

    Text {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 14
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#ddd"
        font.pixelSize: 20
        text: qsTr("Upgrade points: %1").arg(player.upgradePoint)
    }

    Rectangle {
        anchors.fill: parent
        color: "#aa000000"
        radius: 10
        visible: player.dead

        Text {
            anchors.centerIn: parent
            color: "#f55"
            font.pixelSize: 40
            text: qsTr("Out")
        }
    }
}
