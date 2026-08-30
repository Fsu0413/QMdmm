// SPDX-License-Identifier: AGPL-3.0-or-later

import QtQuick 2.15
import QtTest 1.2

import QMdmm.Gui 1.0

// Smoke test for the QMdmmGameClient bridge: drives a local game through the
// exact entry point the QML UI uses (startLocalGame), verifies the room fills
// and the match reaches the playing state, then checks the human's request /
// reply round trip actually advances the match. This is the runtime verification
// the GUI previously lacked.
TestCase {
    id: testCase

    property var game: null

    function cleanup() {
        if (game !== null) {
            game.disconnectAll();
            game.destroy();
        }
        game = null;
    }

    function init() {
        game = gameComponent.createObject(testCase);
        verify(game !== null, "GameClient should instantiate");
    }

    function test_localGameFillsRoomAndStarts() {
        // 1 human + 1 auto-replying bot -> room fills and the match starts.
        game.playerCount = 2;
        game.startLocalGame("Tester");

        tryCompare(game, "gameState", "playing", 15000);
        compare(game.players.length, 2);
        verify(game.isYou(game.localName));

        game.disconnectAll();
        compare(game.gameState, "start");
        compare(game.players.length, 0);
    }

    function test_replyDrivesTheMatch() {
        var rps = createTemporaryObject(signalSpyComponent, testCase, {
                                            target: game,
                                            signalName: "requestRockPaperScissors"
                                        });
        var rpsResult = createTemporaryObject(signalSpyComponent, testCase, {
                                                  target: game,
                                                  signalName: "rpsResult"
                                              });

        game.playerCount = 2;
        game.startLocalGame("Tester");
        tryCompare(game, "gameState", "playing", 15000);

        // The server asks the human for a rock-paper-scissors pick.
        tryCompare(rps, "count", 1, 15000);

        // The auto-replying bot answers its own RPS request, so the human's reply
        // below completes the round and produces an rpsResult broadcast. Require the
        // human's reply to yield *at least one more* result rather than an exact
        // count — an exact count depends on how the match advances, which is
        // fragile: under the old 80 ms request-timeout default the auto-advancing
        // match kept settling and the count reached 194.
        //
        // Note: this test intentionally does NOT rely on the request-timeout
        // fallback. requestTimeout is now seconds-scale (20 s + 60 s grace, see
        // ServerConfiguration), far longer than the 15 s window below, so a human
        // that stops replying leaves the match waiting; the bot's auto-reply is
        // the only thing driving progress.
        var before = rpsResult.count;
        game.replyRps(0); // rock

        tryVerify(function () {
            return rpsResult.count > before;
        }, 15000);
    }

    name: "GameClient"

    Component {
        id: gameComponent

        GameClient {
        }
    }

    Component {
        id: signalSpyComponent

        SignalSpy {
        }
    }
}
