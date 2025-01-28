
#include <QMdmmCore/QMdmmPlayer>
#include <QMdmmCore/QMdmmRoom>

#include <QPointer>
#include <QSignalSpy>
#include <QTest>

class tst_QMdmmPlayer : public QObject
{
    Q_OBJECT

    std::unique_ptr<QMdmmRoom> r;
    QPointer<QMdmmPlayer> p1;
    QPointer<QMdmmPlayer> p2;

private slots:
    // https://doc.qt.io/qt-5/qtest-overview.html#creating-a-test
    // called before each test case is run
    void init()
    {
        r.reset(new QMdmmRoom(QMdmmLogicConfiguration::defaults(), this));
        p1 = r->addPlayer(QStringLiteral("test1"));
        p2 = r->addPlayer(QStringLiteral("test2"));
    }

    void QMdmmPlayerroom()
    {
        QCOMPARE(p1->room(), r.get());
        QCOMPARE(static_cast<const QMdmmPlayer *>(p1)->room(), r.get());
    }

    void QMdmmPlayerhasKnife()
    {
        r->prepareForRoundStart();

        // case 1: initial state is has no knife
        {
            QVERIFY(!p1->hasKnife());
        }

        r->prepareForRoundStart();

        // case 2: set hasKnife = true -> emit hasKnifeChanged
        {
            QSignalSpy s(p1, &QMdmmPlayer::hasKnifeChanged);
            p1->setHasKnife(true);

            QCOMPARE(s.length(), 1);
            if (s.length() > 0) {
                QVERIFY(s.first().first().toBool());
                QVERIFY(p1->hasKnife());
            }
        }

        r->prepareForRoundStart();

        // case 3: set hasKnife = false -> do not emit hasKnifeChanged
        {
            QSignalSpy s(p1, &QMdmmPlayer::hasKnifeChanged);
            p1->setHasKnife(false);

            QCOMPARE(s.length(), 0);
            QVERIFY(!p1->hasKnife());
        }
    }

    void QMdmmPlayerhasHorse()
    {
        r->prepareForRoundStart();

        // case 1: initial state is has no horse
        {
            QVERIFY(!p1->hasHorse());
        }

        r->prepareForRoundStart();

        // case 2: set hasHorse = true -> emit hasHorseChanged
        {
            QSignalSpy s(p1, &QMdmmPlayer::hasHorseChanged);
            p1->setHasHorse(true);

            QCOMPARE(s.length(), 1);
            if (s.length() > 0) {
                QVERIFY(s.first().first().toBool());
                QVERIFY(p1->hasHorse());
            }
        }

        r->prepareForRoundStart();

        // case 3: set hasHorse = false -> do not emit hasHorseChanged
        {
            QSignalSpy s(p1, &QMdmmPlayer::hasHorseChanged);
            p1->setHasHorse(false);

            QCOMPARE(s.length(), 0);
            QVERIFY(!p1->hasHorse());
        }
    }

    // also the test function for dead() and alive(), and signal die()
    void QMdmmPlayerhp()
    {
        r->prepareForRoundStart();

        // case 1: initial state is hp == 10 (provided conf.initialMaxHp == 10, There is no upgrade for now!)
        {
            QVERIFY(p1->hp() == 10);
            QVERIFY(p1->alive());
            QVERIFY(!p1->dead());
        }

        r->prepareForRoundStart();

        // case 2: set hp = 9 -> emit only hpChanged
        {
            QSignalSpy s(p1, &QMdmmPlayer::hpChanged);
            QSignalSpy s2(p1, &QMdmmPlayer::die);
            bool kills = false;
            p1->setHp(9, &kills);

            QCOMPARE(s.length(), 1);
            QCOMPARE(s2.length(), 0);
            if (s.length() > 0) {
                QCOMPARE(s.first().first().toInt(), 9);
                QCOMPARE(p1->hp(), 9);
            }
            if (s2.length() == 0) {
                QVERIFY(!kills);
                QVERIFY(p1->alive());
                QVERIFY(!p1->dead());
            }
        }

        r->prepareForRoundStart();

        // case 3: set hp = 0 (provided conf.zeroHpAsDead == true) -> emit both hpChanged and die
        // damaging corpse do not trigger die
        {
            QSignalSpy s(p1, &QMdmmPlayer::hpChanged);
            QSignalSpy s2(p1, &QMdmmPlayer::die);
            bool kills = false;
            p1->setHp(0, &kills);

            QCOMPARE(s.length(), 1);
            QCOMPARE(s2.length(), 1);
            if (s.length() > 0) {
                QCOMPARE(s.first().first().toInt(), 0);
                QCOMPARE(p1->hp(), 0);
            }
            if (s2.length() > 0) {
                QVERIFY(kills);
                QVERIFY(!p1->alive());
                QVERIFY(p1->dead());
            }

            bool kills2 = false;
            p1->setHp(-1, &kills2);

            QCOMPARE(s.length(), 2);
            QCOMPARE(s2.length(), 1);
            QVERIFY(!kills2);
            QVERIFY(!p1->alive());
            QVERIFY(p1->dead());
        }

        r->prepareForRoundStart();

        // case 4: set hp = 10 -> emit no signal
        {
            QSignalSpy s(p1, &QMdmmPlayer::hpChanged);
            QSignalSpy s2(p1, &QMdmmPlayer::die);
            bool kills = false;
            p1->setHp(10, &kills);

            QCOMPARE(s.length(), 0);
            QCOMPARE(s2.length(), 0);
            QCOMPARE(p1->hp(), 10);
            QVERIFY(!kills);
            QVERIFY(p1->alive());
            QVERIFY(!p1->dead());
        }

        QMdmmLogicConfiguration conf = QMdmmLogicConfiguration::defaults();
        conf.setZeroHpAsDead(false);

        r.reset(new QMdmmRoom(conf, this));

        p1 = r->addPlayer(QStringLiteral("test1"));
        r->prepareForRoundStart();

        // case 5: zeroHpAsDead = false
        {
            QSignalSpy s(p1, &QMdmmPlayer::hpChanged);
            QSignalSpy s2(p1, &QMdmmPlayer::die);
            bool kills = false;
            p1->setHp(0, &kills);

            QCOMPARE(s.length(), 1);
            QCOMPARE(s2.length(), 0);
            if (s.length() > 0) {
                QCOMPARE(s.first().first().toInt(), 0);
                QCOMPARE(p1->hp(), 0);
            }
            if (s2.length() == 0) {
                QVERIFY(!kills);
                QVERIFY(p1->alive());
                QVERIFY(!p1->dead());
            }

            s.clear();
            s2.clear();

            bool kills2 = false;
            p1->setHp(-1, &kills2);

            QCOMPARE(s.length(), 1);
            QCOMPARE(s2.length(), 1);
            if (s.length() > 0) {
                QCOMPARE(s.first().first().toInt(), -1);
                QCOMPARE(p1->hp(), -1);
            }
            if (s2.length() > 0) {
                QVERIFY(kills2);
                QVERIFY(!p1->alive());
                QVERIFY(p1->dead());
            }

            s.clear();
            s2.clear();

            bool kills3 = false;
            p1->setHp(-2, &kills3);

            QCOMPARE(s.length(), 1);
            QCOMPARE(s2.length(), 0);
            QVERIFY(!kills3);
            QVERIFY(!p1->alive());
            QVERIFY(p1->dead());
        }
    }

    void QMdmmPlayerplace()
    {
        r->prepareForRoundStart();

        // case 1: initial state is at place 1
        {
            QCOMPARE(p1->place(), 1);
        }

        r->prepareForRoundStart();

        // case 2: set place = 0 -> emit placeChanged
        {
            QSignalSpy s(p1, &QMdmmPlayer::placeChanged);
            p1->setPlace(QMdmmData::Country);

            QCOMPARE(s.length(), 1);
            if (s.length() > 0) {
                QCOMPARE(s.first().first().toInt(), QMdmmData::Country);
                QCOMPARE(p1->place(), QMdmmData::Country);
            }
        }

        r->prepareForRoundStart();

        // case 3: set place = 1 -> do not emit placeChanged
        {
            QSignalSpy s(p1, &QMdmmPlayer::placeChanged);
            p1->setPlace(1);

            QCOMPARE(s.length(), 0);
            QCOMPARE(p1->place(), 1);
        }
    }

    void QMdmmPlayerinitialPlace()
    {
        r->prepareForRoundStart();

        // case 1: initial state is at initialPlace 1
        {
            QCOMPARE(p1->initialPlace(), 1);
        }

        r->prepareForRoundStart();

        // case 2: set initialPlace = 0 -> emit initialPlaceChanged
        {
            QSignalSpy s(p1, &QMdmmPlayer::initialPlaceChanged);
            p1->setInitialPlace(QMdmmData::Country);

            QCOMPARE(s.length(), 1);
            if (s.length() > 0) {
                QCOMPARE(s.first().first().toInt(), QMdmmData::Country);
                QCOMPARE(p1->initialPlace(), QMdmmData::Country);
            }
        }

        r->prepareForRoundStart();

        // case 3: set initialPlace = 1 -> do not emit initialPlaceChanged
        {
            QSignalSpy s(p1, &QMdmmPlayer::initialPlaceChanged);
            p1->setInitialPlace(1);

            QCOMPARE(s.length(), 0);
            QCOMPARE(p1->initialPlace(), 1);
        }
    }

    void QMdmmPlayerknifeDamage()
    {
        r->resetUpgrades();

        // case 1: initial state is at knifeDamage 1 (provided conf.initialKnifeDamage = 1, There is no upgrade for now!)
        {
            QCOMPARE(p1->knifeDamage(), 1);
        }

        r->resetUpgrades();

        // case 2: set knifeDamage = 2 -> emit knifeDamageChanged
        {
            QSignalSpy s(p1, &QMdmmPlayer::knifeDamageChanged);
            p1->setKnifeDamage(2);

            QCOMPARE(s.length(), 1);
            if (s.length() > 0) {
                QCOMPARE(s.first().first().toInt(), 2);
                QCOMPARE(p1->knifeDamage(), 2);
            }
        }

        r->resetUpgrades();

        // case 3: set knifeDamage = 1 -> do not emit knifeDamageChanged
        {
            QSignalSpy s(p1, &QMdmmPlayer::knifeDamageChanged);
            p1->setKnifeDamage(1);

            QCOMPARE(s.length(), 0);
            QCOMPARE(p1->knifeDamage(), 1);
        }
    }

    void QMdmmPlayerhorseDamage()
    {
        r->resetUpgrades();

        // case 1: initial state is at horseDamage 2 (provided conf.initialHorseDamage = 2, There is no upgrade for now!)
        {
            QCOMPARE(p1->horseDamage(), 2);
        }

        r->resetUpgrades();

        // case 2: set horseDamage = 3 -> emit horseDamageChanged
        {
            QSignalSpy s(p1, &QMdmmPlayer::horseDamageChanged);
            p1->setHorseDamage(3);

            QCOMPARE(s.length(), 1);
            if (s.length() > 0) {
                QCOMPARE(s.first().first().toInt(), 3);
                QCOMPARE(p1->horseDamage(), 3);
            }
        }

        r->resetUpgrades();

        // case 3: set horseDamage = 2 -> do not emit horseDamageChanged
        {
            QSignalSpy s(p1, &QMdmmPlayer::horseDamageChanged);
            p1->setHorseDamage(2);

            QCOMPARE(s.length(), 0);
            QCOMPARE(p1->horseDamage(), 2);
        }
    }

    void QMdmmPlayermaxHp()
    {
        r->resetUpgrades();

        // case 1: initial state is at maxHp 10 (provided conf.initialMaxHp = 10, There is no upgrade for now!)
        {
            QCOMPARE(p1->maxHp(), 10);
        }

        r->resetUpgrades();

        // case 2: set maxHp = 11 -> emit maxHpChanged
        {
            QSignalSpy s(p1, &QMdmmPlayer::maxHpChanged);
            p1->setMaxHp(11);

            QCOMPARE(s.length(), 1);
            if (s.length() > 0) {
                QCOMPARE(s.first().first().toInt(), 11);
                QCOMPARE(p1->maxHp(), 11);
            }
        }

        r->resetUpgrades();

        // case 3: set maxHp = 10 -> do not emit maxHpChanged
        {
            QSignalSpy s(p1, &QMdmmPlayer::maxHpChanged);
            p1->setMaxHp(10);

            QCOMPARE(s.length(), 0);
            QCOMPARE(p1->maxHp(), 10);
        }
    }

    void QMdmmPlayerupgradePoint()
    {
        r->resetUpgrades();

        // case 1: initial state is at upgradePoint 0 (There is no upgrade for now!)
        {
            QCOMPARE(p1->maxHp(), 10);
        }

        r->resetUpgrades();

        // case 2: set upgradePoint = 1 -> emit upgradePointChanged
        {
            QSignalSpy s(p1, &QMdmmPlayer::upgradePointChanged);
            p1->setUpgradePoint(1);

            QCOMPARE(s.length(), 1);
            if (s.length() > 0) {
                QCOMPARE(s.first().first().toInt(), 1);
                QCOMPARE(p1->upgradePoint(), 1);
            }
        }

        r->resetUpgrades();

        // case 3: set upgradePoint = 0 -> do not emit upgradePointChanged
        {
            QSignalSpy s(p1, &QMdmmPlayer::upgradePointChanged);
            p1->setUpgradePoint(0);

            QCOMPARE(s.length(), 0);
            QCOMPARE(p1->upgradePoint(), 0);
        }
    }

    void QMdmmPlayercanBuyKnife()
    {
        r->prepareForRoundStart();

        // case 1: initial state - can buy knife in any city
        {
            QVERIFY(p1->canBuyKnife());
            p1->setPlace(p2->place());
            QVERIFY(p1->canBuyKnife());
        }

        r->prepareForRoundStart();

        // case 2: dead player - cannot buy knife
        {
            p1->setHp(-1);
            QVERIFY(!p1->canBuyKnife());
        }

        r->prepareForRoundStart();

        // case 3: has knife - cannot buy knife
        {
            p1->setHasKnife(true);
            QVERIFY(!p1->canBuyKnife());
        }

        r->prepareForRoundStart();

        // case 4: in country - cannot buy knife
        {
            p1->setPlace(QMdmmData::Country);
            QVERIFY(!p1->canBuyKnife());
        }

        QMdmmLogicConfiguration conf = QMdmmLogicConfiguration::defaults();
        conf.setCanBuyOnlyInInitialCity(true);

        r.reset(new QMdmmRoom(conf, this));

        p1 = r->addPlayer(QStringLiteral("test1"));
        p2 = r->addPlayer(QStringLiteral("test2"));
        r->prepareForRoundStart();

        // case 5: canBuyOnlyInInitialCity = true
        {
            p1->setPlace(p2->place());
            QVERIFY(!p1->canBuyKnife());
        }
    }

    void QMdmmPlayercanBuyHorse()
    {
        r->prepareForRoundStart();

        // case 1: initial state - can buy knife in any city
        {
            QVERIFY(p1->canBuyHorse());
            p1->setPlace(p2->place());
            QVERIFY(p1->canBuyHorse());
        }

        r->prepareForRoundStart();

        // case 2: dead player - cannot buy knife
        {
            p1->setHp(-1);
            QVERIFY(!p1->canBuyHorse());
        }

        r->prepareForRoundStart();

        // case 3: has knife - cannot buy knife
        {
            p1->setHasHorse(true);
            QVERIFY(!p1->canBuyHorse());
        }

        r->prepareForRoundStart();

        // case 4: in country - cannot buy knife
        {
            p1->setPlace(QMdmmData::Country);
            QVERIFY(!p1->canBuyHorse());
        }

        QMdmmLogicConfiguration conf = QMdmmLogicConfiguration::defaults();
        conf.setCanBuyOnlyInInitialCity(true);

        r.reset(new QMdmmRoom(conf, this));

        p1 = r->addPlayer(QStringLiteral("test1"));
        p2 = r->addPlayer(QStringLiteral("test2"));
        r->prepareForRoundStart();

        // case 5: canBuyOnlyInInitialCity = true
        {
            p1->setPlace(p2->place());
            QVERIFY(!p1->canBuyHorse());
        }
    }

    void QMdmmPlayercanSlash()
    {
        r->prepareForRoundStart();

        // case 1: can't slash if place is different
        {
            p1->setHasKnife(true);

            QVERIFY(!p1->canSlash(p2));
        }

        r->prepareForRoundStart();

        // case 2: can't slash without knife
        {
            p1->setPlace(p2->place());

            QVERIFY(!p1->canSlash(p2));
        }

        r->prepareForRoundStart();

        // case 3: dead player can't slash, can't slash dead player
        {
            p1->setHasKnife(true);
            p1->setPlace(p2->place());
            p2->setHp(-1);

            QVERIFY(!p1->canSlash(p2));
            QVERIFY(!p2->canSlash(p1));
        }

        r->prepareForRoundStart();

        // case 4: can't slash himself / herself
        {
            p1->setHasKnife(true);

            QVERIFY(!p1->canSlash(p1));
        }

        r->prepareForRoundStart();

        // case 5: can slash with knife, both healthy state, and same place
        {
            p1->setHasKnife(true);
            p1->setPlace(p2->place());

            QVERIFY(p1->canSlash(p2));
        }
    }

    void QMdmmPlayercanKick()
    {
        r->prepareForRoundStart();

        // case 1: can't kick if place is different
        {
            p1->setHasHorse(true);

            QVERIFY(!p1->canKick(p2));
        }

        r->prepareForRoundStart();

        // case 2: can't kick without horse
        {
            p1->setPlace(p2->place());

            QVERIFY(!p1->canKick(p2));
        }

        r->prepareForRoundStart();

        // case 3: dead player can't kick, can't kick dead player
        {
            p1->setHasHorse(true);
            p1->setPlace(p2->place());
            p2->setHp(-1);

            QVERIFY(!p1->canKick(p2));
            QVERIFY(!p2->canKick(p1));
        }

        r->prepareForRoundStart();

        // case 4: can't kick himself / herself
        {
            p1->setHasHorse(true);

            QVERIFY(!p1->canKick(p1));
        }

        r->prepareForRoundStart();

        // case 5: can kick with horse, both healthy state, and same place in city
        {
            p1->setHasHorse(true);
            p1->setPlace(p2->place());

            QVERIFY(p1->canKick(p2));
        }

        r->prepareForRoundStart();

        // case 6: can't kick in country
        {
            p1->setHasHorse(true);
            p1->setPlace(QMdmmData::Country);
            p2->setPlace(QMdmmData::Country);

            QVERIFY(!p1->canKick(p2));
        }
    }

    void QMdmmPlayercanMove()
    {
        r->prepareForRoundStart();

        // case 1: can move to country when in city
        {
            QVERIFY(p1->canMove(QMdmmData::Country));
        }

        r->prepareForRoundStart();

        // case 2: can't move to another city when in a city
        {
            QVERIFY(!p1->canMove(p2->place()));
        }

        r->prepareForRoundStart();

        // case 3: can move to any city when in a country
        {
            p1->setPlace(QMdmmData::Country);

            QVERIFY(p1->canMove(p1->initialPlace()));
            QVERIFY(p1->canMove(p2->place()));
        }

        r->prepareForRoundStart();

        // case 4: dead player can't move
        {
            p1->setHp(-1);
            QVERIFY(!p1->canMove(QMdmmData::Country));
        }

        r->prepareForRoundStart();

        // case 5: can't move to current place
        {
            QVERIFY(!p1->canMove(p1->place()));
        }
    }

    void QMdmmPlayercanLetMove()
    {
        r->prepareForRoundStart();

        // coverage for this == to
        {
            QVERIFY(p1->canLetMove(p1, QMdmmData::Country));
        }

        r->prepareForRoundStart();

        // case 1: dead players can't be push / pull to / from
        {
            p1->setPlace(p2->place());
            p2->setHp(-1);

            QVERIFY(!p1->canLetMove(p2, QMdmmData::Country));
            QVERIFY(!p2->canLetMove(p1, QMdmmData::Country));
        }

        r->prepareForRoundStart();

        // case 2: push stuff - if p1 and p2 place is same
        {
            p1->setPlace(p2->place());

            QVERIFY(p1->canLetMove(p2, QMdmmData::Country));
            QVERIFY(!p1->canLetMove(p2, p2->initialPlace()));
            QVERIFY(!p1->canLetMove(p2, p1->initialPlace()));
        }

        r->prepareForRoundStart();

        // case 3: pull stuff - if p1 and p2 place is adjacent
        {
            p2->setPlace(QMdmmData::Country);
            QVERIFY(p1->canLetMove(p2, p1->place()));
            QVERIFY(!p1->canLetMove(p2, QMdmmData::Country));
            QVERIFY(!p1->canLetMove(p2, p2->initialPlace()));
        }

        r->prepareForRoundStart();

        // case 4: ??? - if p1 and p2 place is not same nor adjacent
        {
            QVERIFY(!p1->canLetMove(p2, p1->place()));
            QVERIFY(!p1->canLetMove(p2, QMdmmData::Country));
            QVERIFY(!p1->canLetMove(p2, p2->initialPlace()));
        }

        QMdmmLogicConfiguration conf = QMdmmLogicConfiguration::defaults();
        conf.setEnableLetMove(false);

        r.reset(new QMdmmRoom(conf, this));

        p1 = r->addPlayer(QStringLiteral("test1"));
        p2 = r->addPlayer(QStringLiteral("test2"));
        r->prepareForRoundStart();

        // case 5: disabled let move
        {
            p1->setPlace(p2->place());

            QVERIFY(!p1->canLetMove(p2, QMdmmData::Country));
        }
    }
};

QTEST_GUILESS_MAIN(tst_QMdmmPlayer)
#include "tst_qmdmmplayer.moc"
