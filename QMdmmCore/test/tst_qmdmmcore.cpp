
#include <QMdmmCore/QMdmmCoreGlobal>

#include <QHash>
#include <QString>
#include <QTest>

class tst_QMdmmCore : public QObject
{
    Q_OBJECT

private slots:
    void QMdmmDataisPlaceAdjacent_data()
    {
        QTest::addColumn<int>("p1");
        QTest::addColumn<int>("p2");
        QTest::addColumn<bool>("result");

        QTest::newRow("nc-nc") << 1 << 2 << false;
        QTest::newRow("c-c") << (int)QMdmmData::Country << (int)QMdmmData::Country << false;
        QTest::newRow("nc-c") << 1 << (int)QMdmmData::Country << true;
        QTest::newRow("c-nc") << (int)QMdmmData::Country << 2 << true;
    }
    void QMdmmDataisPlaceAdjacent()
    {
        QFETCH(int, p1);
        QFETCH(int, p2);
        QFETCH(bool, result);

        bool r = QMdmmData::isPlaceAdjacent(p1, p2);
        QCOMPARE(r, result);
    }

    void QMdmmDatastoneScissorsClothWinners_data()
    {
        typedef QHash<QString, QMdmmData::StoneScissorsCloth> JudgeHash;

        QTest::addColumn<JudgeHash>("judgers");
        QTest::addColumn<QStringList>("result");

        QTest::newRow("tie-allsame") << JudgeHash {
            std::make_pair(QStringLiteral("1"), QMdmmData::Stone),
            std::make_pair(QStringLiteral("2"), QMdmmData::Stone),
            std::make_pair(QStringLiteral("3"), QMdmmData::Stone),
            std::make_pair(QStringLiteral("4"), QMdmmData::Stone),
        } << QStringList {};
        QTest::newRow("tie-alldiff") << JudgeHash {
            std::make_pair(QStringLiteral("1"), QMdmmData::Stone),
            std::make_pair(QStringLiteral("2"), QMdmmData::Scissors),
            std::make_pair(QStringLiteral("3"), QMdmmData::Cloth),
            std::make_pair(QStringLiteral("4"), QMdmmData::Stone),
        } << QStringList {};
        QTest::newRow("stone-vs-scissors") << JudgeHash {
            std::make_pair(QStringLiteral("1"), QMdmmData::Stone),
            std::make_pair(QStringLiteral("2"), QMdmmData::Scissors),
            std::make_pair(QStringLiteral("3"), QMdmmData::Stone),
            std::make_pair(QStringLiteral("4"), QMdmmData::Scissors),
        } << QStringList {
            QStringLiteral("1"),
            QStringLiteral("1"),
            QStringLiteral("3"),
            QStringLiteral("3"),
        };
        QTest::newRow("cloth-vs-stone") << JudgeHash {
            std::make_pair(QStringLiteral("1"), QMdmmData::Stone),
            std::make_pair(QStringLiteral("2"), QMdmmData::Cloth),
            std::make_pair(QStringLiteral("3"), QMdmmData::Cloth),
            std::make_pair(QStringLiteral("4"), QMdmmData::Cloth),
        } << QStringList {
            QStringLiteral("2"),
            QStringLiteral("3"),
            QStringLiteral("4"),
        };
    }
    void QMdmmDatastoneScissorsClothWinners()
    {
        typedef QHash<QString, QMdmmData::StoneScissorsCloth> JudgeHash;

        QFETCH(JudgeHash, judgers);
        QFETCH(QStringList, result);

        QStringList r = QMdmmData::stoneScissorsClothWinners(judgers);
        foreach (const QString &a, result)
            r.removeOne(a);

        QCOMPARE(r, QStringList {});
    }

    void QMdmmGlobalversion()
    {
        QVersionNumber r = QMdmmGlobal::version();
        QCOMPARE(r, QVersionNumber::fromString(QStringLiteral(QMDMM_VERSION)));
    }

    void QMdmmUtilitieslist2Set()
    {
        QList<int> l {1, 2, 3, 4, 5, 2, 3, 4, 5, 6};
        QSet<int> s {1, 2, 3, 4, 5, 6};

        QSet<int> r = QMdmmUtilities::list2Set(l);
        QCOMPARE(r, s);
    }

    void QMdmmUtilitiesenumList2VariantList1()
    {
        QList<QMdmmData::StoneScissorsCloth> l {QMdmmData::Stone, QMdmmData::Scissors, QMdmmData::Cloth, QMdmmData::Stone};
        QVariantList s {static_cast<int>(QMdmmData::Stone), static_cast<int>(QMdmmData::Scissors), static_cast<int>(QMdmmData::Cloth), static_cast<int>(QMdmmData::Stone)};

        QVariantList r = QMdmmUtilities::enumList2VariantList(l);
        QCOMPARE(r, s);
    }

    void QMdmmUtilitiesenumList2VariantList2()
    {
        QList<QMdmmData::AgentState> l {QMdmmData::StateOnlineBot, QMdmmData::StateOnlineTrust, QMdmmData::StateOffline};
        QVariantList s {static_cast<int>(QMdmmData::AgentState(QMdmmData::StateOnlineBot)), static_cast<int>(QMdmmData::AgentState(QMdmmData::StateOnlineTrust)),
                        static_cast<int>(QMdmmData::AgentState(QMdmmData::StateOffline))};

        QVariantList r = QMdmmUtilities::enumList2VariantList(l);
        QCOMPARE(r, s);
    }

    void QMdmmUtilitiesintList2VariantList()
    {
        QList<int> l {1, 2, 3, 4, 5, 6};
        QVariantList s {1, 2, 3, 4, 5, 6};

        QVariantList r = QMdmmUtilities::intList2VariantList(l);
        QCOMPARE(r, s);
    }

    void QMdmmUtilitiesvariantList2IntList()
    {
        QVariantList l {6, 7, 8, 9, 10, 11};
        QList<int> s {6, 7, 8, 9, 10, 11};

        QList<int> r = QMdmmUtilities::variantList2IntList(l);
        QCOMPARE(r, s);
    }

    void QMdmmUtilitiesstringList2VariantList()
    {
        QStringList l {QStringLiteral("Fs"), QStringLiteral("u"), QStringLiteral("0413")};
        QVariantList s {QStringLiteral("Fs"), QStringLiteral("u"), QStringLiteral("0413")};

        QVariantList r = QMdmmUtilities::stringList2VariantList(l);
        QCOMPARE(r, s);
    }

    void QMdmmUtilitiesvariantList2StringList()
    {
        QVariantList l {QStringLiteral("3140"), QStringLiteral("u"), QStringLiteral("sF")};
        QStringList s {QStringLiteral("3140"), QStringLiteral("u"), QStringLiteral("sF")};

        QStringList r = QMdmmUtilities::variantList2StringList(l);
        QCOMPARE(r, s);
    }
};

QTEST_GUILESS_MAIN(tst_QMdmmCore)
#include "tst_qmdmmcore.moc"
