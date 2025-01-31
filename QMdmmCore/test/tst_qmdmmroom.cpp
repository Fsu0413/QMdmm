#include "test.h"

#include <QMdmmCore/QMdmmRoom>

#include <QTest>

// NOLINTBEGIN

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
            QMdmmLogicConfiguration c;                                         \
            QCOMPARE(c.valueName(), (defaultValue));                           \
        }                                                                      \
        {                                                                      \
            QMdmmLogicConfiguration c = QMdmmLogicConfiguration::v1();         \
            QCOMPARE(c.valueName(), (v1value));                                \
        }                                                                      \
        {                                                                      \
            QMdmmLogicConfiguration c = QMdmmLogicConfiguration::defaults();   \
            QCOMPARE(c.valueName(), (defaultValue));                           \
            c.set##ValueName(value);                                           \
            QCOMPARE(c.valueName(), (value));                                  \
        }                                                                      \
    }

    TEST_CONFIGURATION(playerNumPerRoom, PlayerNumPerRoom, 3, 3, 5)
    TEST_CONFIGURATION(requestTimeout, RequestTimeout, 20, 20, 30)
    TEST_CONFIGURATION(initialKnifeDamage, InitialKnifeDamage, 1, 1, 5)
    TEST_CONFIGURATION(maximumKnifeDamage, MaximumKnifeDamage, 10, 3, 30)
    TEST_CONFIGURATION(initialHorseDamage, InitialHorseDamage, 2, 3, 6)
    TEST_CONFIGURATION(maximumHorseDamage, MaximumHorseDamage, 10, 5, 31)
    TEST_CONFIGURATION(initialMaxHp, InitialMaxHp, 10, 7, 20)
    TEST_CONFIGURATION(maximumMaxHp, MaximumMaxHp, 20, 7, 50)
    TEST_CONFIGURATION(punishHpModifier, PunishHpModifier, 2, 0, 5)
    TEST_CONFIGURATION(punishHpRoundStrategy, PunishHpRoundStrategy, QMdmmLogicConfiguration::RoundToNearest45, QMdmmLogicConfiguration::RoundToNearest45,
                       QMdmmLogicConfiguration::RoundDown)
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
            {QStringLiteral("playerNumPerRoom"), 1, true},
            {QStringLiteral("requestTimeout"), 2, true},
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
    }
    void QMdmmLogicConfigurationdeserialize()
    {
        QFETCH(QJsonValue, value);
        QFETCH(bool, result);

        QMdmmLogicConfiguration conf;
        bool r = conf.deserialize(value);
        QCOMPARE(r, result);
        if (r) {
            QCOMPARE(QJsonValue(QJsonObject(conf)), value);
        }
    }
};

class tst_QMdmmRoom : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE tst_QMdmmRoom() = default;

private slots:
    // TBD
};

namespace {
RegisterTestObject<tst_QMdmmLogicConfiguration> _a;
RegisterTestObject<tst_QMdmmRoom> _b;
} // namespace
#include "tst_qmdmmroom.moc"
