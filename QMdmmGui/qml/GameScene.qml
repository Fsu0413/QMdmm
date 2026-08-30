// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 6.5

import "."

Item {
    id: scene

    property var actionOptions: []

    // ---- properties / logic ----------------------------------------------
    property string activeRequest: ""
    property int orderNeed: 0
    property var orderOptions: []
    property int orderRemaining: 0
    property var orderSelected: []
    property int upgradeNeed: 0
    property var upgradeOptions: []
    property int upgradeRemaining: 0
    property var upgradeSelected: []

    function toggleOrder(v) {
        const i = orderSelected.indexOf(v);
        if (i >= 0) {
            orderSelected.splice(i, 1);
        } else if (orderRemaining > 0) {
            orderSelected.push(v);
        }
        orderRemaining = orderNeed - orderSelected.length;
    }

    function toggleUpgrade(v) {
        const i = upgradeSelected.indexOf(v);
        if (i >= 0) {
            upgradeSelected.splice(i, 1);
        } else if (upgradeRemaining > 0) {
            upgradeSelected.push(v);
        }
        upgradeRemaining = upgradeNeed - upgradeSelected.length;
    }

    anchors.fill: parent

    // ---- top bar ----------------------------------------------------------
    Rectangle {
        id: topbar

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        color: "#cc000000"
        height: 56

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            color: "white"
            font.pixelSize: 28
            text: {
                if (game.gameState === "lobby")
                    return qsTr("Lobby: waiting for other players...");
                if (game.gameState === "playing")
                    return qsTr("Match in progress");
                if (game.gameState === "gameover")
                    return qsTr("Match over");
                return "";
            }
        }

        Button {
            anchors.right: parent.right
            anchors.rightMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            height: 44
            source: "../assets/btn.png"
            text: qsTr("Disconnect")
            width: 120

            onClicked: game.disconnectAll()
        }
    }

    // ---- player panels ----------------------------------------------------
    Row {
        id: playersRow

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: topbar.bottom
        anchors.topMargin: 16
        spacing: 16

        Repeater {
            model: game.players

            PlayerCard {
                displayName: game.screenName(modelData.objectName)
                player: modelData
                you: game.isYou(modelData.objectName)
            }
        }
    }

    // ---- log / chat -------------------------------------------------------
    Item {
        id: logArea

        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
        anchors.left: parent.left
        anchors.leftMargin: 40
        anchors.top: playersRow.bottom
        anchors.topMargin: 12
        width: parent.width / 2 - 50

        Rectangle {
            anchors.fill: parent
            color: "#33000000"
            radius: 8
        }

        Flickable {
            id: logFlick

            anchors.fill: parent
            anchors.margins: 8
            clip: true
            contentHeight: logCol.height

            Column {
                id: logCol

                spacing: 4
                width: parent.width

                Repeater {
                    model: game.chatLog

                    Text {
                        color: game.isYou(modelData.name) ? "#ffe082" : "white"
                        font.pixelSize: 22
                        text: game.screenName(modelData.name) + ": " + modelData.content
                        width: logArea.width - 16
                        wrapMode: Text.Wrap
                    }
                }
            }
        }

        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.margins: 8
            anchors.right: parent.right
            border.color: "#666"
            color: "#000"
            height: 44
            radius: 6

            TextInput {
                id: chatInput

                anchors.fill: parent
                anchors.margins: 6
                color: "white"
                font.pixelSize: 20
                verticalAlignment: TextInput.AlignVCenter

                onAccepted: {
                    game.speak(text);
                    text = "";
                }
            }
        }
    }

    // ---- request overlay --------------------------------------------------
    Item {
        id: requestOverlay

        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
        anchors.right: parent.right
        anchors.rightMargin: 40
        anchors.top: playersRow.bottom
        anchors.topMargin: 12
        visible: activeRequest !== ""
        width: parent.width / 2 - 50

        Rectangle {
            anchors.fill: parent
            color: "#44000000"
            radius: 8
        }

        // SSC
        Column {
            anchors.centerIn: parent
            spacing: 12
            visible: activeRequest === "ssc"

            Text {
                color: "white"
                font.pixelSize: 30
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("Rock-paper-scissors!")
                width: requestOverlay.width
            }

            Row {
                spacing: 12

                Button {
                    height: 80
                    source: "../assets/btn.png"
                    text: qsTr("Rock")
                    width: 120

                    onClicked: {
                        game.replySsc(0);
                        activeRequest = "";
                    }
                }

                Button {
                    height: 80
                    source: "../assets/btn.png"
                    text: qsTr("Scissors")
                    width: 120

                    onClicked: {
                        game.replySsc(1);
                        activeRequest = "";
                    }
                }

                Button {
                    height: 80
                    source: "../assets/btn.png"
                    text: qsTr("Paper")
                    width: 120

                    onClicked: {
                        game.replySsc(2);
                        activeRequest = "";
                    }
                }
            }
        }

        // Action order
        Column {
            anchors.centerIn: parent
            spacing: 10
            visible: activeRequest === "order"

            Text {
                color: "white"
                font.pixelSize: 26
                text: qsTr("Choose action order (%1 more)").arg(orderRemaining)
                width: requestOverlay.width
                wrapMode: Text.Wrap
            }

            Flow {
                spacing: 8
                width: requestOverlay.width - 20

                Repeater {
                    model: orderOptions

                    Button {
                        height: 56
                        source: "../assets/btn.png"
                        text: String(modelData)
                        width: 90

                        onClicked: toggleOrder(modelData)
                    }
                }
            }

            Row {
                spacing: 12

                Button {
                    enabled: orderRemaining === 0
                    height: 56
                    source: "../assets/btn.png"
                    text: qsTr("Confirm order")
                    width: 200

                    onClicked: {
                        game.replyActionOrder(orderSelected);
                        activeRequest = "";
                    }
                }

                Button {
                    height: 56
                    source: "../assets/btn.png"
                    text: qsTr("Yield (auto-assign)")
                    width: 200

                    onClicked: {
                        game.yieldActionOrder(orderNeed);
                        activeRequest = "";
                    }
                }
            }
        }

        // Action
        Column {
            anchors.centerIn: parent
            spacing: 10
            visible: activeRequest === "action"

            Text {
                color: "white"
                font.pixelSize: 28
                text: qsTr("Your turn to act")
            }

            Flow {
                spacing: 8
                width: requestOverlay.width - 20

                Repeater {
                    model: actionOptions

                    Button {
                        height: 56
                        source: "../assets/btn.png"
                        text: modelData.label
                        width: Math.min(260, requestOverlay.width - 20)

                        onClicked: {
                            game.replyAction(modelData.action, modelData.target, modelData.place);
                            activeRequest = "";
                        }
                    }
                }
            }
        }

        // Upgrade
        Column {
            anchors.centerIn: parent
            spacing: 10
            visible: activeRequest === "upgrade"

            Text {
                color: "white"
                font.pixelSize: 26
                text: qsTr("Upgrade (%1 more)").arg(upgradeRemaining)
                width: requestOverlay.width
                wrapMode: Text.Wrap
            }

            Flow {
                spacing: 8
                width: requestOverlay.width - 20

                Repeater {
                    model: upgradeOptions

                    Button {
                        height: 56
                        source: "../assets/btn.png"
                        text: modelData.label
                        width: 240

                        onClicked: toggleUpgrade(modelData.item)
                    }
                }
            }

            Row {
                spacing: 12

                Button {
                    enabled: upgradeRemaining === 0
                    height: 56
                    source: "../assets/btn.png"
                    text: qsTr("Confirm upgrade")
                    width: 200

                    onClicked: {
                        game.replyUpgrade(upgradeSelected);
                        activeRequest = "";
                    }
                }

                Button {
                    height: 56
                    source: "../assets/btn.png"
                    text: qsTr("Default (HP)")
                    width: 200

                    onClicked: {
                        for (var i = 0; i < upgradeNeed; ++i)
                            upgradeSelected.push(2);
                        game.replyUpgrade(upgradeSelected);
                        activeRequest = "";
                    }
                }
            }
        }
    }

    // ---- round / game over banners ---------------------------------------
    Item {
        id: banner

        anchors.fill: parent
        visible: false

        Rectangle {
            anchors.fill: parent
            color: "#aa000000"
        }

        Text {
            id: bannerText

            anchors.centerIn: parent
            color: "white"
            font.pixelSize: 64
            text: ""
        }
    }

    Connections {
        function onGameOver(winners) {
            activeRequest = "";
            let names = [];
            for (let i = 0; i < winners.length; ++i)
                names.push(game.screenName(winners[i]));
            bannerText.text = qsTr("Winner(s): ") + names.join(", ");
            banner.visible = true;
        }

        function onRequestAction() {
            actionOptions = game.getActionOptions();
            activeRequest = "action";
        }

        function onRequestActionOrder(remainedOrders, maximumOrder, selectionNum) {
            orderOptions = remainedOrders;
            orderSelected = [];
            orderNeed = selectionNum;
            orderRemaining = selectionNum;
            activeRequest = "order";
        }

        function onRequestStoneScissorsCloth(playerNames, strivedOrder) {
            activeRequest = "ssc";
        }

        function onRequestUpgrade(remainingTimes) {
            upgradeOptions = game.getUpgradeOptions();
            upgradeSelected = [];
            upgradeNeed = remainingTimes;
            upgradeRemaining = remainingTimes;
            activeRequest = "upgrade";
        }

        function onRoundOver() {
            bannerText.text = qsTr("Round over");
            banner.visible = true;
            bannerTimer.start();
        }

        target: game
    }

    Timer {
        id: bannerTimer

        interval: 1200
        repeat: false

        onTriggered: banner.visible = false
    }
}
