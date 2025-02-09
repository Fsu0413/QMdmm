import QtQuick 6.5

Item {
    property Loader contentLoader: contentLoader

    property string title

    Text {
        text: title
    }

    Loader {
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 25
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width / 16 * 15
        height: (parent.height - 25) / 16 * 15

        id: contentLoader
    }
}
