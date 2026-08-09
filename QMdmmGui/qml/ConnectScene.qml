// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 6.5

Item {
    id: scene
    anchors.fill: parent

    signal back
    signal startLocal(string name)
    signal connectOnline(string host, string name)

    // dim background
    Rectangle {
        anchors.fill: parent
        color: "#66000000"
    }

    Item {
        anchors.centerIn: parent
        width: 720
        height: 560

        Rectangle {
            anchors.fill: parent
            color: "#1b1b1b"
            radius: 12
            border.color: "#555"
        }

        Text {
            id: title
            anchors.top: parent.top
            anchors.topMargin: 24
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("连接游戏")
            color: "white"
            font.pixelSize: 40
        }

        Text {
            anchors.top: title.bottom
            anchors.topMargin: 24
            anchors.left: parent.left
            anchors.leftMargin: 60
            text: qsTr("你的名字")
            color: "#ccc"
        }
        Rectangle {
            id: nameBox
            anchors.top: title.bottom
            anchors.topMargin: 56
            anchors.left: parent.left
            anchors.leftMargin: 60
            width: 600
            height: 56
            color: "#000"
            border.color: "#666"
            radius: 6
            TextInput {
                id: nameInput
                anchors.fill: parent
                anchors.margins: 8
                color: "white"
                text: "You"
                font.pixelSize: 28
                verticalAlignment: TextInput.AlignVCenter
            }
        }

        // Local mode
        Text {
            anchors.top: nameBox.bottom
            anchors.topMargin: 28
            anchors.left: parent.left
            anchors.leftMargin: 60
            text: qsTr("本地对战（进程内服务器 + 机器人填满房间）")
            color: "#ccc"
        }
        Text {
            anchors.top: nameBox.bottom
            anchors.topMargin: 64
            anchors.left: parent.left
            anchors.leftMargin: 60
            text: qsTr("人数: ") + game.playerCount
            color: "white"
            font.pixelSize: 28
        }
        Button {
            height: 48; width: 48
            anchors.top: nameBox.bottom
            anchors.topMargin: 56
            anchors.left: parent.left
            anchors.leftMargin: 200
            source: "../assets/btn.png"
            text: "-"
            onClicked: game.playerCount = Math.max(1, game.playerCount - 1)
        }
        Button {
            height: 48; width: 48
            anchors.top: nameBox.bottom
            anchors.topMargin: 56
            anchors.left: parent.left
            anchors.leftMargin: 260
            source: "../assets/btn.png"
            text: "+"
            onClicked: game.playerCount = Math.min(6, game.playerCount + 1)
        }
        Button {
            height: 56
            width: 220
            anchors.top: nameBox.bottom
            anchors.topMargin: 52
            anchors.right: parent.right
            anchors.rightMargin: 60
            source: "../assets/btn.png"
            text: qsTr("开始本地游戏")
            onClicked: scene.startLocal(nameInput.text)
        }

        // Online mode
        Text {
            anchors.top: parent.top
            anchors.topMargin: 300
            anchors.left: parent.left
            anchors.leftMargin: 60
            text: qsTr("在线模式（连接外部服务器，如 qmdmm://host:6366）")
            color: "#ccc"
        }
        Rectangle {
            id: hostBox
            anchors.top: parent.top
            anchors.topMargin: 332
            anchors.left: parent.left
            anchors.leftMargin: 60
            width: 420
            height: 48
            color: "#000"
            border.color: "#666"
            radius: 6
            TextInput {
                id: hostInput
                anchors.fill: parent
                anchors.margins: 8
                color: "white"
                text: "qmdmm://localhost:6366"
                font.pixelSize: 22
                verticalAlignment: TextInput.AlignVCenter
            }
        }
        Button {
            height: 56
            width: 180
            anchors.top: parent.top
            anchors.topMargin: 328
            anchors.right: parent.right
            anchors.rightMargin: 60
            source: "../assets/btn.png"
            text: qsTr("连接")
            onClicked: scene.connectOnline(hostInput.text, nameInput.text)
        }

        Button {
            height: 48
            width: 120
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 16
            anchors.left: parent.left
            anchors.leftMargin: 60
            source: "../assets/btn.png"
            text: qsTr("返回")
            onClicked: scene.back()
        }
    }
}
