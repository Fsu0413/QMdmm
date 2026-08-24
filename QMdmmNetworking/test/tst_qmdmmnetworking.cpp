// SPDX-License-Identifier: AGPL-3.0-or-later

#include "test.h"

#include <QMdmmAgent>
#include <QMdmmClient>
#include <QMdmmData>
#include <QMdmmLogicConfiguration>
#include <QMdmmLogicRunner>
#include <QMdmmServer>

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
    void addAgent_registersLocalAgent();
    void client_exposesSelfAgent();
    void localAgent_asyncReplyContract();
    void client_giveUpTriggersServerDefaultReply();
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
    conf.setPlayerNumPerRoom(3); // 3-person room: two players leave it not-full

    ServerConfiguration serverConf = ServerConfiguration::defaults();
    serverConf.setTcpPort(16367);
    serverConf.setLocalEnabled(false);
    serverConf.setWebsocketEnabled(false);
    serverConf.setRequestTimeout(60000); // bots stay silent without timing out during the test

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
    conf.setPlayerNumPerRoom(2);

    ServerConfiguration serverConf = ServerConfiguration::defaults();
    serverConf.setTcpPort(16366);
    serverConf.setLocalEnabled(false);
    serverConf.setWebsocketEnabled(false);
    serverConf.setRequestTimeout(60000); // bots stay silent without timing out during the test

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

// addAgent with a locally-owned agent (no ServerConnection child) registers a socket-less
// "local" agent (operation side = GUI / Bot): it joins the room and is reachable through
// agent(), without creating any wire plumbing. A local agent has no socket, so there is
// nothing to disconnect.
void tst_QMdmmNetworking::addAgent_registersLocalAgent()
{
    LogicConfiguration conf = LogicConfiguration::defaults();
    conf.setPlayerNumPerRoom(3);

    LogicRunner runner(conf);

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
    conf.setPlayerNumPerRoom(2);

    LogicRunner runner(conf);

    Agent *p1 = new Agent(QStringLiteral("p1"), &runner);
    p1->setState(Data::StateOnline);
    Agent *p2 = new Agent(QStringLiteral("p2"), &runner);
    p2->setState(Data::StateOnlineBot);

    int sscRequests = 0;
    int sscResults = 0;

    auto wireAsyncSscReply = [&](Agent *agent) {
        QObject::connect(agent, &Agent::stoneScissorsClothRequested, &runner, [agent, &sscRequests]() {
            ++sscRequests;
            QTimer::singleShot(0, agent, [agent]() { agent->stoneScissorsCloth(Data::Stone); });
        });
    };
    wireAsyncSscReply(p1);
    wireAsyncSscReply(p2);

    // The SSC result notification is the observable proof that the async reply round-tripped
    // back through the logic side (Logic -> LogicRunnerP -> Agent::notifyStoneScissorsCloth).
    QObject::connect(p1, &Agent::stoneScissorsClothNotified, &runner, [&sscResults](const QHash<QString, Data::StoneScissorsCloth> &) { ++sscResults; });

    // Registering both agents fills the room and kicks off the game: gameStart + roundStart,
    // then Logic starts driving the first SSC request on its thread.
    QCOMPARE(runner.addAgent(p1), p1);
    QCOMPARE(runner.addAgent(p2), p2);
    QVERIFY(runner.full());

    // Both agents got asked and their async replies produced at least one SSC result. A
    // synchronous reply would never round-trip here; only the deferred singleShot(0) does.
    QTRY_VERIFY_WITH_TIMEOUT(sscResults >= 1, 5000);
    QVERIFY(sscRequests >= 2);
}

// A client whose operation side gives up on a request (Agent::requestTimeout) sends a null reply
// carrying the *correct* request id, and the server recognizes the null value as the give-up
// marker and applies its default reply -- so the logic keeps advancing instead of stalling or
// erroring out. Driven end-to-end over a real TCP connection: p1 (a bot) answers SSC normally,
// p2 (a "human") gives up, and p1 observing the SSC result is the proof that p2's default reply
// was applied. This is the D-020 regression test (the old code reset currentRequest before
// sending, so the give-up reply carried RequestInvalid and was dropped by the server).
void tst_QMdmmNetworking::client_giveUpTriggersServerDefaultReply()
{
    LogicConfiguration conf = LogicConfiguration::defaults();
    conf.setPlayerNumPerRoom(2);

    ServerConfiguration serverConf = ServerConfiguration::defaults();
    serverConf.setTcpPort(16365);
    serverConf.setLocalEnabled(false);
    serverConf.setWebsocketEnabled(false);
    serverConf.setRequestTimeout(60000); // the server's own request timer must not fire during the test

    Server server(serverConf, conf);
    QVERIFY(server.listen());

    const QString host = QStringLiteral("qmdmm://localhost:16365");

    // Wire both agents BEFORE connecting: the first SSC request fires as soon as the room fills
    // (p2 joins), and a signal connected after connectToHost would miss it.
    auto *p1 = new Client(ClientConfiguration(), &server);
    connect(p1->agent(), &Agent::stoneScissorsClothRequested, &server, [p1]() { QTimer::singleShot(0, p1->agent(), [p1]() { p1->agent()->stoneScissorsCloth(Data::Stone); }); });

    auto *p2 = new Client(ClientConfiguration(), &server);
    int p2GiveUps = 0;
    connect(p2->agent(), &Agent::stoneScissorsClothRequested, &server, [&p2GiveUps, p2]() {
        ++p2GiveUps;
        p2->agent()->requestTimeout();
    });

    // The observable proof: p1 receives the SSC result -- which only happens if the server
    // applied p2's default reply (rather than dropping the give-up and stalling the logic).
    int sscResults = 0;
    connect(p1->agent(), &Agent::stoneScissorsClothNotified, &server, [&sscResults](const QHash<QString, Data::StoneScissorsCloth> &) { ++sscResults; });

    // p1 joins first (room not full yet); p2 fills the room, kicking off the game + first SSC.
    QVERIFY(p1->connectToHost(host, Data::StateOnlineBot));
    QTRY_VERIFY_WITH_TIMEOUT(p1->room() != nullptr && p1->room()->player(p1->objectName()) != nullptr, 5000);

    QVERIFY(p2->connectToHost(host, Data::StateOnline));

    QTRY_VERIFY_WITH_TIMEOUT(p2GiveUps >= 1, 5000);
    QTRY_VERIFY_WITH_TIMEOUT(sscResults >= 1, 10000);
}

namespace {
RegisterTestObject<tst_QMdmmNetworking> _;
}
#include "tst_qmdmmnetworking.moc"
