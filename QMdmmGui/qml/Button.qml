// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 6.5

Image {
    id: qMdmmButton
    property bool checkable: false
    property bool checked: false

    property bool enabled: true

    property string text
    property font font

    signal clicked()
    signal doubleClicked()

    onClicked: {
        if (checkable)
            checked = !checked;
    }

    state: "exited"

    states: [
        State {
            name: "exited"
            PropertyChanges {
                target: hover
                visible: false
            }
        },
        State {
            name: "entered"
            PropertyChanges {
                target: hover
                visible: true
            }
        },
        State {
            name: "downEntered"
            PropertyChanges {
                target: hover
                visible: true
            }
        },
        State {
            name: "downExited"
            PropertyChanges {
                target: hover
                visible: true
            }
        },
        State {
            name: "disabled"
            PropertyChanges {
                target: hover
                visible: true
            }
        }
    ]

    onEnabledChanged: {
        if (!enabled)
            state = "disabled"
        else
            state = "exited"
    }

    Rectangle {
        id: hover
        anchors.fill: parent
        color: Qt.rgba(1, 1, 1, .25)
        visible: false
    }

    Text {
        anchors.centerIn: parent
        width: parent.width / 4 * 3
        height: parent.height / 8 * 7
        fontSizeMode: Text.Fit
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: parent.text
        font: parent.font
    }

    MouseArea {
        id: qMdmmButtonMouseArea
        anchors.fill: parent
        hoverEnabled: true

        onClicked: {
            if (parent.enabled) {
                parent.clicked()
                if (!parent.checkable || !parent.checked)
                    parent.state = "entered"
                else
                    parent.state = "downEntered"
            }
        }

        onDoubleClicked: {
            if (parent.enabled) {
                parent.doubleClicked()
            }
        }

        onEntered: {
            if (parent.enabled) {
                if (!parent.checkable || !parent.checked)
                    parent.state = "entered"
                else
                    parent.state = "downEntered"
            }
        }

        onExited: {
            if (parent.enabled) {
                if (!parent.checkable || !parent.checked)
                    parent.state = "exited"
                else
                    parent.state = "downExited"
            }
        }

        onPressed: {
            if (parent.enabled) {
                parent.state = "downEntered"
            } else {
                mouse.accepted = false
            }
        }

        onReleased:  {
            if (parent.enabled) {
                if (!parent.checkable || !parent.checked)
                    parent.state = "entered"
                else
                    parent.state = "downEntered"
            }
        }
    }
}
