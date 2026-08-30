#include "qmdmmcoreglobal.h"
#include "test.h"

#include <QMdmmCore/QMdmmCoreGlobal>

#include <QHash>
#include <QString>
#include <QTest>

// NOLINTBEGIN

using namespace QMdmmCore;

class tst_QMdmmCore : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE tst_QMdmmCore() = default;

private slots:
    void QMdmmDataisPlaceAdjacent_data()
    {
        QTest::addColumn<int>("p1");
        QTest::addColumn<int>("p2");
        QTest::addColumn<bool>("result");

        QTest::newRow("nc-nc") << 1 << 2 << false;
        QTest::newRow("c-c") << (int)Data::Country << (int)Data::Country << false;
        QTest::newRow("nc-c") << 1 << (int)Data::Country << true;
        QTest::newRow("c-nc") << (int)Data::Country << 2 << true;
    }
    void QMdmmDataisPlaceAdjacent()
    {
        QFETCH(int, p1);
        QFETCH(int, p2);
        QFETCH(bool, result);

        bool r = Data::isPlaceAdjacent(p1, p2);
        QCOMPARE(r, result);
    }

    void QMdmmDatarockPaperScissorsWinners_data()
    {
        typedef QHash<QString, Data::RockPaperScissors> JudgeHash;

        QTest::addColumn<JudgeHash>("judgers");
        QTest::addColumn<QStringList>("result");

        QTest::newRow("tie-allsame") << JudgeHash {
            std::make_pair(QStringLiteral("1"), Data::Rock),
            std::make_pair(QStringLiteral("2"), Data::Rock),
            std::make_pair(QStringLiteral("3"), Data::Rock),
            std::make_pair(QStringLiteral("4"), Data::Rock),
        } << QStringList {};
        QTest::newRow("tie-alldiff") << JudgeHash {
            std::make_pair(QStringLiteral("1"), Data::Rock),
            std::make_pair(QStringLiteral("2"), Data::Scissors),
            std::make_pair(QStringLiteral("3"), Data::Paper),
            std::make_pair(QStringLiteral("4"), Data::Rock),
        } << QStringList {};
        QTest::newRow("rock-vs-scissors") << JudgeHash {
            std::make_pair(QStringLiteral("1"), Data::Rock),
            std::make_pair(QStringLiteral("2"), Data::Scissors),
            std::make_pair(QStringLiteral("3"), Data::Rock),
            std::make_pair(QStringLiteral("4"), Data::Scissors),
        } << QStringList {
            QStringLiteral("1"),
            QStringLiteral("1"),
            QStringLiteral("3"),
            QStringLiteral("3"),
        };
        QTest::newRow("paper-vs-rock") << JudgeHash {
            std::make_pair(QStringLiteral("1"), Data::Rock),
            std::make_pair(QStringLiteral("2"), Data::Paper),
            std::make_pair(QStringLiteral("3"), Data::Paper),
            std::make_pair(QStringLiteral("4"), Data::Paper),
        } << QStringList {
            QStringLiteral("2"),
            QStringLiteral("3"),
            QStringLiteral("4"),
        };
    }
    void QMdmmDatarockPaperScissorsWinners()
    {
        typedef QHash<QString, Data::RockPaperScissors> JudgeHash;

        QFETCH(JudgeHash, judgers);
        QFETCH(QStringList, result);

        QStringList r = Data::rockPaperScissorsWinners(judgers);
        foreach (const QString &a, result)
            r.removeOne(a);

        QCOMPARE(r, QStringList {});
    }

    void QMdmmGlobalversion()
    {
        QVersionNumber r = Global::version();
        QCOMPARE(r, QVersionNumber::fromString(QStringLiteral(QMDMM_VERSION)));
    }

    void QMdmmUtilitieslist2Set()
    {
        const int l1[] = {1, 2, 3, 4, 5, 2, 3, 4, 5, 6};
        QSet<int> s {1, 2, 3, 4, 5, 6};

        QSet<int> r = Utilities::list2Set(l1);
        QCOMPARE(r, s);

        std::list<int> l2(std::begin(l1), std::end(l1));
        r = Utilities::list2Set(l2);
        QCOMPARE(r, s);

        QList<int> l3(std::begin(l1), std::end(l1));
        r = Utilities::list2Set(l3);
        QCOMPARE(r, s);

        std::array l4 = std::to_array(l1);
        r = Utilities::list2Set(l4);
        QCOMPARE(r, s);

        r = Utilities::list2Set<std::initializer_list<int>>({1, 2, 3, 4, 5, 2, 3, 4, 5, 6});
        QCOMPARE(r, s);
    }

    void QMdmmUtilitiesenumList2VariantList1()
    {
        QList<Data::RockPaperScissors> l {Data::Rock, Data::Scissors, Data::Paper, Data::Rock};
        QVariantList s {static_cast<int>(Data::Rock), static_cast<int>(Data::Scissors), static_cast<int>(Data::Paper), static_cast<int>(Data::Rock)};

        QVariantList r = Utilities::enumList2VariantList(l);
        QCOMPARE(r, s);
    }

    void QMdmmUtilitiesenumList2VariantList2()
    {
        QList<Data::AgentState> l {Data::StateOnlineBot, Data::StateOnlineTrust, Data::StateOffline};
        QVariantList s {static_cast<int>(Data::AgentState(Data::StateOnlineBot)), static_cast<int>(Data::AgentState(Data::StateOnlineTrust)),
                        static_cast<int>(Data::AgentState(Data::StateOffline))};

        QVariantList r = Utilities::enumList2VariantList(l);
        QCOMPARE(r, s);
    }

    void QMdmmUtilitiesintList2VariantList()
    {
        QList<int> l {1, 2, 3, 4, 5, 6};
        QVariantList s {1, 2, 3, 4, 5, 6};

        QVariantList r = Utilities::intList2VariantList(l);
        QCOMPARE(r, s);
    }

    void QMdmmUtilitiesvariantList2IntList()
    {
        QVariantList l {6, 7, 8, 9, 10, 11};
        QList<int> s {6, 7, 8, 9, 10, 11};

        QList<int> r = Utilities::variantList2IntList(l);
        QCOMPARE(r, s);
    }

    void QMdmmUtilitiesstringList2VariantList()
    {
        QStringList l {QStringLiteral("Fs"), QStringLiteral("u"), QStringLiteral("0413")};
        QVariantList s {QStringLiteral("Fs"), QStringLiteral("u"), QStringLiteral("0413")};

        QVariantList r = Utilities::stringList2VariantList(l);
        QCOMPARE(r, s);
    }

    void QMdmmUtilitiesvariantList2StringList()
    {
        QVariantList l {QStringLiteral("3140"), QStringLiteral("u"), QStringLiteral("sF")};
        QStringList s {QStringLiteral("3140"), QStringLiteral("u"), QStringLiteral("sF")};

        QStringList r = Utilities::variantList2StringList(l);
        QCOMPARE(r, s);
    }
};

namespace {
RegisterTestObject<tst_QMdmmCore> _;
}
#include "tst_qmdmmcore.moc"
