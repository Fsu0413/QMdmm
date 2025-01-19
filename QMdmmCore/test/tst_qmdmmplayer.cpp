
#include <QMdmmCore/QMdmmPlayer>
#include <QMdmmCore/QMdmmRoom>

#include <QScopedPointer>
#include <QSignalSpy>
#include <QTest>

class tst_QMdmmPlayer : public QObject
{
    Q_OBJECT

    static QMdmmLogicConfiguration conf;

private:
    // utility functions
    QMdmmRoom *newRoom()
    {
        return new QMdmmRoom(conf, this);
    }

private slots:
    void QMdmmPlayerroom()
    {
        QScopedPointer<QMdmmRoom> r(newRoom());

        QMdmmPlayer *p = r->addPlayer(QStringLiteral("test1"));

        QCOMPARE(p->room(), r.data());
        QCOMPARE(static_cast<const QMdmmPlayer *>(p)->room(), r.data());
    }

    void QMdmmPlayerhasKnife()
    {
        QScopedPointer<QMdmmRoom> r(newRoom());

        QMdmmPlayer *p = r->addPlayer(QStringLiteral("test1"));

        r->prepareForRoundStart();

        // case 1: initial state is has no knife
        {
            QVERIFY(!p->hasKnife());
        }

        r->prepareForRoundStart();

        // case 2: set hasKnife = true -> emit hasKnifeChanged
        {
            QSignalSpy s(p, &QMdmmPlayer::hasKnifeChanged);
            p->setHasKnife(true);

            QCOMPARE(s.length(), 1);
            if (s.length() > 0) {
                QVERIFY(s.first().first().toBool());
                QVERIFY(p->hasKnife());
            }
        }

        r->prepareForRoundStart();

        // case 3: set hasKnife = false -> do not emit hasKnifeChanged
        {
            QSignalSpy s(p, &QMdmmPlayer::hasKnifeChanged);
            p->setHasKnife(false);

            QCOMPARE(s.length(), 0);
            QVERIFY(!p->hasKnife());
        }
    }

    void QMdmmPlayerhasHorse()
    {
        QScopedPointer<QMdmmRoom> r(newRoom());

        QMdmmPlayer *p = r->addPlayer(QStringLiteral("test1"));

        r->prepareForRoundStart();

        // case 1: initial state is has no horse
        {
            QVERIFY(!p->hasHorse());
        }

        r->prepareForRoundStart();

        // case 2: set hasHorse = true -> emit hasHorseChanged
        {
            QSignalSpy s(p, &QMdmmPlayer::hasHorseChanged);
            p->setHasHorse(true);

            QCOMPARE(s.length(), 1);
            if (s.length() > 0) {
                QVERIFY(s.first().first().toBool());
                QVERIFY(p->hasHorse());
            }
        }

        r->prepareForRoundStart();

        // case 3: set hasHorse = false -> do not emit hasHorseChanged
        {
            QSignalSpy s(p, &QMdmmPlayer::hasHorseChanged);
            p->setHasHorse(false);

            QCOMPARE(s.length(), 0);
            QVERIFY(!p->hasHorse());
        }
    }

    void QMdmmPlayerhp()
    {
        QScopedPointer<QMdmmRoom> r(newRoom());

        QMdmmPlayer *p = r->addPlayer(QStringLiteral("test1"));

        r->prepareForRoundStart();

        // case 1: initial state is hp == conf.initialMaxHp() (There is no upgrade for know!)
        {
            QVERIFY(p->hp() == conf.initialMaxHp());
            QVERIFY(p->alive());
            QVERIFY(!p->dead());
        }

        r->prepareForRoundStart();

        // case 2: set hp = conf.initialMaxHp() - 1 (provided conf.initialMaxHp >= 2) -> emit only hpChanged
        {
            QSignalSpy s(p, &QMdmmPlayer::hpChanged);
            QSignalSpy s2(p, &QMdmmPlayer::die);
            bool kills = false;
            p->setHp(conf.initialMaxHp() - 1, &kills);

            QCOMPARE(s.length(), 1);
            QCOMPARE(s2.length(), 0);
            if (s.length() > 0) {
                QCOMPARE(s.first().first().toInt(), conf.initialMaxHp() - 1);
                QCOMPARE(p->hp(), conf.initialMaxHp() - 1);
            }
            if (s2.length() == 0) {
                QVERIFY(!kills);
                QVERIFY(p->alive());
                QVERIFY(!p->dead());
            }
        }

        r->prepareForRoundStart();

        // case 3: set hp = 0 (provided conf.zeroHpAsDead == true) -> emit both hpChanged and die
        // damaging corpse do not trigger die
        {
            QSignalSpy s(p, &QMdmmPlayer::hpChanged);
            QSignalSpy s2(p, &QMdmmPlayer::die);
            bool kills = false;
            p->setHp(0, &kills);

            QCOMPARE(s.length(), 1);
            QCOMPARE(s2.length(), 1);
            if (s.length() > 0) {
                QCOMPARE(s.first().first().toInt(), 0);
                QCOMPARE(p->hp(), 0);
            }
            if (s2.length() > 0) {
                QVERIFY(kills);
                QVERIFY(!p->alive());
                QVERIFY(p->dead());
            }

            bool kills2 = false;
            p->setHp(-1, &kills2);

            QCOMPARE(s.length(), 2);
            QCOMPARE(s2.length(), 1);
            QVERIFY(!kills2);
            QVERIFY(!p->alive());
            QVERIFY(p->dead());
        }
    }
};

QMdmmLogicConfiguration tst_QMdmmPlayer::conf = QMdmmLogicConfiguration::defaults();

QTEST_GUILESS_MAIN(tst_QMdmmPlayer)
#include "tst_qmdmmplayer.moc"
