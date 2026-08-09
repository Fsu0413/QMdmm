// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 6.5

Image {
    id: rootItem

    anchors.fill: parent

    source: "../assets/1.jpg"

    StartScene {
        id: startScene
        anchors.fill: parent
        visible: game.gameState === "start"

        onStartGameClicked: connectScene.visible = true
        onConfigureClicked: configPopup.visible = true
        onAboutClicked: aboutPopup.visible = true
    }

    ConnectScene {
        id: connectScene
        anchors.fill: parent
        visible: false

        onBack: visible = false
        onStartLocal: (name) => { visible = false; game.startLocalGame(name); }
        onConnectOnline: (host, name) => { visible = false; game.connectOnline(host, name); }
    }

    GameScene {
        id: gameScene
        anchors.fill: parent
        visible: game.gameState === "lobby" || game.gameState === "playing" || game.gameState === "gameover"
    }

    // ---- simple popups ----------------------------------------------------

    Item {
        id: configPopup
        anchors.fill: parent
        visible: false

        Rectangle {
            anchors.fill: parent
            color: "#80000000"
        }
        Item {
            anchors.centerIn: parent
            width: 600
            height: 200
            Rectangle {
                anchors.fill: parent
                color: "#222"
                radius: 10
                border.color: "#555"
            }
            Text {
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: 16
                text: qsTr("玩家人数（本地对战）")
                color: "white"
            }
            Text {
                id: cfgCount
                anchors.centerIn: parent
                text: game.playerCount
                color: "white"
                font.pixelSize: 48
            }
            Button {
                height: 64
                width: 120
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 20
                source: "../assets/btn.png"
                text: "-"
                onClicked: game.playerCount = Math.max(1, game.playerCount - 1)
            }
            Button {
                height: 64
                width: 120
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 20
                source: "../assets/btn.png"
                text: "+"
                onClicked: game.playerCount = Math.min(6, game.playerCount + 1)
            }
            Button {
                height: 56
                width: 160
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 12
                anchors.horizontalCenter: parent.horizontalCenter
                source: "../assets/btn.png"
                text: qsTr("确定")
                onClicked: configPopup.visible = false
            }
        }
    }

    Item {
        id: aboutPopup
        anchors.fill: parent
        visible: false

        Rectangle {
            anchors.fill: parent
            color: "#80000000"
            MouseArea { anchors.fill: parent; onClicked: aboutPopup.visible = false }
        }
        Item {
            anchors.centerIn: parent
            width: 700
            height: 360
            Rectangle {
                anchors.fill: parent
                color: "#222"
                radius: 10
                border.color: "#555"
            }
            Text {
                anchors.fill: parent
                anchors.margins: 24
                wrapMode: Text.Wrap
                color: "white"
                text: qsTr("QMdmm —— 打打闹闹\n\n多人回合制小游戏。Qt 6 / C++20 实现，AGPL-3.0。\n\n本地模式会在进程内启动服务器，并用机器人填满房间，\n单人也能玩完整一局。在线模式可连接外部服务器。")
            }
            Button {
                height: 56
                width: 160
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 12
                anchors.horizontalCenter: parent.horizontalCenter
                source: "../assets/btn.png"
                text: qsTr("关闭")
                onClicked: aboutPopup.visible = false
            }
        }
    }
}
