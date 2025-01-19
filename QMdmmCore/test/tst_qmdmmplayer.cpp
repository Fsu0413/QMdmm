
#include <QMdmmCore/QMdmmPlayer>
#include <QMdmmCore/QMdmmRoom>

#include <QSignalSpy>
#include <QTest>

class tst_QMdmmPlayer : public QObject
{
    Q_OBJECT

    std::unique_ptr<QMdmmRoom> r;
    QMdmmPlayer *p1;
    QMdmmPlayer *p2;

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

        p1 = nullptr;
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
};

QTEST_GUILESS_MAIN(tst_QMdmmPlayer)
#include "tst_qmdmmplayer.moc"
