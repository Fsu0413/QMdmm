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

    // All winners yield (0 sentinel): leftover orders are assigned automatically
    // and the engine advances to Action.
    void QMdmmLogicactionOrderAllYield()
    {
        l->roundStart();

        QSignalSpy ord(l.get(), &Logic::actionOrderResult);
        QSignalSpy act(l.get(), &Logic::requestAction);

        l->sscReply(QStringLiteral("test1"), Data::Stone);
        l->sscReply(QStringLiteral("test2"), Data::Stone);
        l->sscReply(QStringLiteral("test3"), Data::Scissors);

        QVERIFY(l->actionOrderReply(QStringLiteral("test1"), {0}));
        QVERIFY(l->actionOrderReply(QStringLiteral("test2"), {0}));

        QVERIFY(ord.count() > 0);
        QVERIFY(act.count() > 0);
        QCOMPARE(l->d->confirmedActionOrders.value(1), QStringLiteral("test1"));
        QCOMPARE(l->d->confirmedActionOrders.value(2), QStringLiteral("test2"));
    }

    // Partial yield: one winner yields, the other picks. The picker keeps its
    // order, the yielder receives the leftover one.
    void QMdmmLogicactionOrderPartialYield()
    {
        l->roundStart();

        QSignalSpy ord(l.get(), &Logic::actionOrderResult);

        l->sscReply(QStringLiteral("test1"), Data::Stone);
        l->sscReply(QStringLiteral("test2"), Data::Stone);
        l->sscReply(QStringLiteral("test3"), Data::Scissors);

        QVERIFY(l->actionOrderReply(QStringLiteral("test1"), {0}));
        QVERIFY(l->actionOrderReply(QStringLiteral("test2"), {1}));

        QVERIFY(ord.count() > 0);
        QCOMPARE(l->d->confirmedActionOrders.value(1), QStringLiteral("test2"));
        QCOMPARE(l->d->confirmedActionOrders.value(2), QStringLiteral("test1"));
    }

    // Yield by times: a player with two action opportunities can yield one and
    // pick the other (D-024 "yield per opportunity").
    void QMdmmLogicactionOrderYieldByTimes()
    {
        l->addPlayer(QStringLiteral("test4"));
        l->roundStart();

        QSignalSpy ord(l.get(), &Logic::actionOrderResult);

        // test1/test2 win twice each (two losers), orders 1..4.
        l->sscReply(QStringLiteral("test1"), Data::Stone);
        l->sscReply(QStringLiteral("test2"), Data::Stone);
        l->sscReply(QStringLiteral("test3"), Data::Scissors);
        l->sscReply(QStringLiteral("test4"), Data::Scissors);

        // test1 yields once and picks order 1; test2 picks orders 2 and 3.
        QVERIFY(l->actionOrderReply(QStringLiteral("test1"), {0, 1}));
        QVERIFY(l->actionOrderReply(QStringLiteral("test2"), {2, 3}));

        QVERIFY(ord.count() > 0);
        QCOMPARE(l->d->confirmedActionOrders.value(1), QStringLiteral("test1"));
        QCOMPARE(l->d->confirmedActionOrders.value(2), QStringLiteral("test2"));
        QCOMPARE(l->d->confirmedActionOrders.value(3), QStringLiteral("test2"));
        QCOMPARE(l->d->confirmedActionOrders.value(4), QStringLiteral("test1"));
    }

    // Yield plus conflict: a yielder sits out while two others fight over the
    // same order; the conflict loser and the yielder get the leftover orders.
    void QMdmmLogicactionOrderYieldWithConflict()
    {
        l->addPlayer(QStringLiteral("test4"));
        l->roundStart();

        QSignalSpy tie(l.get(), &Logic::requestSscForActionOrder);
        QSignalSpy act(l.get(), &Logic::requestAction);

        // Three winners (test1/test2/test3), one loser -> orders 1..3.
        l->sscReply(QStringLiteral("test1"), Data::Stone);
        l->sscReply(QStringLiteral("test2"), Data::Stone);
        l->sscReply(QStringLiteral("test3"), Data::Stone);
        l->sscReply(QStringLiteral("test4"), Data::Scissors);

        QVERIFY(l->actionOrderReply(QStringLiteral("test1"), {0}));
        QVERIFY(l->actionOrderReply(QStringLiteral("test2"), {1}));
        QVERIFY(l->actionOrderReply(QStringLiteral("test3"), {1}));

        QVERIFY(tie.count() > 0);

        // test2 wins the tie-break SSC (Cloth beats Stone).
        l->sscReply(QStringLiteral("test2"), Data::Cloth);
        l->sscReply(QStringLiteral("test3"), Data::Stone);

        QVERIFY(act.count() > 0);
        QCOMPARE(l->d->confirmedActionOrders.value(1), QStringLiteral("test2"));
        QCOMPARE(l->d->confirmedActionOrders.value(2), QStringLiteral("test1"));
        QCOMPARE(l->d->confirmedActionOrders.value(3), QStringLiteral("test3"));
    }

    // Reject invalid replies: out-of-range order, wrong length, unknown player,
    // duplicated order, and a second reply from an already-answered player.
    void QMdmmLogicactionOrderReplyValidation()
    {
        l->roundStart();

        l->sscReply(QStringLiteral("test1"), Data::Stone);
        l->sscReply(QStringLiteral("test2"), Data::Stone);
        l->sscReply(QStringLiteral("test3"), Data::Scissors);

        // Out-of-range order (maximumOrderNum == 2).
        QVERIFY(!l->actionOrderReply(QStringLiteral("test1"), {3}));
        // Wrong length (selections == 1).
        QVERIFY(!l->actionOrderReply(QStringLiteral("test1"), {1, 2}));
        // Unknown player.
        QVERIFY(!l->actionOrderReply(QStringLiteral("ghost"), {0}));
        // None of the above advanced the state.
        QCOMPARE(l->state(), Logic::ActionOrder);

        // Valid pick, then a duplicated reply is rejected.
        QVERIFY(l->actionOrderReply(QStringLiteral("test1"), {1}));
        QVERIFY(!l->actionOrderReply(QStringLiteral("test1"), {1}));
    }

    // A player with two opportunities must not pick the same order twice.
    void QMdmmLogicactionOrderReplyDuplicateOrder()
    {
        l->addPlayer(QStringLiteral("test4"));
        l->roundStart();

        l->sscReply(QStringLiteral("test1"), Data::Stone);
        l->sscReply(QStringLiteral("test2"), Data::Stone);
        l->sscReply(QStringLiteral("test3"), Data::Scissors);
        l->sscReply(QStringLiteral("test4"), Data::Scissors);

        // Duplicated order within a single reply (selections == 2).
        QVERIFY(!l->actionOrderReply(QStringLiteral("test1"), {1, 1}));
        QCOMPARE(l->state(), Logic::ActionOrder);
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

    // --- Gap coverage: branches the first pass of tests never exercised ---

    // A. SSC tie (no winner) must restart SSC instead of advancing.
    void QMdmmLogicsscTieRestarts()
    {
        QSignalSpy req(l.get(), &Logic::requestSscForAction);
        QSignalSpy res(l.get(), &Logic::sscResult);

        l->roundStart();

        // All three pick Stone -> a tie, no winner.
        l->sscReply(QStringLiteral("test1"), Data::Stone);
        l->sscReply(QStringLiteral("test2"), Data::Stone);
        l->sscReply(QStringLiteral("test3"), Data::Stone);

        // requestSscForAction fired once at roundStart and once more on the restart.
        QCOMPARE(req.length(), 2);
        // sscResult is emitted (reports the tie) but the engine does NOT advance.
        QCOMPARE(res.length(), 1);
        QCOMPARE(l->state(), Logic::SscForAction);

        // The restarted round is still playable: a winning combo now advances.
        QSignalSpy ord(l.get(), &Logic::requestActionOrder);
        l->sscReply(QStringLiteral("test1"), Data::Stone);
        l->sscReply(QStringLiteral("test2"), Data::Stone);
        l->sscReply(QStringLiteral("test3"), Data::Scissors);
        QVERIFY(ord.count() > 0);
    }

    // B. Exactly one SSC winner takes every action order (no ActionOrder phase).
    void QMdmmLogicsingleWinnerNoActionOrder()
    {
        l->roundStart();

        QSignalSpy ord(l.get(), &Logic::requestActionOrder);
        QSignalSpy act(l.get(), &Logic::requestAction);

        // test1=Stone beats test2=test3=Scissors; the two Scissors tie among themselves.
        l->sscReply(QStringLiteral("test1"), Data::Stone);
        l->sscReply(QStringLiteral("test2"), Data::Scissors);
        l->sscReply(QStringLiteral("test3"), Data::Scissors);

        // Single winner -> no action-order negotiation, straight to Action.
        QCOMPARE(ord.length(), 0);
        QVERIFY(act.count() > 0);
    }

    // C. Two winners picking the SAME order must fight a tie-break SSC (SscForActionOrder).
    void QMdmmLogicactionOrderTieBreak()
    {
        l->roundStart();

        QSignalSpy reqOrder(l.get(), &Logic::requestActionOrder);
        QSignalSpy reqTie(l.get(), &Logic::requestSscForActionOrder);
        QSignalSpy act(l.get(), &Logic::requestAction);

        l->sscReply(QStringLiteral("test1"), Data::Stone);
        l->sscReply(QStringLiteral("test2"), Data::Stone);
        l->sscReply(QStringLiteral("test3"), Data::Scissors);

        QVERIFY(reqOrder.count() > 0);

        // Both winners demand order 1 -> engine asks them to break the tie with SSC.
        l->actionOrderReply(QStringLiteral("test1"), {1});
        l->actionOrderReply(QStringLiteral("test2"), {1});

        QVERIFY(reqTie.count() > 0);

        // Non-tie SSC resolves the struggle (Cloth beats Stone here); Action follows.
        l->sscReply(QStringLiteral("test1"), Data::Stone);
        l->sscReply(QStringLiteral("test2"), Data::Cloth);

        QVERIFY(act.count() > 0);
    }

    // D. actionOrderReply must reject unknown players and wrong states.
    void QMdmmLogicactionOrderReplyNegative()
    {
        l->roundStart();

        // unknown player, even once we are in the right state
        {
            l->sscReply(QStringLiteral("test1"), Data::Stone);
            l->sscReply(QStringLiteral("test2"), Data::Stone);
            l->sscReply(QStringLiteral("test3"), Data::Scissors);

            QVERIFY(!l->actionOrderReply(QStringLiteral("ghost"), {1}));
        }

        // wrong state: actionOrderReply before any SSC negotiation
        {
            init();
            l->roundStart();
            QVERIFY(!l->actionOrderReply(QStringLiteral("test1"), {1}));
        }
    }

    // E. actionReply must reject an infeasible action (no knife -> cannot Slash).
    void QMdmmLogicactionReplyInfeasible()
    {
        l->roundStart();

        QSignalSpy res(l.get(), &Logic::actionResult);

        l->sscReply(QStringLiteral("test1"), Data::Stone);
        l->sscReply(QStringLiteral("test2"), Data::Stone);
        l->sscReply(QStringLiteral("test3"), Data::Scissors);

        QVERIFY(l->actionOrderReply(QStringLiteral("test1"), {1}));
        QVERIFY(l->actionOrderReply(QStringLiteral("test2"), {2}));

        // test1 has no knife, so Slash is infeasible -> rejected, no actionResult.
        QVERIFY(!l->actionReply(QStringLiteral("test1"), Data::Slash, QStringLiteral("test2"), 0));
        QCOMPARE(res.length(), 0);
    }

    // F. A feasible Slash is applied, can kill, and triggers roundOver.
    void QMdmmLogicactionReplyKillsAndRoundOver()
    {
        l->roundStart();

        // Put attacker and victim in the same place (Country == 0) and arm the attacker.
        l->d->room->player(QStringLiteral("test1"))->setHasKnife(true);
        l->d->room->player(QStringLiteral("test1"))->setPlace(0);
        l->d->room->player(QStringLiteral("test2"))->setPlace(0);
        // Victim one hit from death.
        l->d->room->player(QStringLiteral("test2"))->setHp(1);

        QSignalSpy res(l.get(), &Logic::actionResult);
        QSignalSpy over(l.get(), &Logic::roundOver);

        // All three alive during SSC -> two winners (Stones) -> action-order phase.
        l->sscReply(QStringLiteral("test1"), Data::Stone);
        l->sscReply(QStringLiteral("test2"), Data::Stone);
        l->sscReply(QStringLiteral("test3"), Data::Scissors);

        QVERIFY(l->actionOrderReply(QStringLiteral("test1"), {1}));
        QVERIFY(l->actionOrderReply(QStringLiteral("test2"), {2}));

        // Kill the third player now, so the round ends the moment the victim dies.
        l->d->room->player(QStringLiteral("test3"))->setHp(0);

        QVERIFY(l->actionReply(QStringLiteral("test1"), Data::Slash, QStringLiteral("test2"), 0));

        QVERIFY(res.count() > 0);
        QVERIFY(over.count() > 0);
        QVERIFY(l->d->room->player(QStringLiteral("test2"))->dead());
    }

    // G. When a fully-maxed player exists and the round is over, upgrade ends the game.
    void QMdmmLogicupgradeGameOver()
    {
        l->roundStart();

        // End the round: only test1 survives.
        l->d->room->player(QStringLiteral("test2"))->setHp(0);
        l->d->room->player(QStringLiteral("test3"))->setHp(0);

        // test1 is fully upgraded (max HP / knife / horse) but still earns a point.
        Player *p = l->d->room->player(QStringLiteral("test1"));
        p->setMaxHp(20);
        p->setKnifeDamage(10);
        p->setHorseDamage(10);
        p->setUpgradePoint(1);
        l->d->state = Logic::Upgrade;

        QSignalSpy gameOver(l.get(), &Logic::gameOver);
        QSignalSpy up(l.get(), &Logic::upgradeResult);

        // Reply with no items: nothing to apply, but the game is already over.
        QVERIFY(l->upgradeReply(QStringLiteral("test1"), {}));
        QVERIFY(gameOver.count() > 0);
        QCOMPARE(up.length(), 0);
    }

    // H. upgradeReply actually applies the chosen upgrade items.
    void QMdmmLogicupgradeAppliesItems()
    {
        l->roundStart();

        l->d->room->player(QStringLiteral("test2"))->setHp(0);
        l->d->room->player(QStringLiteral("test3"))->setHp(0);
        Player *p = l->d->room->player(QStringLiteral("test1"));
        const int beforeMaxHp = p->maxHp();
        p->setUpgradePoint(1);
        l->d->state = Logic::Upgrade;

        QSignalSpy up(l.get(), &Logic::upgradeResult);
        QVERIFY(l->upgradeReply(QStringLiteral("test1"), {Data::UpgradeMaxHp}));
        QVERIFY(up.count() > 0);
        QCOMPARE(p->maxHp(), beforeMaxHp + 1);
    }

    // I. actionReply must reject unknown players and wrong states (guards that
    //    return false before any feasibility check).
    void QMdmmLogicactionReplyNegative()
    {
        l->roundStart();

        // Wrong state: still negotiating SSC/action-order, not yet in Action.
        {
            QSignalSpy res(l.get(), &Logic::actionResult);
            QVERIFY(!l->actionReply(QStringLiteral("test1"), Data::DoNothing, {}, 0));
            QCOMPARE(res.length(), 0);
        }

        // Unknown player while in the Action state.
        {
            l->sscReply(QStringLiteral("test1"), Data::Stone);
            l->sscReply(QStringLiteral("test2"), Data::Stone);
            l->sscReply(QStringLiteral("test3"), Data::Scissors);
            QVERIFY(l->actionOrderReply(QStringLiteral("test1"), {1}));
            QVERIFY(l->actionOrderReply(QStringLiteral("test2"), {2}));

            QSignalSpy res(l.get(), &Logic::actionResult);
            QVERIFY(!l->actionReply(QStringLiteral("ghost"), Data::DoNothing, {}, 0));
            QCOMPARE(res.length(), 0);
        }
    }

    // J. upgradeReply must reject an unknown player even in the Upgrade state.
    void QMdmmLogicupgradeReplyNegative()
    {
        l->d->room->player(QStringLiteral("test2"))->setHp(0);
        l->d->room->player(QStringLiteral("test3"))->setHp(0);
        l->d->room->player(QStringLiteral("test1"))->setUpgradePoint(1);
        l->d->state = Logic::Upgrade;

        QSignalSpy up(l.get(), &Logic::upgradeResult);
        QVERIFY(!l->upgradeReply(QStringLiteral("ghost"), {Data::UpgradeMaxHp}));
        QCOMPARE(up.length(), 0);
    }
};

namespace {
RegisterTestObject<tst_QMdmmLogic> _;
}
#include "tst_qmdmmlogic.moc"
