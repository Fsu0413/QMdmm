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
    name: "GameClient"

    property var game: null

    Component {
        id: gameComponent
        GameClient {}
    }

    Component {
        id: signalSpyComponent
        SignalSpy {}
    }

    function init() {
        game = gameComponent.createObject(testCase)
        verify(game !== null, "GameClient should instantiate")
    }

    function cleanup() {
        if (game !== null) {
            game.disconnectAll()
            game.destroy()
        }
        game = null
    }

    function test_localGameFillsRoomAndStarts() {
        // 1 human + 1 auto-replying bot -> room fills and the match starts.
        game.playerCount = 2
        game.startLocalGame("Tester")

        tryCompare(game, "gameState", "playing", 15000)
        compare(game.players.length, 2)
        verify(game.isYou(game.localName))

        game.disconnectAll()
        compare(game.gameState, "start")
        compare(game.players.length, 0)
    }

    function test_replyDrivesTheMatch() {
        var ssc = createTemporaryObject(signalSpyComponent, testCase,
                                        { target: game, signalName: "requestStoneScissorsCloth" })
        var sscResult = createTemporaryObject(signalSpyComponent, testCase,
                                              { target: game, signalName: "sscResult" })

        game.playerCount = 2
        game.startLocalGame("Tester")
        tryCompare(game, "gameState", "playing", 15000)

        // The server asks the human for a rock-paper-scissors pick.
        tryCompare(ssc, "count", 1, 15000)

        // The auto-replying bot keeps the match advancing on its own (and the human
        // falls back to the server's default reply on later rounds), so sscResult is
        // broadcast once per round and keeps growing. Require the human's reply to
        // produce *at least one more* result rather than an exact count — an exact
        // match races the auto-advancing game (observed flaky on CI: count reached 194).
        var before = sscResult.count
        game.replySsc(0) // rock

        tryVerify(function() { return sscResult.count > before; }, 15000)
    }
}
