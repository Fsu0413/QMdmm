// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.15
import QtTest 1.2

// Smoke test for the match log GameScene builds out of the operation-result
// signals: the rock-paper-scissors picks, the action order, the actions and the
// upgrades of *every* player have to show up there, not only the ones the local
// player makes. That is the gap this guards -- the request handlers only ever
// cover your own turn. The match / round start markers are guarded here too:
// they carry no operation data, so nothing else would put them on screen.
//
// Like tst_scene.qml, the scene is loaded from the source tree (the QMdmm.Gui
// module resource lives in the QMdmm6 executable, which this test does not
// link). It reads the client through the `game` context property, which
// tst_qmdmmgui.cpp installs the same way MainWindow does. Emitting the client's
// result signals is what the networking side does when a notification arrives.
TestCase {
    id: testCase

    function makeScene() {
        const comp = Qt.createComponent(Qt.resolvedUrl("../qml/GameScene.qml"));
        tryCompare(comp, "status", Component.Ready);
        verify(comp.status === Component.Ready, "GameScene should load");

        const scene = createTemporaryObject(comp, testCase);
        verify(scene !== null, "GameScene should instantiate");
        return scene;
    }

    function test_actionOrderResultIsLogged() {
        const scene = makeScene();

        // Key "1" is the first order, so it decides who acts first.
        game.actionOrderResult({
                                   "1": "p2",
                                   "2": "p1"
                               });

        compare(scene.matchLog.length, 1);
        compare(scene.matchLog[0], "Action order: p2 then p1");
    }

    function test_actionResultIsLogged() {
        const scene = makeScene();

        // One line per action kind: every branch of the action-to-text mapping
        // gets guarded, not just the interesting ones.
        game.actionResult("p1", 0, "", 0); // DoNothing
        game.actionResult("p1", 1, "", -1); // BuyKnife
        game.actionResult("p1", 2, "", -1); // BuyHorse
        game.actionResult("p1", 3, "p2", -1); // Slash
        game.actionResult("p1", 4, "p2", -1); // Kick
        game.actionResult("p1", 5, "", 2); // Move (toPlace carries the target)
        game.actionResult("p1", 6, "p2", 2); // LetMove (both carry information)

        compare(scene.matchLog.length, 7);
        compare(scene.matchLog[0], "p1 did nothing");
        compare(scene.matchLog[1], "p1 bought a knife");
        compare(scene.matchLog[2], "p1 bought a horse");
        compare(scene.matchLog[3], "p1 slashed p2");
        compare(scene.matchLog[4], "p1 kicked p2");
        compare(scene.matchLog[5], "p1 moved to City 2");
        compare(scene.matchLog[6], "p1 moved p2 to City 2");
    }

    function test_gameStartIsLogged() {
        const scene = makeScene();

        game.gameStart();

        compare(scene.matchLog.length, 1);
        compare(scene.matchLog[0], "Match started");
    }

    function test_matchLogIsBounded() {
        const scene = makeScene();

        // A whole match is long; the log must not grow without end.
        for (let i = 0; i < 250; ++i)
            game.rpsResult({
                               "p1": 0
                           });

        compare(scene.matchLog.length, 200);
    }

    function test_roundStartIsLogged() {
        const scene = makeScene();

        game.roundStart();

        compare(scene.matchLog.length, 1);
        compare(scene.matchLog[0], "Round started");
    }

    function test_rpsResultIsLogged() {
        const scene = makeScene();

        // All three throws, so no branch of the throw-to-text mapping is left
        // unguarded.
        game.rpsResult({
                           "p1": 0,
                           "p2": 1,
                           "p3": 2
                       });

        compare(scene.matchLog.length, 1);
        compare(scene.matchLog[0], "Rock-paper-scissors: p1 (Rock), p2 (Scissors), p3 (Paper)");
    }

    function test_upgradeResultIsLogged() {
        const scene = makeScene();

        // All three upgrade items, for the same reason.
        game.upgradeResult({
                               "p1": [0, 1, 2]
                           });

        compare(scene.matchLog.length, 1);
        compare(scene.matchLog[0], "Upgrades: p1 (knife damage, horse damage, max HP)");
    }

    name: "GameScene"
}
