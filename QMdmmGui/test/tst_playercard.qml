// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.15
import QtTest 1.2

// Smoke test for the agent-state line on a player card. The state arrives as a bare mask
// (Data::AgentState: online / bot / managed) and the card is the only place in the GUI where
// it turns into words -- the managed flag in particular has nothing else on screen, so each
// bit gets its own case here. Like tst_scene.qml, the card is loaded from the source tree
// (the QMdmm.Gui module resource lives in the QMdmm6 executable, which this test does not
// link) and reads the client through the `game` context property that tst_qmdmmgui.cpp
// installs the same way MainWindow does. The player is a plain object: the card only ever
// reads properties off it.
TestCase {
    id: testCase

    function expectStateText(agentState, expected) {
        const card = makeCard(agentState);
        verify(hasText(card, expected), "state " + agentState + " should read as '" + expected + "'");
    }

    function hasText(root, text) {
        for (let i = 0; i < root.children.length; ++i) {
            const c = root.children[i];
            if (c.text === text)
                return true;
            if (hasText(c, text))
                return true;
        }
        return false;
    }

    function makeCard(agentState) {
        const comp = Qt.createComponent(Qt.resolvedUrl("../qml/PlayerCard.qml"));
        tryCompare(comp, "status", Component.Ready);
        verify(comp.status === Component.Ready, "PlayerCard should load");

        const card = createTemporaryObject(comp, testCase, {
                                               agentState: agentState,
                                               displayName: "P1",
                                               player: {
                                                   "dead": false,
                                                   "hasHorse": false,
                                                   "hasKnife": false,
                                                   "hp": 10,
                                                   "maxHp": 20,
                                                   "place": 0,
                                                   "upgradePoint": 0
                                               }
                                           });
        verify(card !== null, "PlayerCard should instantiate");
        return card;
    }

    function test_eachStateBitIsSpelledOut() {
        // One case per bit, with both readings of the online bit: a bit the wire sets has to
        // show up, and a bit it does not set must not be invented. The state is read off the
        // rendered card, not off the mapping, so the binding is on trial too.
        expectStateText(0x00, "Offline");
        expectStateText(0x10, "Online");
        expectStateText(0x08, "Offline, Managed");
        expectStateText(0x11, "Online, Bot");
        expectStateText(0x18, "Online, Managed");
        expectStateText(0x19, "Online, Bot, Managed");
    }

    name: "PlayerCard"
}
