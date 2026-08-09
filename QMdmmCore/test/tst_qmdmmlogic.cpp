#include "test.h"

#include <QMdmmCore/QMdmmLogic>
#include <QMdmmCore/QMdmmLogicConfiguration>
#include <QMdmmPlayer>

#include "qmdmmlogic_p.h"

#include <QSignalSpy>
#include <QTest>

// NOLINTBEGIN

using namespace QMdmmCore;

class tst_QMdmmLogic : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE tst_QMdmmLogic() = default;

    std::unique_ptr<Logic> l;

private slots:
    // https://doc.qt.io/qt-5/qtest-overview.html#creating-a-test
    // called before each test case is run

    void init()
    {
        l.reset(new Logic(LogicConfiguration::defaults(), this));
        l->addPlayer(QStringLiteral("test1"));
        l->addPlayer(QStringLiteral("test2"));
        l->addPlayer(QStringLiteral("test3"));
    }

    void QMdmmLogicstate()
    {
        QCOMPARE(l->state(), Logic::BeforeRoundStart);
    }

    void QMdmmLogicaddPlayer()
    {
        // case 1
        {
            bool r = l->addPlayer(QStringLiteral("test11"));
            QVERIFY(r);
        }

        // case 2
        {
            l->d->state = Logic::SscForAction;
            bool r = l->addPlayer(QStringLiteral("test12"));
            QVERIFY(!r);
        }

        // case 3
        {
            l->d->state = Logic::BeforeRoundStart;
            bool r = l->addPlayer(QStringLiteral("test11"));
            QVERIFY(!r);
        }

        // case 4: d->players.contains(playerName) returns false, d->room->addPlayer(playerName) returns nullptr
        // This is Q_UNREACHABLE so do not test
    }

    void QMdmmLogicremovePlayer()
    {
        // case 1
        {
            bool r = l->removePlayer(QStringLiteral("test1"));
            QVERIFY(r);
        }

        // case 2
        {
            l->d->state = Logic::SscForAction;
            bool r = l->removePlayer(QStringLiteral("test2"));
            QVERIFY(!r);
        }

        // case 3
        {
            l->d->state = Logic::BeforeRoundStart;
            bool r = l->removePlayer(QStringLiteral("test1"));
            QVERIFY(!r);
        }

        // case 4: d->players.contains(playerName) returns true, d->room->removePlayer(playerName) returns false
        // This is Q_UNREACHABLE so do not test
    }

    void QMdmmLogicroundStart()
    {
        // case 1
        {
            bool r = l->roundStart();
            QVERIFY(r);
        }

        // case 2
        {
            bool r = l->roundStart();
            QVERIFY(!r);
        }

        init();
        l->removePlayer(QStringLiteral("test1"));
        l->removePlayer(QStringLiteral("test2"));
        l->removePlayer(QStringLiteral("test3"));

        // case 3
        {
            bool r = l->roundStart();
            QVERIFY(!r);
        }
    }

    void QMdmmLogicsscReply()
    {
        // preparation
        {
            l->roundStart();
        }

        // case 0
        {
            QSignalSpy s(l.get(), &Logic::sscResult);

            bool r = l->sscReply(QStringLiteral("test00"), Data::Stone);
            QVERIFY(!r);

            QCOMPARE(s.length(), 0);
        }

        {
            init();
            l->roundStart();
        }

        // case 1
        {
            QSignalSpy s(l.get(), &Logic::sscResult);

            bool r = l->sscReply(QStringLiteral("test1"), Data::Stone);
            QVERIFY(r);

            r = l->sscReply(QStringLiteral("test1"), Data::Scissors);
            QVERIFY(!r);

            QCOMPARE(s.length(), 0);
        }

        {
            init();
            l->roundStart();
        }

        // case 2
        {
            QSignalSpy s(l.get(), &Logic::sscResult);
            QSignalSpy q(l.get(), &Logic::requestSscForAction);

            bool r1 = l->sscReply(QStringLiteral("test1"), Data::Stone);
            QVERIFY(r1);
            bool r2 = l->sscReply(QStringLiteral("test2"), Data::Stone);
            QVERIFY(r2);
            bool r3 = l->sscReply(QStringLiteral("test3"), Data::Stone);
            QVERIFY(r3);

            QCOMPARE(s.length(), 1);
            QCOMPARE(q.length(), 1);
        }
    }

    // Drive the SSC -> action-order -> action -> upgrade loop so that the GUI,
    // once wired to this engine, is not bitten by a backend bug.
    //
    // Note: SSC choices must NOT be a Stone/Scissors/Cloth cycle -- that yields
    // no winner and the engine restarts SSC forever. Two Stones vs one Scissors
    // gives exactly two winners, which is what exercises the action-order phase.
    // The request* signals are emitted synchronously inside the reply calls, so
    // the spies must be connected *before* the triggering call.
    void QMdmmLogicactionOrderReply()
    {
        l->roundStart(); // init() already added test1/test2/test3

        QSignalSpy req(l.get(), &Logic::requestActionOrder);
        QSignalSpy act(l.get(), &Logic::requestAction);

        l->sscReply(QStringLiteral("test1"), Data::Stone);
        l->sscReply(QStringLiteral("test2"), Data::Stone);
        l->sscReply(QStringLiteral("test3"), Data::Scissors);

        // requestActionOrder is emitted synchronously inside the 3rd sscReply().
        QVERIFY(req.count() > 0);

        bool r = l->actionOrderReply(QStringLiteral("test1"), {1});
        QVERIFY(r);
        r = l->actionOrderReply(QStringLiteral("test2"), {2});
        QVERIFY(r);

        // After both winners pick an order the engine enters the Action phase and
        // emits requestAction for the order-1 player, again synchronously.
        QVERIFY(act.count() > 0);
    }

    void QMdmmLogicactionReply()
    {
        l->roundStart();

        QSignalSpy act(l.get(), &Logic::requestAction);

        l->sscReply(QStringLiteral("test1"), Data::Stone);
        l->sscReply(QStringLiteral("test2"), Data::Stone);
        l->sscReply(QStringLiteral("test3"), Data::Scissors);

        QVERIFY(l->actionOrderReply(QStringLiteral("test1"), {1}));
        QVERIFY(l->actionOrderReply(QStringLiteral("test2"), {2}));

        QVERIFY(act.count() > 0);

        // The order-1 player (test1) is the one requested to act.
        bool r = l->actionReply(QStringLiteral("test1"), Data::DoNothing, {}, 0);
        QVERIFY(r);
    }

    void QMdmmLogicupgradeReply()
    {
        l->roundStart();

        // Negative contract: outside the Upgrade state the reply is rejected.
        QVERIFY(!l->upgradeReply(QStringLiteral("test1"), {Data::UpgradeMaxHp}));

        // Positive contract: upgrades happen at round end, when <= 1 player is
        // alive. Simulate that (kill the other two) and give test1 an upgrade
        // point, then verify the reply is accepted and upgradeResult emitted.
        l->d->room->player(QStringLiteral("test2"))->setHp(0);
        l->d->room->player(QStringLiteral("test3"))->setHp(0);
        l->d->room->player(QStringLiteral("test1"))->setUpgradePoint(1);
        l->d->state = Logic::Upgrade;

        QSignalSpy up(l.get(), &Logic::upgradeResult);
        QVERIFY(l->upgradeReply(QStringLiteral("test1"), {Data::UpgradeMaxHp}));
        QVERIFY(up.count() > 0);
    }
};

namespace {
RegisterTestObject<tst_QMdmmLogic> _;
}
#include "tst_qmdmmlogic.moc"
