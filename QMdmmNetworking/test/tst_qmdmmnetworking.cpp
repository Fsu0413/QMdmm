// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmlogicrunner_p.h"
#include "qmdmmsocket_p.h"

#include "test.h"

#include <QMdmmData>
#include <QMdmmLogicConfiguration>
#include <QMdmmLogicRunner>
#include <QMdmmSocket>

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

namespace {
RegisterTestObject<tst_QMdmmNetworking> _;
}
#include "tst_qmdmmnetworking.moc"
