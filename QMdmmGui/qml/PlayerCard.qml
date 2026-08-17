// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 6.5

import "."

Item {
    id: card
    width: 280
    height: 300

    property var player
    property string displayName
    property bool you

    Rectangle {
        anchors.fill: parent
        color: you ? "#223322" : "#222222"
        radius: 10
        border.color: you ? "#7c4" : "#555"
        border.width: you ? 3 : 1
    }

    Text {
        id: nameText
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        text: displayName + (you ? qsTr(" (you)") : "")
        color: "white"
        font.pixelSize: 28
        elide: Text.ElideRight
        width: parent.width - 20
        horizontalAlignment: Text.AlignHCenter
    }

    // HP bar
    Rectangle {
        id: hpBack
        anchors.top: nameText.bottom
        anchors.topMargin: 14
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 40
        height: 26
        color: "#000"
        radius: 4
        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.width * Math.max(0, Math.min(1, player.hp / Math.max(1, player.maxHp)))
            color: player.hp > player.maxHp * 0.3 ? "#4caf50" : "#e53935"
            radius: 4
        }
        Text {
            anchors.centerIn: parent
            text: player.hp + " / " + player.maxHp
            color: "white"
            font.pixelSize: 18
        }
    }

    Row {
        anchors.top: hpBack.bottom
        anchors.topMargin: 16
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 12
        Image { source: "../assets/knife.png"; visible: player.hasKnife; width: 48; height: 48; fillMode: Image.PreserveAspectFit }
        Image { source: "../assets/horse.png"; visible: player.hasHorse; width: 48; height: 48; fillMode: Image.PreserveAspectFit }
    }

    Text {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
        anchors.horizontalCenter: parent.horizontalCenter
        text: qsTr("Place: %1").arg(game.placeName(player.place))
        color: "#ddd"
        font.pixelSize: 20
    }
    Text {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 14
        anchors.horizontalCenter: parent.horizontalCenter
        text: qsTr("Upgrade points: %1").arg(player.upgradePoint)
        color: "#ddd"
        font.pixelSize: 20
    }

    Rectangle {
        anchors.fill: parent
        color: "#aa000000"
        radius: 10
        visible: player.dead
        Text {
            anchors.centerIn: parent
            text: qsTr("Out")
            color: "#f55"
            font.pixelSize: 40
        }
    }
}
