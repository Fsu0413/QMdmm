#include "test.h"

#include <QMdmmCore/QMdmmPlayer>
#include <QMdmmCore/QMdmmRoom>

#include <QPointer>
#include <QSignalSpy>
#include <QTest>

#include <memory>

using namespace Qt::StringLiterals;

// NOLINTBEGIN
// Exempt from clang-tidy by policy; see AGENTS.md.

using namespace QMdmmCore;

class tst_QMdmmRoom : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE tst_QMdmmRoom() = default;

    std::unique_ptr<Room> r;

private slots:
    // called before each test case is run
    void init()
    {
        r.reset(new Room(LogicConfiguration::defaults()));
    }

    // called after each test case is run
    void cleanup()
    {
        r.reset();
    }

    void QMdmmRoomlogicConfiguration()
    {
        // ctor'ed configuration should be kept as-is
        QCOMPARE(QJsonObject(r->logicConfiguration()), QJsonObject(LogicConfiguration::defaults()));

        LogicConfiguration c = LogicConfiguration::v1();
        r->setLogicConfiguration(c);
        QCOMPARE(QJsonObject(r->logicConfiguration()), QJsonObject(c));

        // configuration is taken by value, so it affects players immediately
        Player *p = r->addPlayer(u"p1"_s);
        QCOMPARE(p->upgradeMaxHpRemainingTimes(), c.maximumMaxHp() - p->maxHp());
    }

    void QMdmmRoomaddPlayer()
    {
        QSignalSpy spy(r.get(), &Room::playerAdded);

        Player *p1 = r->addPlayer(u"p1"_s);
        QVERIFY(p1 != nullptr);
        QCOMPARE(p1->objectName(), u"p1"_s);
        QCOMPARE(p1->room(), r.get());
        QCOMPARE(spy.length(), 1);
        QCOMPARE(spy.first().first().toString(), u"p1"_s);

        // duplicated name is rejected, and no signal is emitted
        Player *dup = r->addPlayer(u"p1"_s);
        QCOMPARE(dup, nullptr);
        QCOMPARE(spy.length(), 1);

        Player *p2 = r->addPlayer(u"p2"_s);
        QVERIFY(p2 != nullptr);
        QVERIFY(p2 != p1);
        QCOMPARE(spy.length(), 2);
        QCOMPARE(spy.at(1).first().toString(), u"p2"_s);
    }

    void QMdmmRoomremovePlayer()
    {
        QPointer<Player> p1 = r->addPlayer(u"p1"_s);
        r->addPlayer(u"p2"_s);

        QSignalSpy spy(r.get(), &Room::playerRemoved);

        QVERIFY(r->removePlayer(u"p1"_s));
        QCOMPARE(spy.length(), 1);
        QCOMPARE(spy.first().first().toString(), u"p1"_s);

        // the removed player is deleted
        QVERIFY(p1.isNull());
        QCOMPARE(r->player(u"p1"_s), nullptr);
        QCOMPARE(r->playerNames(), QStringList {u"p2"_s});

        // removing a nonexistent player fails, and no signal is emitted
        QVERIFY(!r->removePlayer(u"p1"_s));
        QVERIFY(!r->removePlayer(u"nonexist"_s));
        QCOMPARE(spy.length(), 1);
    }

    void QMdmmRoomplayer()
    {
        Player *p1 = r->addPlayer(u"p1"_s);

        QCOMPARE(r->player(u"p1"_s), p1);
        QCOMPARE(r->player(u"nonexist"_s), nullptr);

        const Room *cr = r.get();
        QCOMPARE(cr->player(u"p1"_s), p1);
        QCOMPARE(cr->player(u"nonexist"_s), nullptr);
    }

    void QMdmmRoomplayers()
    {
        QVERIFY(r->players().isEmpty());
        QVERIFY(r->playerNames().isEmpty());

        Player *p1 = r->addPlayer(u"p1"_s);
        Player *p2 = r->addPlayer(u"p2"_s);

        // players are stored in a name-keyed map, so the order is sorted by name
        QCOMPARE(r->players(), (QList<Player *> {p1, p2}));
        QCOMPARE(r->playerNames(), (QStringList {u"p1"_s, u"p2"_s}));

        const Room *cr = r.get();
        QCOMPARE(cr->players(), (QList<const Player *> {p1, p2}));
    }

    void QMdmmRoomalivePlayers()
    {
        Player *p1 = r->addPlayer(u"p1"_s);
        Player *p2 = r->addPlayer(u"p2"_s);
        Player *p3 = r->addPlayer(u"p3"_s);
        r->prepareForRoundStart();

        const Room *cr = r.get();

        QCOMPARE(r->alivePlayers(), (QList<Player *> {p1, p2, p3}));
        QCOMPARE(cr->alivePlayers(), (QList<const Player *> {p1, p2, p3}));
        QCOMPARE(r->alivePlayerNames(), (QStringList {u"p1"_s, u"p2"_s, u"p3"_s}));
        QCOMPARE(r->alivePlayersCount(), 3);
        QVERIFY(!r->isRoundOver());

        // defaults() treats zero HP as dead
        QVERIFY(r->logicConfiguration().zeroHpAsDead());
        p2->setHp(0);
        QVERIFY(p2->dead());

        QCOMPARE(r->alivePlayers(), (QList<Player *> {p1, p3}));
        QCOMPARE(cr->alivePlayers(), (QList<const Player *> {p1, p3}));
        QCOMPARE(r->alivePlayerNames(), (QStringList {u"p1"_s, u"p3"_s}));
        QCOMPARE(r->alivePlayersCount(), 2);
        QVERIFY(!r->isRoundOver());

        // round is over when only one (or zero) player is alive
        p3->setHp(-1);
        QCOMPARE(r->alivePlayersCount(), 1);
        QVERIFY(r->isRoundOver());

        p1->setHp(-1);
        QCOMPARE(r->alivePlayersCount(), 0);
        QVERIFY(r->alivePlayers().isEmpty());
        QVERIFY(r->alivePlayerNames().isEmpty());
        QVERIFY(r->isRoundOver());
    }

    void QMdmmRoomisGameOver()
    {
        Player *p1 = r->addPlayer(u"p1"_s);
        r->addPlayer(u"p2"_s);
        r->resetUpgrades();

        const LogicConfiguration &c = r->logicConfiguration();

        QStringList winners {u"dirty"_s};
        QVERIFY(!r->isGameOver(&winners));
        // the out param is cleared even when game is not over
        QVERIFY(winners.isEmpty());

        // nullptr as out param is allowed
        QVERIFY(!r->isGameOver());

        // a player who has maxed out every upgrade wins
        p1->setKnifeDamage(c.maximumKnifeDamage());
        QVERIFY(!r->isGameOver(&winners));
        QVERIFY(winners.isEmpty());

        p1->setHorseDamage(c.maximumHorseDamage());
        QVERIFY(!r->isGameOver(&winners));
        QVERIFY(winners.isEmpty());

        p1->setMaxHp(c.maximumMaxHp());
        QVERIFY(r->isGameOver(&winners));
        QCOMPARE(winners, QStringList {u"p1"_s});

        QVERIFY(r->isGameOver());

        // multiple winners are all reported
        Player *p2 = r->player(u"p2"_s);
        p2->setKnifeDamage(c.maximumKnifeDamage());
        p2->setHorseDamage(c.maximumHorseDamage());
        p2->setMaxHp(c.maximumMaxHp());
        QVERIFY(r->isGameOver(&winners));
        QCOMPARE(winners, (QStringList {u"p1"_s, u"p2"_s}));
    }

    void QMdmmRoomprepareForRoundStart()
    {
        Player *p1 = r->addPlayer(u"p1"_s);
        Player *p2 = r->addPlayer(u"p2"_s);
        Player *p3 = r->addPlayer(u"p3"_s);

        foreach (Player *p, r->players()) {
            p->setHasKnife(true);
            p->setHasHorse(true);
            p->setHp(1);
            p->setUpgradePoint(3);
        }

        r->prepareForRoundStart();

        // seats are assigned in map (i.e. name) order, starting from 1
        const QList<Player *> expected {p1, p2, p3};
        for (int i = 0; i < expected.size(); ++i) {
            Player *p = expected.at(i);
            QCOMPARE(p->initialPlace(), i + 1);
            QCOMPARE(p->place(), i + 1);
            QCOMPARE(p->hp(), p->maxHp());
            QVERIFY(!p->hasKnife());
            QVERIFY(!p->hasHorse());
            QCOMPARE(p->upgradePoint(), 0);
            QVERIFY(p->alive());
        }
    }

    void QMdmmRoomresetUpgrades()
    {
        const LogicConfiguration &c = r->logicConfiguration();

        Player *p1 = r->addPlayer(u"p1"_s);
        Player *p2 = r->addPlayer(u"p2"_s);

        foreach (Player *p, r->players()) {
            p->setKnifeDamage(c.maximumKnifeDamage());
            p->setHorseDamage(c.maximumHorseDamage());
            p->setMaxHp(c.maximumMaxHp());
            p->setUpgradePoint(5);
        }

        r->resetUpgrades();

        foreach (Player *p, (QList<Player *> {p1, p2})) {
            QCOMPARE(p->knifeDamage(), c.initialKnifeDamage());
            QCOMPARE(p->horseDamage(), c.initialHorseDamage());
            QCOMPARE(p->maxHp(), c.initialMaxHp());
            QCOMPARE(p->upgradePoint(), 0);
        }

        QVERIFY(!r->isGameOver());
    }
};

namespace {
RegisterTestObject<tst_QMdmmRoom> _b;
} // namespace
#include "tst_qmdmmroom.moc"

// NOLINTEND
