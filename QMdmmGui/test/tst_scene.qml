// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.15
import QtTest 1.2

// Interaction smoke test for the QML scenes: loads the real StartScene from the
// source tree (exercising its `import "."` Button import and `../assets/`
// resolution, which the app relies on) and verifies the scene's button wiring —
// triggering the "Start game" button must emit the scene's startGameClicked
// signal that RootItem connects to the connect scene.
TestCase {
    id: testCase

    function findButtonByText(root, text) {
        for (var i = 0; i < root.children.length; ++i) {
            var c = root.children[i];
            if (c.text === text)
                return c;
            var found = findButtonByText(c, text);
            if (found)
                return found;
        }
        return null;
    }

    function test_startSceneStartGameButton() {
        var comp = Qt.createComponent(Qt.resolvedUrl("../qml/StartScene.qml"));
        tryCompare(comp, "status", Component.Ready);
        verify(comp.status === Component.Ready, "StartScene should load");

        var scene = createTemporaryObject(comp, testCase);
        verify(scene !== null);

        var spy = createTemporaryObject(signalSpyComponent, testCase, {
                                            target: scene,
                                            signalName: "startGameClicked"
                                        });

        var button = findButtonByText(scene, "Start game");
        verify(button !== null, "Start game button should exist");

        // Emit the button's clicked signal (what its MouseArea does on a real
        // click) and assert the scene forwards it as startGameClicked.
        button.clicked();
        compare(spy.count, 1, "clicking Start game should emit startGameClicked");
    }

    name: "Scenes"

    Component {
        id: signalSpyComponent

        SignalSpy {
        }
    }
}
