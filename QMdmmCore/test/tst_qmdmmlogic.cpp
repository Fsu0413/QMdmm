#include "test.h"

#include <QMdmmCore/QMdmmLogic>
#include <QMdmmCore/QMdmmLogicConfiguration>

#include "qmdmmlogic_p.h"

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
    }

    void QMdmmLogicstate()
    {
        QCOMPARE(l->state(), Logic::BeforeRoundStart);
    }

    void QMdmmLogicaddPlayer()
    {
        // case 1
        {
            bool r = l->addPlayer(QStringLiteral("test1"));
            QVERIFY(r);
        }

        // case 2
        {
            l->d->state = Logic::SscForAction;
            bool r = l->addPlayer(QStringLiteral("test2"));
            QVERIFY(!r);
        }

        // case 3
        {
            l->d->state = Logic::BeforeRoundStart;
            bool r = l->addPlayer(QStringLiteral("test1"));
            QVERIFY(!r);
        }

        // case 4: d->players.contains(playerName) returns false, d->room->addPlayer(playerName) returns nullptr
        // This is Q_UNREACHABLE so do not test
    }

    void QMdmmLogicremovePlayer()
    {
        // preparation
        {
            l->addPlayer(QStringLiteral("test1"));
            l->addPlayer(QStringLiteral("test2"));
            l->addPlayer(QStringLiteral("test3"));
        }

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
        // preparation
        {
            l->addPlayer(QStringLiteral("test1"));
            l->addPlayer(QStringLiteral("test2"));
            l->addPlayer(QStringLiteral("test3"));
        }

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

        // case 3
        {
            bool r = l->roundStart();
            QVERIFY(!r);
        }
    }
};

namespace {
RegisterTestObject<tst_QMdmmLogic> _;
}
#include "tst_qmdmmlogic.moc"
