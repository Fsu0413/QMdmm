// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmlogicrunner_p.h"
#include "qmdmmserver_p.h"
#include "qmdmmsocket_p.h"

#include "test.h"

#include <QMdmmData>
#include <QMdmmLogicConfiguration>
#include <QMdmmLogicRunner>
#include <QMdmmServer>
#include <QMdmmSocket>

#include <QJsonObject>
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
    void signIn_reconnectsPlayerInNonCurrentRoom();
};

// Build a server-side Socket that wraps a fresh (unconnected) QTcpSocket, and
// hand back its SocketP so the test can drive the disconnect path directly
// without a real transport. The QTcpSocket is owned (via deleteLater) by the
// SocketP, which is itself a child of the Socket.
static QMdmmNetworking::p::SocketP *makeServerSocket(Socket **outSocket, QObject *parent)
{
    auto *raw = new QTcpSocket();
    auto *sock = new Socket(raw, parent);
    if (outSocket != nullptr)
        *outSocket = sock;
    return sock->findChild<QMdmmNetworking::p::SocketP *>();
}

// A room that is not full has not started a game yet: a dropped socket removes
// the agent entirely (it can be re-added later as a fresh player).
void tst_QMdmmNetworking::socketDisconnected_notFull_removesAgent()
{
    LogicConfiguration conf = LogicConfiguration::defaults();
    conf.setPlayerNumPerRoom(2);

    LogicRunner runner(conf);

    Socket *sock = nullptr;
    QMdmmNetworking::p::SocketP *sockP = makeServerSocket(&sock, &runner);
    QVERIFY(sockP != nullptr);

    Agent *agent = runner.addSocket(QStringLiteral("p1"), QStringLiteral("screen1"), Data::StateOnline, sock);
    QVERIFY(agent != nullptr);
    QVERIFY(!runner.full());

    sockP->socketDisconnected();

    QVERIFY(runner.agent(QStringLiteral("p1")) == nullptr);
}

// A full room has already started the game: a dropped socket keeps the agent
// in the room but marks it offline (and untrusted), so the seat is preserved
// for a possible reconnect before round over.
void tst_QMdmmNetworking::socketDisconnected_full_marksOffline()
{
    LogicConfiguration conf = LogicConfiguration::defaults();
    conf.setPlayerNumPerRoom(2);

    LogicRunner runner(conf);

    Socket *sock1 = nullptr;
    QMdmmNetworking::p::SocketP *sockP1 = makeServerSocket(&sock1, &runner);
    QVERIFY(sockP1 != nullptr);
    Agent *agent1 = runner.addSocket(QStringLiteral("p1"), QStringLiteral("screen1"), Data::StateOnline, sock1);
    QVERIFY(agent1 != nullptr);

    Socket *sock2 = nullptr;
    QMdmmNetworking::p::SocketP *sockP2 = makeServerSocket(&sock2, &runner);
    QVERIFY(sockP2 != nullptr);
    QVERIFY(runner.addSocket(QStringLiteral("p2"), QStringLiteral("screen2"), Data::StateOnlineBot, sock2) != nullptr);

    QVERIFY(runner.full());

    sockP1->socketDisconnected();

    // p1 stays in the room but is offline; p2 is untouched.
    Agent *stayed = runner.agent(QStringLiteral("p1"));
    QVERIFY(stayed != nullptr);
    QVERIFY(!stayed->state().testFlag(Data::StateMaskOnline));
    QVERIFY(!stayed->state().testFlag(Data::StateMaskTrust));

    Agent *other = runner.agent(QStringLiteral("p2"));
    QVERIFY(other != nullptr);
    QVERIFY(other->state().testFlag(Data::StateMaskOnline));
}

// After a full-room disconnect, reconnect() rebinds a fresh socket and restores
// the online / trust flags (the setSocket rebind + state restore path).
void tst_QMdmmNetworking::reconnect_rebindsSocketAndRestoresState()
{
    LogicConfiguration conf = LogicConfiguration::defaults();
    conf.setPlayerNumPerRoom(2);

    LogicRunner runner(conf);

    Socket *sock1 = nullptr;
    QMdmmNetworking::p::SocketP *sockP1 = makeServerSocket(&sock1, &runner);
    QVERIFY(sockP1 != nullptr);
    Agent *agent1 = runner.addSocket(QStringLiteral("p1"), QStringLiteral("screen1"), Data::StateOnline, sock1);
    QVERIFY(agent1 != nullptr);

    Socket *sock2 = nullptr;
    QMdmmNetworking::p::SocketP *sockP2 = makeServerSocket(&sock2, &runner);
    QVERIFY(sockP2 != nullptr);
    QVERIFY(runner.addSocket(QStringLiteral("p2"), QStringLiteral("screen2"), Data::StateOnlineBot, sock2) != nullptr);

    QVERIFY(runner.full());

    sockP1->socketDisconnected();
    QVERIFY(!runner.agent(QStringLiteral("p1"))->state().testFlag(Data::StateMaskOnline));

    Socket *sock1b = nullptr;
    QMdmmNetworking::p::SocketP *sockP1b = makeServerSocket(&sock1b, &runner);
    QVERIFY(sockP1b != nullptr);

    Agent *reconnected = runner.reconnect(QStringLiteral("p1"), sock1b);
    QVERIFY(reconnected != nullptr);
    QCOMPARE(reconnected, runner.agent(QStringLiteral("p1")));
    QVERIFY(reconnected->state().testFlag(Data::StateMaskOnline));
    QVERIFY(reconnected->state().testFlag(Data::StateMaskTrust));
}

// Build a ServerP::signIn payload for a player (playerName / screenName / agentState).
static QJsonObject makeSignInValue(const QString &playerName, const QString &screenName, QMdmmCore::Data::AgentState agentState)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerName"), playerName);
    ob.insert(QStringLiteral("screenName"), screenName);
    ob.insert(QStringLiteral("agentState"), static_cast<int>(agentState));
    return ob;
}

// A reconnecting player may live in ANY room, not just `current`. `current` only tracks the room
// that is currently recruiting; a full room keeps running in the background and is deleted only on
// gameOver. So once a later sign-in moves `current` to a fresh room, an offline player in the old
// full room can no longer be found through `current` alone. This is the regression test for the
// findChildren fix: signIn's reconnect path must scan every LogicRunner child for the offline
// agent and reconnect it in whichever room it lives.
void tst_QMdmmNetworking::signIn_reconnectsPlayerInNonCurrentRoom()
{
    LogicConfiguration conf = LogicConfiguration::defaults();
    conf.setPlayerNumPerRoom(2);

    Server server(ServerConfiguration::defaults(), conf);
    QMdmmNetworking::p::ServerP *serverP = server.findChild<QMdmmNetworking::p::ServerP *>();
    QVERIFY(serverP != nullptr);

    // Room 1: p1 (human) + p2 (bot) -> full.
    Socket *sock1 = nullptr;
    QMdmmNetworking::p::SocketP *sockP1 = makeServerSocket(&sock1, serverP);
    QVERIFY(sockP1 != nullptr);
    serverP->signIn(sock1, makeSignInValue(QStringLiteral("p1"), QStringLiteral("screen1"), Data::StateOnline));

    Socket *sock2 = nullptr;
    QMdmmNetworking::p::SocketP *sockP2 = makeServerSocket(&sock2, serverP);
    QVERIFY(sockP2 != nullptr);
    serverP->signIn(sock2, makeSignInValue(QStringLiteral("p2"), QStringLiteral("screen2"), Data::StateOnlineBot));

    LogicRunner *r1 = serverP->current;
    QVERIFY(r1 != nullptr);
    QVERIFY(r1->full());

    // Room 2: p3 (bot) -> `current` moves to a fresh room; r1 keeps running in the background.
    Socket *sock3 = nullptr;
    QMdmmNetworking::p::SocketP *sockP3 = makeServerSocket(&sock3, serverP);
    QVERIFY(sockP3 != nullptr);
    serverP->signIn(sock3, makeSignInValue(QStringLiteral("p3"), QStringLiteral("screen3"), Data::StateOnlineBot));

    LogicRunner *r2 = serverP->current;
    QVERIFY(r2 != nullptr);
    QVERIFY(r2 != r1);

    // Drop p1 (in the non-current full room r1): it goes offline but stays seated.
    sockP1->socketDisconnected();

    Agent *offline = r1->agent(QStringLiteral("p1"));
    QVERIFY(offline != nullptr);
    QVERIFY(!offline->state().testFlag(Data::StateMaskOnline));

    // Re-sign-in p1 with a fresh socket. The reconnect path must locate it in r1 via findChildren
    // (not in the `current` room r2).
    Socket *sock1b = nullptr;
    QMdmmNetworking::p::SocketP *sockP1b = makeServerSocket(&sock1b, serverP);
    QVERIFY(sockP1b != nullptr);
    serverP->signIn(sock1b, makeSignInValue(QStringLiteral("p1"), QStringLiteral("screen1"), Data::StateOnline));

    Agent *reconnected = r1->agent(QStringLiteral("p1"));
    QVERIFY(reconnected != nullptr);
    QVERIFY(reconnected->state().testFlag(Data::StateMaskOnline));
    QVERIFY(reconnected->state().testFlag(Data::StateMaskTrust));

    // p1 must not have been added as a *new* player to the current room r2, and `current` must
    // still point at r2.
    QVERIFY(r2->agent(QStringLiteral("p1")) == nullptr);
    QVERIFY(serverP->current == r2);
}

namespace {
RegisterTestObject<tst_QMdmmNetworking> _;
}
#include "tst_qmdmmnetworking.moc"
