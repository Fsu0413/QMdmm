// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 6.5

Item {
    anchors.fill: parent

    Item {
        height: 1024
        scale: parent.height / 1024
        transformOrigin: Item.TopLeft
        visible: true
        width: Math.max(800, parent.width * 1024 / Math.max(0.1, parent.height))

        RootItem {
            // All visual children can assume the height is never changed
            // only horizontal position / width may be considered when resizing
        }
    }
}
