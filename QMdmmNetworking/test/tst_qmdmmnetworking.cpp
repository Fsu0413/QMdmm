// SPDX-License-Identifier: AGPL-3.0-or-later

#include "test.h"

#include <QMdmmAgent>
#include <QMdmmClient>
#include <QMdmmData>
#include <QMdmmLogicConfiguration>
#include <QMdmmLogicRunner>
#include <QMdmmPacket>
#include <QMdmmProtocol>
#include <QMdmmServer>
#include <QMdmmSocket>

#include <QHostAddress>
#include <QJsonArray>
#include <QJsonObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTest>

// NOLINTBEGIN

using namespace QMdmmCore;
using namespace QMdmmNetworking;

class tst_QMdmmNetworking : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE tst_QMdmmNetworking() = default;

private slots:
    void socketDisconnected_notFull_removesAgent();
    void socketDisconnected_full_marksOffline();
    void reconnect_rebindsSocketAndRestoresState();
    void reconnect_replaysMissedRoundEvents();
    void signIn_reconnectsPlayerInNonCurrentRoom();
    void addSocket_nullSocket_registersLocalAgent();
};

// Build a real loopback TCP connection and wrap the server side in a Socket, so the
// disconnect path can be driven by actually dropping the client end (no private access).
// The QTcpServer and the client socket are children of `parent`; the accepted socket is
// taken over by the returned Socket.
static bool makeServerSocket(QTcpServer **outServer, QTcpSocket **outClient, Socket **outSocket, QObject *parent)
{
    auto *server = new QTcpServer(parent);
    if (!server->listen(QHostAddress::LocalHost, 0))
        return false;

    auto *client = new QTcpSocket(server);
    client->connectToHost(QHostAddress::LocalHost, server->serverPort());
    if (!client->waitForConnected(5000))
        return false;
    if (!server->waitForNewConnection(5000))
        return false;

    QTcpSocket *accepted = server->nextPendingConnection();
    if (accepted == nullptr)
        return false;

    auto *sock = new Socket(accepted, parent);

    *outServer = server;
    *outClient = client;
    *outSocket = sock;
    return true;
}

// A room that is not full has not started a game yet: a dropped socket removes the
// agent entirely (it can be re-added later as a fresh player).
void tst_QMdmmNetworking::socketDisconnected_notFull_removesAgent()
{
    LogicConfiguration conf = LogicConfiguration::defaults();
    conf.setPlayerNumPerRoom(2);
    conf.setRequestTimeout(60000);

    LogicRunner runner(conf);

    QTcpServer *server = nullptr;
    QTcpSocket *client = nullptr;
    Socket *sock = nullptr;
    QVERIFY(makeServerSocket(&server, &client, &sock, &runner));

    Agent *agent = runner.addSocket(QStringLiteral("p1"), QStringLiteral("screen1"), Data::StateOnline, sock);
    QVERIFY(agent != nullptr);
    QVERIFY(!runner.full());

    client->abort();
    QTRY_VERIFY_WITH_TIMEOUT(runner.agent(QStringLiteral("p1")) == nullptr, 5000);
}

// A full room has already started the game: a dropped socket keeps the agent in the
// room but marks it offline (and untrusted), so the seat is preserved for a possible
// reconnect before round over.
void tst_QMdmmNetworking::socketDisconnected_full_marksOffline()
{
    LogicConfiguration conf = LogicConfiguration::defaults();
    conf.setPlayerNumPerRoom(2);
    conf.setRequestTimeout(60000);

    LogicRunner runner(conf);

    QTcpServer *server1 = nullptr;
    QTcpSocket *client1 = nullptr;
    Socket *sock1 = nullptr;
    QVERIFY(makeServerSocket(&server1, &client1, &sock1, &runner));
    QVERIFY(runner.addSocket(QStringLiteral("p1"), QStringLiteral("screen1"), Data::StateOnline, sock1) != nullptr);

    QTcpServer *server2 = nullptr;
    QTcpSocket *client2 = nullptr;
    Socket *sock2 = nullptr;
    QVERIFY(makeServerSocket(&server2, &client2, &sock2, &runner));
    QVERIFY(runner.addSocket(QStringLiteral("p2"), QStringLiteral("screen2"), Data::StateOnlineBot, sock2) != nullptr);

    QVERIFY(runner.full());

    client1->abort();
    QTRY_VERIFY_WITH_TIMEOUT(runner.agent(QStringLiteral("p1")) != nullptr && !runner.agent(QStringLiteral("p1"))->state().testFlag(Data::StateMaskOnline), 5000);

    // p1 stays in the room but is offline; p2 is untouched.
    Agent *stayed = runner.agent(QStringLiteral("p1"));
    QVERIFY(stayed != nullptr);
    QVERIFY(!stayed->state().testFlag(Data::StateMaskOnline));
    QVERIFY(!stayed->state().testFlag(Data::StateMaskTrust));

    Agent *other = runner.agent(QStringLiteral("p2"));
    QVERIFY(other != nullptr);
    QVERIFY(other->state().testFlag(Data::StateMaskOnline));
}

// After a full-room disconnect, reconnect() rebinds a fresh socket and restores the
// online / trust flags (the setSocket rebind + state restore path).
void tst_QMdmmNetworking::reconnect_rebindsSocketAndRestoresState()
{
    LogicConfiguration conf = LogicConfiguration::defaults();
    conf.setPlayerNumPerRoom(2);
    conf.setRequestTimeout(60000);

    LogicRunner runner(conf);

    QTcpServer *server1 = nullptr;
    QTcpSocket *client1 = nullptr;
    Socket *sock1 = nullptr;
    QVERIFY(makeServerSocket(&server1, &client1, &sock1, &runner));
    QVERIFY(runner.addSocket(QStringLiteral("p1"), QStringLiteral("screen1"), Data::StateOnline, sock1) != nullptr);

    QTcpServer *server2 = nullptr;
    QTcpSocket *client2 = nullptr;
    Socket *sock2 = nullptr;
    QVERIFY(makeServerSocket(&server2, &client2, &sock2, &runner));
    QVERIFY(runner.addSocket(QStringLiteral("p2"), QStringLiteral("screen2"), Data::StateOnlineBot, sock2) != nullptr);

    QVERIFY(runner.full());

    client1->abort();
    QTRY_VERIFY_WITH_TIMEOUT(!runner.agent(QStringLiteral("p1"))->state().testFlag(Data::StateMaskOnline), 5000);

    QTcpServer *server1b = nullptr;
    QTcpSocket *client1b = nullptr;
    Socket *sock1b = nullptr;
    QVERIFY(makeServerSocket(&server1b, &client1b, &sock1b, &runner));

    Agent *reconnected = runner.reconnect(QStringLiteral("p1"), sock1b);
    QVERIFY(reconnected != nullptr);
    QCOMPARE(reconnected, runner.agent(QStringLiteral("p1")));
    QVERIFY(reconnected->state().testFlag(Data::StateMaskOnline));
    QVERIFY(reconnected->state().testFlag(Data::StateMaskTrust));
}

// reconnect() must not re-announce the round (the client never left it) -- instead it
// replays, in order, only the cached round events after the index the client reported
// as its received count. This drives the server's notification path through the public
// Agent controller methods (what LogicRunner would call) and observes the wire through
// the reconnected Socket's public sendPacket signal.
void tst_QMdmmNetworking::reconnect_replaysMissedRoundEvents()
{
    LogicConfiguration conf = LogicConfiguration::defaults();
    conf.setPlayerNumPerRoom(2);
    conf.setRequestTimeout(60000);

    LogicRunner runner(conf);

    QTcpServer *server1 = nullptr;
    QTcpSocket *client1 = nullptr;
    Socket *sock1 = nullptr;
    QVERIFY(makeServerSocket(&server1, &client1, &sock1, &runner));
    QVERIFY(runner.addSocket(QStringLiteral("p1"), QStringLiteral("screen1"), Data::StateOnline, sock1) != nullptr);

    QTcpServer *server2 = nullptr;
    QTcpSocket *client2 = nullptr;
    Socket *sock2 = nullptr;
    QVERIFY(makeServerSocket(&server2, &client2, &sock2, &runner));
    QVERIFY(runner.addSocket(QStringLiteral("p2"), QStringLiteral("screen2"), Data::StateOnlineBot, sock2) != nullptr);
    QVERIFY(runner.full());

    // Drop p1 (offline, seat preserved).
    client1->abort();
    QTRY_VERIFY_WITH_TIMEOUT(!runner.agent(QStringLiteral("p1"))->state().testFlag(Data::StateMaskOnline), 5000);

    // While p1 is gone, the logic side broadcasts three round events through p1's
    // Agent (the public controller entry points LogicRunner calls). These are recorded
    // in p1's connection round-event log for later catch-up.
    Agent *agent1 = runner.agent(QStringLiteral("p1"));
    QVERIFY(agent1 != nullptr);

    QHash<QString, Data::StoneScissorsCloth> ssc;
    ssc.insert(QStringLiteral("p1"), Data::Stone);
    ssc.insert(QStringLiteral("p2"), Data::Cloth);
    agent1->notifyStoneScissorsCloth(ssc); // event index 0

    QHash<int, QString> order;
    order.insert(1, QStringLiteral("p1"));
    order.insert(2, QStringLiteral("p2"));
    agent1->notifyActionOrder(order); // event index 1

    agent1->notifyAction(QStringLiteral("p1"), Data::DoNothing, QString(), 0); // event index 2

    // Reconnect reporting it already received 1 round event (index 0): only the 2nd and
    // 3rd events are replayed, in order. Only round events are counted here.
    QTcpServer *server1b = nullptr;
    QTcpSocket *client1b = nullptr;
    Socket *sock1b = nullptr;
    QVERIFY(makeServerSocket(&server1b, &client1b, &sock1b, &runner));

    QList<Protocol::NotifyId> replayedNotifies;
    connect(sock1b, &Socket::sendPacket, [&](const Packet &packet) {
        const Protocol::NotifyId id = packet.notifyId();
        if (id == Protocol::NotifyStoneScissorsCloth || id == Protocol::NotifyActionOrder || id == Protocol::NotifyAction || id == Protocol::NotifyUpgrade) {
            replayedNotifies << id;
        }
    });

    Agent *reconnected = runner.reconnect(QStringLiteral("p1"), sock1b, 1);
    QVERIFY(reconnected != nullptr);
    QVERIFY(reconnected->state().testFlag(Data::StateMaskOnline));
    QVERIFY(reconnected->state().testFlag(Data::StateMaskTrust));

    // The wrong old behavior re-sent notifyGameStart / notifyRoundStart; the precise
    // catch-up must not. Only the missed events (index >= 1) are replayed, in order.
    QCOMPARE(replayedNotifies.size(), 2);
    QVERIFY(replayedNotifies.at(0) == Protocol::NotifyActionOrder);
    QVERIFY(replayedNotifies.at(1) == Protocol::NotifyAction);
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
    conf.setRequestTimeout(60000); // bots stay silent without timing out during the test

    ServerConfiguration serverConf = ServerConfiguration::defaults();
    serverConf.setTcpPort(16366);
    serverConf.setLocalEnabled(false);
    serverConf.setWebsocketEnabled(false);

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

// addSocket with a null socket registers a socket-less "local" agent (operation side =
// GUI / Bot): it joins the room and is reachable through agent(), without creating any
// wire plumbing. A local agent has no socket, so there is nothing to disconnect.
void tst_QMdmmNetworking::addSocket_nullSocket_registersLocalAgent()
{
    LogicConfiguration conf = LogicConfiguration::defaults();
    conf.setPlayerNumPerRoom(3);
    conf.setRequestTimeout(60000);

    LogicRunner runner(conf);

    Agent *local = runner.addSocket(QStringLiteral("p1"), QStringLiteral("screen1"), Data::StateOnline, nullptr);
    QVERIFY(local != nullptr);
    QCOMPARE(runner.agent(QStringLiteral("p1")), local);
    QVERIFY(local->state().testFlag(Data::StateMaskOnline));
    QVERIFY(!runner.full()); // playerNumPerRoom = 3, only one agent added
}

namespace {
RegisterTestObject<tst_QMdmmNetworking> _;
}
#include "tst_qmdmmnetworking.moc"
