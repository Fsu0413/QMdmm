// SPDX-License-Identifier: AGPL-3.0-or-later

#include "test.h"

#include <QMdmmAgent>
#include <QMdmmClient>
#include <QMdmmData>
#include <QMdmmLogicConfiguration>
#include <QMdmmLogicRunner>
#include <QMdmmServer>

#include <QTcpServer>
#include <QTcpSocket>
#include <QTest>
#include <QTimer>

// NOLINTBEGIN

using namespace QMdmmCore;
using namespace QMdmmNetworking;

class tst_QMdmmNetworking : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE tst_QMdmmNetworking() = default;

private slots:
    void signIn_disconnectInNotFullRoom_removesPlayer();
    void signIn_reconnectsPlayerInNonCurrentRoom();
    void reconnectDoesNotAutoTrust();
    void addAgent_registersLocalAgent();
    void client_exposesSelfAgent();
    void localAgent_asyncReplyContract();
    void client_giveUpTriggersServerDefaultReply();
    void client_actionOrderYieldAcceptsAssignment();
    void client_routesAgentStateChangeToSelfAgent();
    void server_disconnectsOnAbnormalPacket();
    void client_disconnectFromHostStopsAutoReconnect();
    void server_listenErrorAndClose();
};

// A room that is not full has not started a game yet: a dropped socket removes the player
// entirely (it can re-join later as a fresh player) rather than preserving the seat for a
// reconnect. Driven end-to-end through the public Server / Client API: p1 and p2 share a
// not-full room, p1 drops, and p2 observes the remove. (p1's own client then auto-reconnects
// and rejoins as a fresh player -- that reconnect path is covered by the reconnect test below
// and the smoke test.)
void tst_QMdmmNetworking::signIn_disconnectInNotFullRoom_removesPlayer()
{
    LogicConfiguration conf = LogicConfiguration::defaults();

    ServerConfiguration serverConf = ServerConfiguration::defaults();
    serverConf.setPlayerNumPerRoom(3); // 3-person room: two players leave it not-full
    serverConf.setTcpPort(16367);
    serverConf.setLocalEnabled(false);
    serverConf.setWebsocketEnabled(false);
    serverConf.setRequestTimeout(60); // bots stay silent without timing out during the test

    Server server(serverConf, conf);
    QVERIFY(server.listen());

    const QString host = QStringLiteral("qmdmm://localhost:16367");

    auto *p1 = new Client(ClientConfiguration(), &server);
    QVERIFY(p1->connectToHost(host, Data::StateOnline));
    QTRY_VERIFY_WITH_TIMEOUT(p1->room() != nullptr && p1->room()->player(p1->objectName()) != nullptr, 5000);

    auto *p2 = new Client(ClientConfiguration(), &server);
    QVERIFY(p2->connectToHost(host, Data::StateOnlineBot));
    QTRY_VERIFY_WITH_TIMEOUT(p2->room() != nullptr && p2->room()->player(p1->objectName()) != nullptr, 5000);

    // p1 and p2 share a not-full room (3-person room, 2 players).
    QVERIFY(p1->room()->player(p2->objectName()) != nullptr);

    // Drop p1's socket. The room is not full, so the server removes p1's agent and broadcasts
    // notifyPlayerRemove to the remaining player (instead of preserving the seat).
    bool p1Removed = false;
    connect(p2->agent(), &Agent::playerRemoveNotified, [&](const QString &playerName) {
        if (playerName == p1->objectName())
            p1Removed = true;
    });

    QTcpSocket *p1Sock = p1->findChild<QTcpSocket *>();
    QVERIFY(p1Sock != nullptr);
    p1Sock->abort();

    QTRY_VERIFY_WITH_TIMEOUT(p1Removed, 5000);
}

// A reconnecting player may live in ANY room, not just `current`. `current` only tracks
// the room currently recruiting; a full room keeps running in the background and is
// deleted only on gameOver. This is the regression test for the findChildren fix: once a
// later sign-in moves `current` to a fresh room, an offline player in the old full room
// must still be reconnected in whichever room it lives. Driven end-to-end through the
// public Server / Client API over a real TCP connection.
void tst_QMdmmNetworking::signIn_reconnectsPlayerInNonCurrentRoom()
{
    LogicConfiguration conf = LogicConfiguration::defaults();

    ServerConfiguration serverConf = ServerConfiguration::defaults();
    serverConf.setPlayerNumPerRoom(2);
    serverConf.setTcpPort(16366);
    serverConf.setLocalEnabled(false);
    serverConf.setWebsocketEnabled(false);
    serverConf.setRequestTimeout(60); // bots stay silent without timing out during the test

    Server server(serverConf, conf);
    QVERIFY(server.listen());

    const QString host = QStringLiteral("qmdmm://localhost:16366");

    // Sign in one at a time so room assignment is deterministic (p1 -> room 1, p2 fills
    // room 1, p3 -> fresh room 2).
    auto *p1 = new Client(ClientConfiguration(), &server);
    QVERIFY(p1->connectToHost(host, Data::StateOnline));
    QTRY_VERIFY_WITH_TIMEOUT(p1->room() != nullptr && p1->room()->player(p1->objectName()) != nullptr, 5000);

    auto *p2 = new Client(ClientConfiguration(), &server);
    QVERIFY(p2->connectToHost(host, Data::StateOnlineBot));
    QTRY_VERIFY_WITH_TIMEOUT(p1->room() != nullptr && p1->room()->player(p2->objectName()) != nullptr, 5000);

    auto *p3 = new Client(ClientConfiguration(), &server);
    QVERIFY(p3->connectToHost(host, Data::StateOnlineBot));
    QTRY_VERIFY_WITH_TIMEOUT(p3->room() != nullptr && p3->room()->player(p3->objectName()) != nullptr, 5000);

    // Drop p1 (in the non-current full room 1); it must reconnect into room 1, not room 2.
    QTcpSocket *p1Sock = p1->findChild<QTcpSocket *>();
    QVERIFY(p1Sock != nullptr);

    bool reconnected = false;
    connect(p1, &Client::socketReconnectSucceeded, [&reconnected]() { reconnected = true; });
    p1Sock->abort();

    QTRY_VERIFY_WITH_TIMEOUT(reconnected, 10000);

    // p1 is back in room 1 (it still sees p2), and it was not added as a new player to
    // room 2 (p3's room).
    QTRY_VERIFY_WITH_TIMEOUT(p1->room() != nullptr && p1->room()->player(p2->objectName()) != nullptr, 5000);
    QVERIFY(p3->room() == nullptr || p3->room()->player(p1->objectName()) == nullptr);
}

// D-021 regression: a reconnect restores the player's Online flag but must NOT auto-mark the
// player Trusted. Trust ("managed") is only toggled from the player's own client UI; the server
// must never default a reconnecting player to Trust (the old reconnectAgent set StateMaskTrust on
// reconnect, which is the bug this guards against). p2, still connected in the same full room,
// observes p1's reconnected state via the agentStateChangeNotified broadcast: the drop first
// marks p1 offline (Online cleared), then the reconnect must report it online with Trust clear.
void tst_QMdmmNetworking::reconnectDoesNotAutoTrust()
{
    LogicConfiguration conf = LogicConfiguration::defaults();

    ServerConfiguration serverConf = ServerConfiguration::defaults();
    serverConf.setPlayerNumPerRoom(2);
    serverConf.setTcpPort(16363);
    serverConf.setLocalEnabled(false);
    serverConf.setWebsocketEnabled(false);
    serverConf.setRequestTimeout(60); // bots stay silent without timing out during the test

    Server server(serverConf, conf);
    QVERIFY(server.listen());

    const QString host = QStringLiteral("qmdmm://localhost:16363");

    auto *p1 = new Client(ClientConfiguration(), &server);
    QVERIFY(p1->connectToHost(host, Data::StateOnline));
    QTRY_VERIFY_WITH_TIMEOUT(p1->room() != nullptr && p1->room()->player(p1->objectName()) != nullptr, 5000);

    auto *p2 = new Client(ClientConfiguration(), &server);
    QVERIFY(p2->connectToHost(host, Data::StateOnlineBot));
    QTRY_VERIFY_WITH_TIMEOUT(p2->room() != nullptr && p2->room()->player(p1->objectName()) != nullptr, 5000);

    // p2 observes p1's state broadcasts. The drop reports p1 offline (Online cleared); only the
    // reconnect broadcasts p1 online again, and it must do so with Trust still clear. If the old
    // auto-Trust behavior regressed, no broadcast would ever carry Online-without-Trust and this
    // flag stays false, failing the assertion below.
    bool p1OnlineNotTrusted = false;
    connect(p2->agent(), &Agent::agentStateChangeNotified, &server, [&p1OnlineNotTrusted, p1](const QString &playerName, const Data::AgentState &state) {
        if (playerName == p1->objectName() && state.testFlag(Data::StateMaskOnline) && !state.testFlag(Data::StateMaskTrust))
            p1OnlineNotTrusted = true;
    });

    QTcpSocket *p1Sock = p1->findChild<QTcpSocket *>();
    QVERIFY(p1Sock != nullptr);

    bool reconnected = false;
    connect(p1, &Client::socketReconnectSucceeded, &server, [&reconnected]() { reconnected = true; });
    p1Sock->abort();

    QTRY_VERIFY_WITH_TIMEOUT(reconnected, 10000);
    QTRY_VERIFY_WITH_TIMEOUT(p1OnlineNotTrusted, 5000);
}

// addAgent with a locally-owned agent (no ServerConnection child) registers a socket-less
// "local" agent (operation side = GUI / Bot): it joins the room and is reachable through
// agent(), without creating any wire plumbing. A local agent has no socket, so there is
// nothing to disconnect.
void tst_QMdmmNetworking::addAgent_registersLocalAgent()
{
    LogicConfiguration conf = LogicConfiguration::defaults();

    LogicRunner runner(conf, 3);

    Agent *local = new Agent(QStringLiteral("p1"), &runner);
    local->setScreenName(QStringLiteral("screen1"));
    local->setState(Data::StateOnline);

    QCOMPARE(runner.addAgent(local), local);
    QCOMPARE(runner.agent(QStringLiteral("p1")), local);
    QVERIFY(local->state().testFlag(Data::StateMaskOnline));
    QVERIFY(!runner.full()); // playerNumPerRoom = 3, only one agent added
}

// The client pre-creates its own Agent on construction (symmetric to the server side where
// the operation side creates the agent and hands it to LogicRunner), so the operation side
// always has an Agent to drive even before any network connection. agent() exposes it, keyed
// by the client's own objectName.
void tst_QMdmmNetworking::client_exposesSelfAgent()
{
    Client client(ClientConfiguration {});
    QVERIFY(client.agent() != nullptr);
    QCOMPARE(client.agent()->objectName(), client.objectName());
}

// A local (socket-less) agent plays through the async reply contract: its operation side (here
// the test itself) answers each xxxRequested signal asynchronously via singleShot(0), never
// synchronously. This is the end-to-end check that a local agent -- no ServerConnection, no
// socket -- can actually participate (receive a request, reply async, observe the result), not
// merely be registered. The request/reply round-trip only completes because the reply is
// deferred to the event loop, which is the contract's whole point.
void tst_QMdmmNetworking::localAgent_asyncReplyContract()
{
    LogicConfiguration conf = LogicConfiguration::defaults();

    LogicRunner runner(conf, 2);

    Agent *p1 = new Agent(QStringLiteral("p1"), &runner);
    p1->setState(Data::StateOnline);
    Agent *p2 = new Agent(QStringLiteral("p2"), &runner);
    p2->setState(Data::StateOnlineBot);

    int rpsRequests = 0;
    int rpsResults = 0;

    auto wireAsyncRpsReply = [&](Agent *agent) {
        QObject::connect(agent, &Agent::rockPaperScissorsRequested, &runner, [agent, &rpsRequests]() {
            ++rpsRequests;
            QTimer::singleShot(0, agent, [agent]() { agent->rockPaperScissors(Data::Rock); });
        });
    };
    wireAsyncRpsReply(p1);
    wireAsyncRpsReply(p2);

    // The RPS result notification is the observable proof that the async reply round-tripped
    // back through the logic side (Logic -> LogicRunnerP -> Agent::notifyRockPaperScissors).
    QObject::connect(p1, &Agent::rockPaperScissorsNotified, &runner, [&rpsResults](const QHash<QString, Data::RockPaperScissors> &) { ++rpsResults; });

    // Registering both agents fills the room and kicks off the game: gameStart + roundStart,
    // then Logic starts driving the first RPS request on its thread.
    QCOMPARE(runner.addAgent(p1), p1);
    QCOMPARE(runner.addAgent(p2), p2);
    QVERIFY(runner.full());

    // Both agents got asked and their async replies produced at least one RPS result. A
    // synchronous reply would never round-trip here; only the deferred singleShot(0) does.
    QTRY_VERIFY_WITH_TIMEOUT(rpsResults >= 1, 5000);
    QVERIFY(rpsRequests >= 2);
}

// A client whose operation side gives up on a request (Agent::requestTimeout) sends a null reply
// carrying the *correct* request id, and the server recognizes the null value as the give-up
// marker and applies its default reply -- so the logic keeps advancing instead of stalling or
// erroring out. Driven end-to-end over a real TCP connection: p1 (a bot) answers RPS normally,
// p2 (a "human") gives up, and p1 observing the RPS result is the proof that p2's default reply
// was applied. This is the D-020 regression test (the old code reset currentRequest before
// sending, so the give-up reply carried RequestInvalid and was dropped by the server).
void tst_QMdmmNetworking::client_giveUpTriggersServerDefaultReply()
{
    LogicConfiguration conf = LogicConfiguration::defaults();

    ServerConfiguration serverConf = ServerConfiguration::defaults();
    serverConf.setPlayerNumPerRoom(2);
    serverConf.setTcpPort(16365);
    serverConf.setLocalEnabled(false);
    serverConf.setWebsocketEnabled(false);
    serverConf.setRequestTimeout(60); // the server's own request timer must not fire during the test

    Server server(serverConf, conf);
    QVERIFY(server.listen());

    const QString host = QStringLiteral("qmdmm://localhost:16365");

    // Wire both agents BEFORE connecting: the first RPS request fires as soon as the room fills
    // (p2 joins), and a signal connected after connectToHost would miss it.
    auto *p1 = new Client(ClientConfiguration(), &server);
    connect(p1->agent(), &Agent::rockPaperScissorsRequested, &server, [p1]() { QTimer::singleShot(0, p1->agent(), [p1]() { p1->agent()->rockPaperScissors(Data::Rock); }); });

    auto *p2 = new Client(ClientConfiguration(), &server);
    int p2GiveUps = 0;
    connect(p2->agent(), &Agent::rockPaperScissorsRequested, &server, [&p2GiveUps, p2]() {
        ++p2GiveUps;
        p2->agent()->requestTimeout();
    });

    // The observable proof: p1 receives the RPS result -- which only happens if the server
    // applied p2's default reply (rather than dropping the give-up and stalling the logic).
    int rpsResults = 0;
    connect(p1->agent(), &Agent::rockPaperScissorsNotified, &server, [&rpsResults](const QHash<QString, Data::RockPaperScissors> &) { ++rpsResults; });

    // p1 joins first (room not full yet); p2 fills the room, kicking off the game + first RPS.
    QVERIFY(p1->connectToHost(host, Data::StateOnlineBot));
    QTRY_VERIFY_WITH_TIMEOUT(p1->room() != nullptr && p1->room()->player(p1->objectName()) != nullptr, 5000);

    QVERIFY(p2->connectToHost(host, Data::StateOnline));

    QTRY_VERIFY_WITH_TIMEOUT(p2GiveUps >= 1, 5000);
    QTRY_VERIFY_WITH_TIMEOUT(rpsResults >= 1, 10000);
}

// A client yields the action-order contest by replying with a 0 sentinel -- the "yield" marker:
// accept whatever order is assigned and stop competing. Yielding is an explicit reply carrying
// semantics, distinct from requestTimeout()'s null give-up (which makes the server answer with
// its default reply). The 0 sentinel must round-trip the wire (ClientP encodes it into the JSON
// array, ServerConnection decodes it back) and reach the core Logic, which counts the yield and
// hands the leftover order to the yielder. In a 3-player room p1/p2 win the RPS (Rock beats p3's
// Scissors) and enter the action-order negotiation; p1 yields while p2 strives for order 1. p2
// reaching the action phase is the proof that p1's yield was applied -- if the 0 sentinel had not
// round-tripped, the logic would stall in the action-order phase waiting for p1's selection.
void tst_QMdmmNetworking::client_actionOrderYieldAcceptsAssignment()
{
    LogicConfiguration conf = LogicConfiguration::defaults();

    ServerConfiguration serverConf = ServerConfiguration::defaults();
    serverConf.setPlayerNumPerRoom(3);
    serverConf.setTcpPort(16361);
    serverConf.setLocalEnabled(false);
    serverConf.setWebsocketEnabled(false);
    serverConf.setRequestTimeout(60); // the server's request timer must not fire during the test

    Server server(serverConf, conf);
    QVERIFY(server.listen());

    const QString host = QStringLiteral("qmdmm://localhost:16361");

    // Wire the replies before connecting: the first RPS request fires as soon as the room fills
    // (p3 joins), and a signal connected after connectToHost would miss it. p1 and p2 play Rock,
    // p3 plays Scissors, so Rock beats Scissors and the RPS winners are [p1, p2] -- two winners
    // enter the action-order negotiation (a single winner would just take every order without it).
    auto *p1 = new Client(ClientConfiguration(), &server);
    connect(p1->agent(), &Agent::rockPaperScissorsRequested, &server, [p1]() { p1->agent()->rockPaperScissors(Data::Rock); });
    auto *p2 = new Client(ClientConfiguration(), &server);
    connect(p2->agent(), &Agent::rockPaperScissorsRequested, &server, [p2]() { p2->agent()->rockPaperScissors(Data::Rock); });
    auto *p3 = new Client(ClientConfiguration(), &server);
    connect(p3->agent(), &Agent::rockPaperScissorsRequested, &server, [p3]() { p3->agent()->rockPaperScissors(Data::Scissors); });

    // p1 yields its action-order selection (0 sentinel); p2 strives for order 1.
    int p1ActionOrderRequests = 0;
    connect(p1->agent(), &Agent::actionOrderRequested, &server, [p1, &p1ActionOrderRequests](const QList<int> &, int, int) {
        ++p1ActionOrderRequests;
        p1->agent()->actionOrder({0});
    });
    connect(p2->agent(), &Agent::actionOrderRequested, &server, [p2](const QList<int> &, int, int) { p2->agent()->actionOrder({1}); });

    // p2 (order 1) acts first, then p1 (assigned the leftover order 2); both reply DoNothing to
    // keep the turn moving. p2 reaching the action phase is the proof p1's yield took effect.
    int p2ActionRequests = 0;
    int p1ActionRequests = 0;
    connect(p2->agent(), &Agent::actionRequested, &server, [p2, &p2ActionRequests](int) {
        ++p2ActionRequests;
        p2->agent()->action(Data::DoNothing, {}, 0);
    });
    connect(p1->agent(), &Agent::actionRequested, &server, [p1, &p1ActionRequests](int) {
        ++p1ActionRequests;
        p1->agent()->action(Data::DoNothing, {}, 0);
    });

    QVERIFY(p1->connectToHost(host, Data::StateOnline));
    QTRY_VERIFY_WITH_TIMEOUT(p1->room() != nullptr && p1->room()->player(p1->objectName()) != nullptr, 5000);
    QVERIFY(p2->connectToHost(host, Data::StateOnline));
    QTRY_VERIFY_WITH_TIMEOUT(p2->room() != nullptr && p2->room()->player(p1->objectName()) != nullptr, 5000);
    QVERIFY(p3->connectToHost(host, Data::StateOnline));
    QTRY_VERIFY_WITH_TIMEOUT(p3->room() != nullptr && p3->room()->player(p1->objectName()) != nullptr, 5000);

    // p1 entered the action-order negotiation and yielded.
    QTRY_VERIFY_WITH_TIMEOUT(p1ActionOrderRequests >= 1, 5000);

    // The game advanced past the action-order phase into the action phase for both winners.
    QTRY_VERIFY_WITH_TIMEOUT(p2ActionRequests >= 1, 10000);
    QTRY_VERIFY_WITH_TIMEOUT(p1ActionRequests >= 1, 10000);
}

// A player's state change (online -> offline on a drop) is broadcast to every client, and the
// receiving client must route it out through its selfAgent's agentStateChangeNotified signal --
// not just silently setState the mirror agent. This is the regression test for the D-019
// clarification ("agent state must be routed over"): without the selfAgent->notifyAgentStateChange
// call, the change updates the mirror agent's data but never reaches the operation side (GUI).
void tst_QMdmmNetworking::client_routesAgentStateChangeToSelfAgent()
{
    LogicConfiguration conf = LogicConfiguration::defaults();

    ServerConfiguration serverConf = ServerConfiguration::defaults();
    serverConf.setPlayerNumPerRoom(3); // not full: p2's drop marks it offline then removes it
    serverConf.setTcpPort(16364);
    serverConf.setLocalEnabled(false);
    serverConf.setWebsocketEnabled(false);
    serverConf.setRequestTimeout(60);

    Server server(serverConf, conf);
    QVERIFY(server.listen());

    const QString host = QStringLiteral("qmdmm://localhost:16364");

    auto *p1 = new Client(ClientConfiguration(), &server);
    QVERIFY(p1->connectToHost(host, Data::StateOnline));
    QTRY_VERIFY_WITH_TIMEOUT(p1->room() != nullptr && p1->room()->player(p1->objectName()) != nullptr, 5000);

    auto *p2 = new Client(ClientConfiguration(), &server);
    QVERIFY(p2->connectToHost(host, Data::StateOnlineBot));
    QTRY_VERIFY_WITH_TIMEOUT(p2->room() != nullptr && p2->room()->player(p1->objectName()) != nullptr, 5000);

    // p2's mirror agent exists in p1's view.
    QVERIFY(p1->room()->player(p2->objectName()) != nullptr);

    // Wire the observation BEFORE dropping p2: its state change (online -> offline) is broadcast
    // to p1, whose selfAgent must route it out via agentStateChangeNotified.
    bool stateRouted = false;
    connect(p1->agent(), &Agent::agentStateChangeNotified, &server, [&stateRouted, p2](const QString &playerName, const Data::AgentState &state) {
        if (playerName == p2->objectName() && !state.testFlag(Data::StateMaskOnline))
            stateRouted = true;
    });

    QTcpSocket *p2Sock = p2->findChild<QTcpSocket *>();
    QVERIFY(p2Sock != nullptr);
    p2Sock->abort();

    QTRY_VERIFY_WITH_TIMEOUT(stateRouted, 5000);
}

// A client sending a packet the server does not expect from a client -- here an invalid packet
// type -- is an abnormal case (D-025): the server must drop the connection rather than silently
// ignore the packet. In a not-full room the drop removes the misbehaving player, so the remaining
// player observes notifyPlayerRemove. The invalid packet is written straight onto the client's raw
// TCP socket (the public Client API has no "send arbitrary packet" entry point).
void tst_QMdmmNetworking::server_disconnectsOnAbnormalPacket()
{
    LogicConfiguration conf = LogicConfiguration::defaults();

    ServerConfiguration serverConf = ServerConfiguration::defaults();
    serverConf.setPlayerNumPerRoom(3); // not full with two players: the drop removes, not preserves
    serverConf.setTcpPort(16362);
    serverConf.setLocalEnabled(false);
    serverConf.setWebsocketEnabled(false);
    serverConf.setRequestTimeout(60);

    Server server(serverConf, conf);
    QVERIFY(server.listen());

    const QString host = QStringLiteral("qmdmm://localhost:16362");

    auto *p1 = new Client(ClientConfiguration(), &server);
    QVERIFY(p1->connectToHost(host, Data::StateOnline));
    QTRY_VERIFY_WITH_TIMEOUT(p1->room() != nullptr && p1->room()->player(p1->objectName()) != nullptr, 5000);

    auto *p2 = new Client(ClientConfiguration(), &server);
    QVERIFY(p2->connectToHost(host, Data::StateOnlineBot));
    QTRY_VERIFY_WITH_TIMEOUT(p2->room() != nullptr && p2->room()->player(p1->objectName()) != nullptr, 5000);

    bool p2Removed = false;
    connect(p1->agent(), &Agent::playerRemoveNotified, &server, [&p2Removed, p2](const QString &playerName) {
        if (playerName == p2->objectName())
            p2Removed = true;
    });

    // Write a packet with an out-of-range type (99) straight onto p2's raw socket. fromJson rejects
    // it at the protocol layer (enum range check), so the socket marks itself errored and p2 drops.
    QTcpSocket *p2Sock = p2->findChild<QTcpSocket *>();
    QVERIFY(p2Sock != nullptr);
    p2Sock->write("{\"type\":99,\"requestId\":0,\"notifyId\":0,\"value\":null}\n");
    p2Sock->flush();

    QTRY_VERIFY_WITH_TIMEOUT(p2Removed, 5000);
}

// A client can actively disconnect via disconnectFromHost(): the reconnect loop is stopped, no
// "connection lost" / "reconnect succeeded" signal fires (those are the passive-drop notices),
// and isConnected() flips to false immediately. The upper layer stays in control and can
// reconnect with a fresh connectToHost. This is the A1 regression test: before the API existed
// the only way to disconnect was destroying the whole Client.
void tst_QMdmmNetworking::client_disconnectFromHostStopsAutoReconnect()
{
    LogicConfiguration conf = LogicConfiguration::defaults();

    ServerConfiguration serverConf = ServerConfiguration::defaults();
    serverConf.setPlayerNumPerRoom(2);
    serverConf.setTcpPort(16360);
    serverConf.setLocalEnabled(false);
    serverConf.setWebsocketEnabled(false);
    serverConf.setRequestTimeout(60);

    Server server(serverConf, conf);
    QVERIFY(server.listen());

    const QString host = QStringLiteral("qmdmm://localhost:16360");

    auto *p1 = new Client(ClientConfiguration(), &server);
    QVERIFY(!p1->isConnected());

    QVERIFY(p1->connectToHost(host, Data::StateOnline));
    QVERIFY(p1->isConnected());
    QTRY_VERIFY_WITH_TIMEOUT(p1->room() != nullptr && p1->room()->player(p1->objectName()) != nullptr, 5000);

    bool connectionLost = false;
    bool reconnectSucceeded = false;
    connect(p1, &Client::socketConnectionLost, &server, [&connectionLost](const QString &) { connectionLost = true; });
    connect(p1, &Client::socketReconnectSucceeded, &server, [&reconnectSucceeded]() { reconnectSucceeded = true; });

    p1->disconnectFromHost();
    QVERIFY(!p1->isConnected());

    // The reconnect loop is stopped: even after the first retry interval (500ms) elapses the
    // client stays disconnected, and neither passive-drop signal has fired.
    QTest::qWait(700);
    QVERIFY(!p1->isConnected());
    QVERIFY(!connectionLost);
    QVERIFY(!reconnectSucceeded);

    // A fresh connectToHost reconnects normally.
    QVERIFY(p1->connectToHost(host, Data::StateOnline));
    QVERIFY(p1->isConnected());
    QTRY_VERIFY_WITH_TIMEOUT(p1->room() != nullptr && p1->room()->player(p1->objectName()) != nullptr, 5000);
}

// The server reports per-transport listen failures via listenError (the aggregate return value
// alone can't tell which transport broke), and close() shuts the listening sockets down so a
// later listen() can bind again. This is the A2 regression test.
void tst_QMdmmNetworking::server_listenErrorAndClose()
{
    LogicConfiguration conf = LogicConfiguration::defaults();

    // Occupy a TCP port so the server's TCP transport cannot bind it.
    QTcpServer blocker;
    QVERIFY(blocker.listen(QHostAddress::Any, 16361));

    ServerConfiguration serverConf = ServerConfiguration::defaults();
    serverConf.setTcpPort(16361);
    serverConf.setLocalEnabled(false);
    serverConf.setWebsocketEnabled(false);

    Server server(serverConf, conf);

    QString errorTransport;
    QString errorString;
    connect(&server, &Server::listenError, [&](const QString &transport, const QString &err) {
        errorTransport = transport;
        errorString = err;
    });

    // The port is taken, so listen() fails and reports which transport broke.
    QVERIFY(!server.listen());
    QCOMPARE(errorTransport, QStringLiteral("tcp"));
    QVERIFY(!errorString.isEmpty());

    // Free the port; the server can now bind it, close() releases it, and a second listen()
    // binds again (proving the listening socket was actually shut down).
    blocker.close();
    QVERIFY(server.listen());
    server.close();
    QVERIFY(server.listen());
}

namespace {
RegisterTestObject<tst_QMdmmNetworking> _;
}
#include "tst_qmdmmnetworking.moc"
