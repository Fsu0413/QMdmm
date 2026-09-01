// SPDX-License-Identifier: AGPL-3.0-or-later

#include "test.h"

#include <QMdmmServer>

#include <QTest>

#include <limits>

// NOLINTBEGIN

using namespace QMdmmNetworking;

class tst_QMdmmServerConfiguration : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE tst_QMdmmServerConfiguration() = default;

private slots:

    void QMdmmServerConfigurationdefaults()
    {
        ServerConfiguration c;
        QCOMPARE(c.tcpEnabled(), true);
        QCOMPARE(c.tcpPort(), (uint16_t)6366);
        QCOMPARE(c.localEnabled(), true);
        QCOMPARE(c.localSocketName(), QStringLiteral("QMdmm"));
        QCOMPARE(c.websocketEnabled(), true);
        QCOMPARE(c.websocketName(), QStringLiteral("QMdmm"));
        QCOMPARE(c.websocketPort(), (uint16_t)6367);
        QCOMPARE(c.playerNumPerRoom(), 3);
        QCOMPARE(c.requestTimeout(), 20);
    }

    void QMdmmServerConfigurationsetters()
    {
        ServerConfiguration c;
        c.setTcpEnabled(false);
        c.setTcpPort(7000);
        c.setLocalEnabled(false);
        c.setLocalSocketName(QStringLiteral("test-local"));
        c.setWebsocketEnabled(false);
        c.setWebsocketName(QStringLiteral("test-websocket"));
        c.setWebsocketPort(8000);
        c.setPlayerNumPerRoom(8);
        c.setRequestTimeout(30);

        QCOMPARE(c.tcpEnabled(), false);
        QCOMPARE(c.tcpPort(), (uint16_t)7000);
        QCOMPARE(c.localEnabled(), false);
        QCOMPARE(c.localSocketName(), QStringLiteral("test-local"));
        QCOMPARE(c.websocketEnabled(), false);
        QCOMPARE(c.websocketName(), QStringLiteral("test-websocket"));
        QCOMPARE(c.websocketPort(), (uint16_t)8000);
        QCOMPARE(c.playerNumPerRoom(), 8);
        QCOMPARE(c.requestTimeout(), 30);
    }

    void QMdmmServerConfigurationdeserialize_data()
    {
        QTest::addColumn<QJsonValue>("value");
        QTest::addColumn<bool>("result");

        const QJsonObject validOb {
            {QStringLiteral("tcpEnabled"), true},       {QStringLiteral("tcpPort"), 6366},
            {QStringLiteral("localEnabled"), true},     {QStringLiteral("localSocketName"), QStringLiteral("QMdmm")},
            {QStringLiteral("websocketEnabled"), true}, {QStringLiteral("websocketName"), QStringLiteral("QMdmm")},
            {QStringLiteral("websocketPort"), 6367},    {QStringLiteral("playerNumPerRoom"), 3},
            {QStringLiteral("requestTimeout"), 20},
        };

        QTest::newRow("valid") << QJsonValue(validOb) << true;
        QTest::newRow("notObject") << QJsonValue(QJsonValue::Null) << false;

        // Missing key (a typo leaves a required key absent) must be rejected.
        {
            QJsonObject ob = validOb;
            ob.remove(QStringLiteral("tcpPort"));
            QTest::newRow("missingKey") << QJsonValue(ob) << false;
        }
        // Wrong type: boolean field holding a string.
        {
            QJsonObject ob = validOb;
            ob.insert(QStringLiteral("tcpEnabled"), QStringLiteral("true"));
            QTest::newRow("boolWrongType") << QJsonValue(ob) << false;
        }
        // Wrong type: string field holding a number.
        {
            QJsonObject ob = validOb;
            ob.insert(QStringLiteral("localSocketName"), 123);
            QTest::newRow("stringWrongType") << QJsonValue(ob) << false;
        }
        // Fractional port.
        {
            QJsonObject ob = validOb;
            ob.insert(QStringLiteral("tcpPort"), 6366.5);
            QTest::newRow("fraction") << QJsonValue(ob) << false;
        }
        // Negative port.
        {
            QJsonObject ob = validOb;
            ob.insert(QStringLiteral("tcpPort"), -1);
            QTest::newRow("negativePort") << QJsonValue(ob) << false;
        }
        // Port 0 is reserved.
        {
            QJsonObject ob = validOb;
            ob.insert(QStringLiteral("tcpPort"), 0);
            QTest::newRow("portZero") << QJsonValue(ob) << false;
        }
        // Port out of uint16_t range (would silently truncate on read).
        {
            QJsonObject ob = validOb;
            ob.insert(QStringLiteral("websocketPort"), 70000);
            QTest::newRow("portOverflow") << QJsonValue(ob) << false;
        }
        // playerNumPerRoom below the minimum of 2.
        {
            QJsonObject ob = validOb;
            ob.insert(QStringLiteral("playerNumPerRoom"), 1);
            QTest::newRow("playersBelowMinimum") << QJsonValue(ob) << false;
        }
        // requestTimeout below 15 but not 0.
        {
            QJsonObject ob = validOb;
            ob.insert(QStringLiteral("requestTimeout"), 1);
            QTest::newRow("timeoutBelowMinimum") << QJsonValue(ob) << false;
        }
        // Negative requestTimeout.
        {
            QJsonObject ob = validOb;
            ob.insert(QStringLiteral("requestTimeout"), -1);
            QTest::newRow("negativeTimeout") << QJsonValue(ob) << false;
        }
        // NaN requestTimeout.
        {
            QJsonObject ob = validOb;
            ob.insert(QStringLiteral("requestTimeout"), std::numeric_limits<double>::quiet_NaN());
            QTest::newRow("nanTimeout") << QJsonValue(ob) << false;
        }
        // requestTimeout = 0 (no explicit timeout, grace only) is valid.
        {
            QJsonObject ob = validOb;
            ob.insert(QStringLiteral("requestTimeout"), 0);
            QTest::newRow("timeoutZero") << QJsonValue(ob) << true;
        }
        // requestTimeout = 15 (minimum explicit timeout) is valid.
        {
            QJsonObject ob = validOb;
            ob.insert(QStringLiteral("requestTimeout"), 15);
            QTest::newRow("timeoutMinimum") << QJsonValue(ob) << true;
        }
    }

    void QMdmmServerConfigurationdeserialize()
    {
        QFETCH(QJsonValue, value);
        QFETCH(bool, result);

        ServerConfiguration conf;
        bool r = conf.deserialize(value);
        QCOMPARE(r, result);
        if (r) {
            QCOMPARE(QJsonValue(QJsonObject(conf)), value);
        }
    }

    void QMdmmServerConfigurationdeserializeIgnoresUnknownKeys()
    {
        const QJsonObject ob {
            {QStringLiteral("tcpEnabled"), true},       {QStringLiteral("tcpPort"), 6366},
            {QStringLiteral("localEnabled"), true},     {QStringLiteral("localSocketName"), QStringLiteral("QMdmm")},
            {QStringLiteral("websocketEnabled"), true}, {QStringLiteral("websocketName"), QStringLiteral("QMdmm")},
            {QStringLiteral("websocketPort"), 6367},    {QStringLiteral("playerNumPerRoom"), 3},
            {QStringLiteral("requestTimeout"), 20},     {QStringLiteral("unknownKey"), 123},
        };

        ServerConfiguration conf;
        QVERIFY(conf.deserialize(QJsonValue(ob)));
        QVERIFY(!conf.contains(QStringLiteral("unknownKey")));
    }
};

namespace {
RegisterTestObject<tst_QMdmmServerConfiguration> _a;
} // namespace
#include "tst_qmdmmserverconfiguration.moc"
