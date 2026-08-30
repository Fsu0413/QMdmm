#include "test.h"

#include <QMdmmCore/QMdmmPlayer>
#include <QMdmmCore/QMdmmRoom>

#include <QPointer>
#include <QSignalSpy>
#include <QTest>

#include <limits>

// NOLINTBEGIN

using namespace QMdmmCore;

class tst_QMdmmLogicConfiguration : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE tst_QMdmmLogicConfiguration() = default;

private slots:

#define TEST_CONFIGURATION(valueName, ValueName, defaultValue, v1value, value) \
    void QMdmmLogicConfiguration##valueName()                                  \
    {                                                                          \
        {                                                                      \
            LogicConfiguration c;                                              \
            QCOMPARE(c.valueName(), (defaultValue));                           \
        }                                                                      \
        {                                                                      \
            LogicConfiguration c = LogicConfiguration::v1();                   \
            QCOMPARE(c.valueName(), (v1value));                                \
        }                                                                      \
        {                                                                      \
            LogicConfiguration c = LogicConfiguration::defaults();             \
            QCOMPARE(c.valueName(), (defaultValue));                           \
            c.set##ValueName(value);                                           \
            QCOMPARE(c.valueName(), (value));                                  \
        }                                                                      \
    }

    TEST_CONFIGURATION(initialKnifeDamage, InitialKnifeDamage, 1, 1, 5)
    TEST_CONFIGURATION(maximumKnifeDamage, MaximumKnifeDamage, 10, 3, 30)
    TEST_CONFIGURATION(initialHorseDamage, InitialHorseDamage, 2, 3, 6)
    TEST_CONFIGURATION(maximumHorseDamage, MaximumHorseDamage, 10, 5, 31)
    TEST_CONFIGURATION(initialMaxHp, InitialMaxHp, 10, 7, 20)
    TEST_CONFIGURATION(maximumMaxHp, MaximumMaxHp, 20, 7, 50)
    TEST_CONFIGURATION(punishHpModifier, PunishHpModifier, 2, 0, 5)
    TEST_CONFIGURATION(punishHpRoundStrategy, PunishHpRoundStrategy, LogicConfiguration::RoundToNearest45, LogicConfiguration::RoundToNearest45, LogicConfiguration::RoundDown)
    TEST_CONFIGURATION(zeroHpAsDead, ZeroHpAsDead, true, false, false)
    TEST_CONFIGURATION(enableLetMove, EnableLetMove, true, false, false)
    TEST_CONFIGURATION(canBuyOnlyInInitialCity, CanBuyOnlyInInitialCity, false, false, true)

#undef TEST_CONFIGURATION

    void QMdmmLogicConfigurationdeserialize_data()
    {
        QTest::addColumn<QJsonValue>("value");
        QTest::addColumn<bool>("result");

        struct ConfigurationTestTriplet
        {
            QString key;
            QJsonValue validValue;
            QJsonValue invalidValue;
        };

        static const QList<ConfigurationTestTriplet> testTriplets {
            {QStringLiteral("initialKnifeDamage"), 2, true},
            {QStringLiteral("maximumKnifeDamage"), 2, true},
            {QStringLiteral("initialHorseDamage"), 2, true},
            {QStringLiteral("maximumHorseDamage"), 2, true},
            {QStringLiteral("initialMaxHp"), 2, true},
            {QStringLiteral("maximumMaxHp"), 2, true},
            {QStringLiteral("punishHpModifier"), 2, true},
            {QStringLiteral("punishHpRoundStrategy"), 2, true},
            {QStringLiteral("zeroHpAsDead"), false, QStringLiteral("what?")},
            {QStringLiteral("enableLetMove"), false, QStringLiteral("what?")},
            {QStringLiteral("canBuyOnlyInInitialCity"), false, QStringLiteral("what?")},
        };

        QJsonObject ob;

        foreach (const ConfigurationTestTriplet &t, testTriplets) {
            QTest::newRow((t.key + QStringLiteral("-nonexist")).toUtf8().constData()) << QJsonValue(ob) << false;
            ob.insert(t.key, t.invalidValue);
            QTest::newRow((t.key + QStringLiteral("-invalid")).toUtf8().constData()) << QJsonValue(ob) << false;
            ob.insert(t.key, t.validValue);
        }

        QTest::newRow("valid") << QJsonValue(ob) << true;
        QTest::newRow("notObject") << QJsonValue(QJsonValue::Null) << false;

        // Value-level validation (defensive programming): negative / fraction / NaN /
        // enum-out-of-range / initial-exceeds-maximum must all be rejected.
        QJsonObject validOb {
            {QStringLiteral("initialKnifeDamage"), 1},
            {QStringLiteral("maximumKnifeDamage"), 10},
            {QStringLiteral("initialHorseDamage"), 2},
            {QStringLiteral("maximumHorseDamage"), 10},
            {QStringLiteral("initialMaxHp"), 10},
            {QStringLiteral("maximumMaxHp"), 20},
            {QStringLiteral("punishHpModifier"), 2},
            {QStringLiteral("punishHpRoundStrategy"), static_cast<int>(LogicConfiguration::RoundToNearest45)},
            {QStringLiteral("zeroHpAsDead"), true},
            {QStringLiteral("enableLetMove"), true},
            {QStringLiteral("canBuyOnlyInInitialCity"), false},
        };

        {
            QJsonObject negativeOb = validOb;
            negativeOb.insert(QStringLiteral("initialKnifeDamage"), -1);
            QTest::newRow("negative") << QJsonValue(negativeOb) << false;
        }
        {
            QJsonObject fractionOb = validOb;
            fractionOb.insert(QStringLiteral("initialKnifeDamage"), 1.5);
            QTest::newRow("fraction") << QJsonValue(fractionOb) << false;
        }
        {
            QJsonObject nanOb = validOb;
            nanOb.insert(QStringLiteral("initialKnifeDamage"), std::numeric_limits<double>::quiet_NaN());
            QTest::newRow("nan") << QJsonValue(nanOb) << false;
        }
        {
            QJsonObject enumOb = validOb;
            enumOb.insert(QStringLiteral("punishHpRoundStrategy"), 4);
            QTest::newRow("enumOutOfRange") << QJsonValue(enumOb) << false;
        }
        {
            QJsonObject knifeOb = validOb;
            knifeOb.insert(QStringLiteral("initialKnifeDamage"), 11);
            QTest::newRow("initialKnifeGreaterThanMaximum") << QJsonValue(knifeOb) << false;
        }
        {
            QJsonObject horseOb = validOb;
            horseOb.insert(QStringLiteral("initialHorseDamage"), 11);
            QTest::newRow("initialHorseGreaterThanMaximum") << QJsonValue(horseOb) << false;
        }
        {
            QJsonObject maxHpOb = validOb;
            maxHpOb.insert(QStringLiteral("initialMaxHp"), 21);
            QTest::newRow("initialMaxHpGreaterThanMaximum") << QJsonValue(maxHpOb) << false;
        }
    }
    void QMdmmLogicConfigurationdeserialize()
    {
        QFETCH(QJsonValue, value);
        QFETCH(bool, result);

        LogicConfiguration conf;
        bool r = conf.deserialize(value);
        QCOMPARE(r, result);
        if (r) {
            QCOMPARE(QJsonValue(QJsonObject(conf)), value);
        }
    }
};

namespace {
RegisterTestObject<tst_QMdmmLogicConfiguration> _a;
} // namespace
#include "tst_qmdmmlogicconfiguration.moc"