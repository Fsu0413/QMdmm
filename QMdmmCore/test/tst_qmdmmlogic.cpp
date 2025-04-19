#include "test.h"

#include <QMdmmCore/QMdmmLogic>
#include <QMdmmCore/QMdmmLogicConfiguration>

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

        QSignalSpy s(l.get(), &Logic::sscResult);

        // case 0
        {
            bool r = l->sscReply(QStringLiteral("test00"), Data::Stone);
            QVERIFY(!r);

            QCOMPARE(s.length(), 0);
        }

        {
            init();
            l->roundStart();
            s.clear();
        }

        // case 1
        {
            bool r = l->sscReply(QStringLiteral("test1"), Data::Stone);
            QVERIFY(r);

            r = l->sscReply(QStringLiteral("test1"), Data::Scissors);
            QVERIFY(!r);

            QCOMPARE(s.length(), 0);
        }

        {
            init();
            l->roundStart();
            s.clear();
        }

        // case 2
        {
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
};

namespace {
RegisterTestObject<tst_QMdmmLogic> _;
}
#include "tst_qmdmmlogic.moc"
