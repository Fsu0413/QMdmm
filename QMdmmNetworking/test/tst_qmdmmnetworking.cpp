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

#include <QJsonArray>
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
    void reconnect_replaysMissedRoundEvents();
    void signIn_reconnectsPlayerInNonCurrentRoom();
    void roundEventCache_recordsAndClearsEvents();
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

// reconnect() must not re-announce the round (the client never left it) -- instead it replays,
// in order, only the cached round events with a sequence number higher than what the client
// reported as its last received one. This is the server half of the "precise catch-up" on
// reconnect.
void tst_QMdmmNetworking::reconnect_replaysMissedRoundEvents()
{
    LogicConfiguration conf = LogicConfiguration::defaults();
    conf.setPlayerNumPerRoom(2);

    LogicRunner runner(conf);

    Socket *sock1 = nullptr;
    QMdmmNetworking::p::SocketP *sockP1 = makeServerSocket(&sock1, &runner);
    QVERIFY(sockP1 != nullptr);
    QVERIFY(runner.addSocket(QStringLiteral("p1"), QStringLiteral("screen1"), Data::StateOnline, sock1) != nullptr);

    Socket *sock2 = nullptr;
    QMdmmNetworking::p::SocketP *sockP2 = makeServerSocket(&sock2, &runner);
    QVERIFY(sockP2 != nullptr);
    QVERIFY(runner.addSocket(QStringLiteral("p2"), QStringLiteral("screen2"), Data::StateOnlineBot, sock2) != nullptr);
    QVERIFY(runner.full());

    // Drop p1 (offline, seat preserved).
    sockP1->socketDisconnected();
    QVERIFY(!runner.agent(QStringLiteral("p1"))->state().testFlag(Data::StateMaskOnline));

    QMdmmNetworking::p::LogicRunnerP *runnerP = runner.findChild<QMdmmNetworking::p::LogicRunnerP *>();
    QVERIFY(runnerP != nullptr);
    QMdmmNetworking::p::ServerAgentP *agent1p = runnerP->agents.value(QStringLiteral("p1"));
    QVERIFY(agent1p != nullptr);

    // Three round events broadcast while p1 was gone (drive the cache directly, as the
    // roundEventCache_recordsAndClearsEvents test does).
    QHash<QString, Data::StoneScissorsCloth> ssc;
    ssc.insert(QStringLiteral("p1"), Data::Stone);
    ssc.insert(QStringLiteral("p2"), Data::Cloth);
    runnerP->sscResult(ssc); // seq 1

    QHash<int, QString> order;
    order.insert(1, QStringLiteral("p1"));
    order.insert(2, QStringLiteral("p2"));
    runnerP->actionOrderResult(order); // seq 2

    runnerP->actionResult(QStringLiteral("p1"), Data::DoNothing, QString(), 0); // seq 3

    // Observe what gets sent to p1's rebound socket on reconnect (connected only now, so the
    // broadcasts made while p1 was offline above are not counted).
    QList<QMdmmCore::Protocol::NotifyId> allNotifies;
    QList<QMdmmCore::Protocol::NotifyId> replayedNotifies;
    QList<int> replayedSeqs;
    connect(agent1p, &QMdmmNetworking::p::ServerAgentP::sendPacket, [&](const QMdmmCore::Packet &packet) {
        const QMdmmCore::Protocol::NotifyId id = packet.notifyId();
        allNotifies << id;
        if (id == Protocol::NotifyStoneScissorsCloth || id == Protocol::NotifyActionOrder || id == Protocol::NotifyAction || id == Protocol::NotifyUpgrade) {
            replayedNotifies << id;
            replayedSeqs << packet.value().toObject().value(QStringLiteral("seq")).toInt();
        }
    });

    // Reconnect with last received seq = 1: only seq 2 and seq 3 are replayed, in order.
    Socket *sock1b = nullptr;
    QMdmmNetworking::p::SocketP *sockP1b = makeServerSocket(&sock1b, &runner);
    QVERIFY(sockP1b != nullptr);

    Agent *reconnected = runner.reconnect(QStringLiteral("p1"), sock1b, 1);
    QVERIFY(reconnected != nullptr);
    QVERIFY(reconnected->state().testFlag(Data::StateMaskOnline));
    QVERIFY(reconnected->state().testFlag(Data::StateMaskTrust));

    // The wrong old behavior re-sent notifyGameStart / notifyRoundStart; the precise catch-up must not.
    QVERIFY(!allNotifies.contains(Protocol::NotifyGameStart));
    QVERIFY(!allNotifies.contains(Protocol::NotifyRoundStart));

    // Only the missed events (seq > 1) are replayed, in ascending sequence order.
    QCOMPARE(replayedNotifies.size(), 2);
    QVERIFY(replayedNotifies.at(0) == Protocol::NotifyActionOrder);
    QCOMPARE(replayedSeqs.at(0), 2);
    QVERIFY(replayedNotifies.at(1) == Protocol::NotifyAction);
    QCOMPARE(replayedSeqs.at(1), 3);
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

// The round-event cache records each round event (ssc / action-order / action / upgrade) keyed by
// its round-event sequence number, and is cleared at every round boundary (roundOver, and the
// new-round start inside upgradeResult). This is the server-side half of the "precise catch-up"
// replay that a reconnecting client will request.
void tst_QMdmmNetworking::roundEventCache_recordsAndClearsEvents()
{
    LogicConfiguration conf = LogicConfiguration::defaults();
    conf.setPlayerNumPerRoom(3); // 2 agents stay below capacity, so addSocket does not start a game

    LogicRunner runner(conf);

    Socket *sock1 = nullptr;
    QMdmmNetworking::p::SocketP *sockP1 = makeServerSocket(&sock1, &runner);
    QVERIFY(sockP1 != nullptr);
    QVERIFY(runner.addSocket(QStringLiteral("p1"), QStringLiteral("screen1"), Data::StateOnline, sock1) != nullptr);

    Socket *sock2 = nullptr;
    QMdmmNetworking::p::SocketP *sockP2 = makeServerSocket(&sock2, &runner);
    QVERIFY(sockP2 != nullptr);
    QVERIFY(runner.addSocket(QStringLiteral("p2"), QStringLiteral("screen2"), Data::StateOnline, sock2) != nullptr);
    QVERIFY(!runner.full());

    QMdmmNetworking::p::LogicRunnerP *runnerP = runner.findChild<QMdmmNetworking::p::LogicRunnerP *>();
    QVERIFY(runnerP != nullptr);
    QVERIFY(runnerP->roundEventCache.isEmpty());

    // ssc (seq 1) and action-order (seq 2) each append their packet to the cache.
    QHash<QString, Data::StoneScissorsCloth> ssc;
    ssc.insert(QStringLiteral("p1"), Data::Stone);
    ssc.insert(QStringLiteral("p2"), Data::Cloth);
    runnerP->sscResult(ssc);
    QCOMPARE(runnerP->roundEventSeq, 1);
    QVERIFY(runnerP->roundEventCache.size() == 1);
    QVERIFY(runnerP->roundEventCache.value(1).notifyId() == Protocol::NotifyStoneScissorsCloth);

    QHash<int, QString> order;
    order.insert(1, QStringLiteral("p1"));
    order.insert(2, QStringLiteral("p2"));
    runnerP->actionOrderResult(order);
    QCOMPARE(runnerP->roundEventSeq, 2);
    QVERIFY(runnerP->roundEventCache.size() == 2);
    QVERIFY(runnerP->roundEventCache.value(2).notifyId() == Protocol::NotifyActionOrder);
    // Regression: the order array must carry every player (off-by-one dropped the last one).
    const QJsonArray orderArr = runnerP->roundEventCache.value(2).value().toObject().value(QStringLiteral("order")).toArray();
    QVERIFY(orderArr.size() == order.size());

    // roundOver drops the current round's events.
    runnerP->roundOver();
    QVERIFY(runnerP->roundEventCache.isEmpty());

    // upgradeResult caches the upgrade event, then the new-round start resets both the cache and seq.
    QHash<QString, QList<Data::UpgradeItem>> upgrades;
    QList<Data::UpgradeItem> items;
    items << Data::UpgradeMaxHp;
    upgrades.insert(QStringLiteral("p1"), items);
    runnerP->upgradeResult(upgrades);
    QCOMPARE(runnerP->roundEventSeq, 0);
    QVERIFY(runnerP->roundEventCache.isEmpty());
}

namespace {
RegisterTestObject<tst_QMdmmNetworking> _;
}
#include "tst_qmdmmnetworking.moc"
