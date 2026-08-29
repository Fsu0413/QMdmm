// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 6.5

import "."

Item {
    id: scene
    anchors.fill: parent

    // ---- top bar ----------------------------------------------------------
    Rectangle {
        id: topbar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 56
        color: "#cc000000"
        Text {
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            text: {
                if (game.gameState === "lobby") return qsTr("Lobby: waiting for other players...");
                if (game.gameState === "playing") return qsTr("Match in progress");
                if (game.gameState === "gameover") return qsTr("Match over");
                return "";
            }
            color: "white"
            font.pixelSize: 28
        }
        Button {
            height: 44
            width: 120
            anchors.right: parent.right
            anchors.rightMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            source: "../assets/btn.png"
            text: qsTr("Disconnect")
            onClicked: game.disconnectAll()
        }
    }

    // ---- player panels ----------------------------------------------------
    Row {
        id: playersRow
        anchors.top: topbar.bottom
        anchors.topMargin: 16
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 16

        Repeater {
            model: game.players
            PlayerCard {
                player: modelData
                displayName: game.screenName(modelData.objectName)
                you: game.isYou(modelData.objectName)
            }
        }
    }

    // ---- log / chat -------------------------------------------------------
    Item {
        id: logArea
        anchors.top: playersRow.bottom
        anchors.topMargin: 12
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
        anchors.left: parent.left
        anchors.leftMargin: 40
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
            contentHeight: logCol.height
            clip: true
            Column {
                id: logCol
                width: parent.width
                spacing: 4
                Repeater {
                    model: game.chatLog
                    Text {
                        width: logArea.width - 16
                        wrapMode: Text.Wrap
                        text: game.screenName(modelData.name) + ": " + modelData.content
                        color: game.isYou(modelData.name) ? "#ffe082" : "white"
                        font.pixelSize: 22
                    }
                }
            }
        }
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 8
            height: 44
            color: "#000"
            border.color: "#666"
            radius: 6
            TextInput {
                id: chatInput
                anchors.fill: parent
                anchors.margins: 6
                color: "white"
                font.pixelSize: 20
                verticalAlignment: TextInput.AlignVCenter
                onAccepted: { game.speak(text); text = ""; }
            }
        }
    }

    // ---- request overlay --------------------------------------------------
    Item {
        id: requestOverlay
        anchors.top: playersRow.bottom
        anchors.topMargin: 12
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
        anchors.right: parent.right
        anchors.rightMargin: 40
        width: parent.width / 2 - 50
        visible: activeRequest !== ""

        Rectangle {
            anchors.fill: parent
            color: "#44000000"
            radius: 8
        }

        // SSC
        Column {
            visible: activeRequest === "ssc"
            anchors.centerIn: parent
            spacing: 12
            Text { text: qsTr("Rock-paper-scissors!"); color: "white"; font.pixelSize: 30; horizontalAlignment: Text.AlignHCenter; width: requestOverlay.width }
            Row {
                spacing: 12
                Button { height: 80; width: 120; source: "../assets/btn.png"; text: qsTr("Rock"); onClicked: { game.replySsc(0); activeRequest = ""; } }
                Button { height: 80; width: 120; source: "../assets/btn.png"; text: qsTr("Scissors"); onClicked: { game.replySsc(1); activeRequest = ""; } }
                Button { height: 80; width: 120; source: "../assets/btn.png"; text: qsTr("Paper"); onClicked: { game.replySsc(2); activeRequest = ""; } }
            }
        }

        // Action order
        Column {
            visible: activeRequest === "order"
            anchors.centerIn: parent
            spacing: 10
            Text { text: qsTr("Choose action order (%1 more)").arg(orderRemaining); color: "white"; font.pixelSize: 26; width: requestOverlay.width; wrapMode: Text.Wrap }
            Flow {
                width: requestOverlay.width - 20
                spacing: 8
                Repeater {
                    model: orderOptions
                    Button {
                        height: 56; width: 90
                        source: "../assets/btn.png"
                        text: String(modelData)
                        onClicked: toggleOrder(modelData)
                    }
                }
            }
            Row {
                spacing: 12
                Button {
                    height: 56; width: 200
                    source: "../assets/btn.png"
                    text: qsTr("Confirm order")
                    enabled: orderRemaining === 0
                    onClicked: { game.replyActionOrder(orderSelected); activeRequest = ""; }
                }
                Button {
                    height: 56; width: 200
                    source: "../assets/btn.png"
                    text: qsTr("Yield (auto-assign)")
                    onClicked: { game.yieldActionOrder(orderNeed); activeRequest = ""; }
                }
            }
        }

        // Action
        Column {
            visible: activeRequest === "action"
            anchors.centerIn: parent
            spacing: 10
            Text { text: qsTr("Your turn to act"); color: "white"; font.pixelSize: 28 }
            Flow {
                width: requestOverlay.width - 20
                spacing: 8
                Repeater {
                    model: actionOptions
                    Button {
                        height: 56
                        width: Math.min(260, requestOverlay.width - 20)
                        source: "../assets/btn.png"
                        text: modelData.label
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
            visible: activeRequest === "upgrade"
            anchors.centerIn: parent
            spacing: 10
            Text { text: qsTr("Upgrade (%1 more)").arg(upgradeRemaining); color: "white"; font.pixelSize: 26; width: requestOverlay.width; wrapMode: Text.Wrap }
            Flow {
                width: requestOverlay.width - 20
                spacing: 8
                Repeater {
                    model: upgradeOptions
                    Button {
                        height: 56; width: 240
                        source: "../assets/btn.png"
                        text: modelData.label
                        onClicked: toggleUpgrade(modelData.item)
                    }
                }
            }
            Row {
                spacing: 12
                Button {
                    height: 56; width: 200
                    source: "../assets/btn.png"
                    text: qsTr("Confirm upgrade")
                    enabled: upgradeRemaining === 0
                    onClicked: { game.replyUpgrade(upgradeSelected); activeRequest = ""; }
                }
                Button {
                    height: 56; width: 200
                    source: "../assets/btn.png"
                    text: qsTr("Default (HP)")
                    onClicked: { for (var i = 0; i < upgradeNeed; ++i) upgradeSelected.push(2); game.replyUpgrade(upgradeSelected); activeRequest = ""; }
                }
            }
        }
    }

    // ---- round / game over banners ---------------------------------------
    Item {
        id: banner
        anchors.fill: parent
        visible: false
        Rectangle { anchors.fill: parent; color: "#aa000000" }
        Text {
            id: bannerText
            anchors.centerIn: parent
            color: "white"
            font.pixelSize: 64
            text: ""
        }
    }

    // ---- properties / logic ----------------------------------------------
    property string activeRequest: ""
    property var actionOptions: []
    property var orderOptions: []
    property var orderSelected: []
    property int orderRemaining: 0
    property var upgradeOptions: []
    property var upgradeSelected: []
    property int upgradeRemaining: 0
    property int upgradeNeed: 0

    function toggleOrder(v) {
        const i = orderSelected.indexOf(v);
        if (i >= 0) { orderSelected.splice(i, 1); }
        else if (orderRemaining > 0) { orderSelected.push(v); }
        orderRemaining = orderNeed - orderSelected.length;
    }
    property int orderNeed: 0

    function toggleUpgrade(v) {
        const i = upgradeSelected.indexOf(v);
        if (i >= 0) { upgradeSelected.splice(i, 1); }
        else if (upgradeRemaining > 0) { upgradeSelected.push(v); }
        upgradeRemaining = upgradeNeed - upgradeSelected.length;
    }

    Connections {
        target: game
        function onRequestStoneScissorsCloth(playerNames, strivedOrder) {
            activeRequest = "ssc";
        }
        function onRequestActionOrder(remainedOrders, maximumOrder, selectionNum) {
            orderOptions = remainedOrders;
            orderSelected = [];
            orderNeed = selectionNum;
            orderRemaining = selectionNum;
            activeRequest = "order";
        }
        function onRequestAction() {
            actionOptions = game.getActionOptions();
            activeRequest = "action";
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
        function onGameOver(winners) {
            activeRequest = "";
            let names = [];
            for (let i = 0; i < winners.length; ++i) names.push(game.screenName(winners[i]));
            bannerText.text = qsTr("Winner(s): ") + names.join(", ");
            banner.visible = true;
        }
    }

    Timer {
        id: bannerTimer
        interval: 1200
        repeat: false
        onTriggered: banner.visible = false
    }
}
