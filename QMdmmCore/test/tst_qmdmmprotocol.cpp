#include "test.h"

#include <QMdmmCore/QMdmmProtocol>

#include <QJsonObject>
#include <QTest>

class tst_QMdmmProtocol : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE tst_QMdmmProtocol() = default;

private slots:
    // coverage for QMdmmPacketData::QMdmmPacketData
    void QMdmmPacketDataCopy()
    {
        QJsonObject ob;
        QMdmmPacketData e = ob;
        QJsonObject ob2;
        ob2.insert(QStringLiteral("test"), QJsonValue());
        e = ob2;
    }

    void QMdmmProtocolprotocolVersion()
    {
        int r = QMdmmProtocol::protocolVersion();
        QCOMPARE(r, 0);
    }

    void QMdmmPackettype()
    {
        // case 1
        {
            QMdmmPacket p;
            QMdmmProtocol::PacketType t = p.type();
            QCOMPARE(t, QMdmmProtocol::TypeInvalid);
        }

        // case 2
        {
            QMdmmPacket p(QMdmmProtocol::TypeRequest, QMdmmProtocol::RequestStoneScissorsCloth, {});
            QMdmmProtocol::PacketType t = p.type();
            QCOMPARE(t, QMdmmProtocol::TypeRequest);
        }

        // case 3
        {
            QMdmmPacket p(QMdmmProtocol::NotifyVersion, {});
            QMdmmProtocol::PacketType t = p.type();
            QCOMPARE(t, QMdmmProtocol::TypeNotify);
        }
    }

    void QMdmmPacketrequestId()
    {
        // case 1
        {
            QMdmmPacket p;
            QMdmmProtocol::RequestId t = p.requestId();
            QCOMPARE(t, QMdmmProtocol::RequestInvalid);
        }

        // case 2
        {
            QMdmmPacket p(QMdmmProtocol::TypeRequest, QMdmmProtocol::RequestStoneScissorsCloth, {});
            QMdmmProtocol::RequestId t = p.requestId();
            QCOMPARE(t, QMdmmProtocol::RequestStoneScissorsCloth);
        }

        // case 3
        {
            QMdmmPacket p(QMdmmProtocol::NotifyVersion, {});
            QMdmmProtocol::RequestId t = p.requestId();
            QCOMPARE(t, QMdmmProtocol::RequestInvalid);
        }
    }

    void QMdmmPacketnotifyId()
    {
        // case 1
        {
            QMdmmPacket p;
            QMdmmProtocol::NotifyId t = p.notifyId();
            QCOMPARE(t, QMdmmProtocol::NotifyInvalid);
        }

        // case 2
        {
            QMdmmPacket p(QMdmmProtocol::TypeRequest, QMdmmProtocol::RequestStoneScissorsCloth, {});
            QMdmmProtocol::NotifyId t = p.notifyId();
            QCOMPARE(t, QMdmmProtocol::NotifyInvalid);
        }

        // case 3
        {
            QMdmmPacket p(QMdmmProtocol::NotifyVersion, {});
            QMdmmProtocol::NotifyId t = p.notifyId();
            QCOMPARE(t, QMdmmProtocol::NotifyVersion);
        }
    }

    void QMdmmPacketvalue()
    {
        // case 1
        {
            QMdmmPacket p;
            QJsonValue t = p.value();
            QVERIFY(t.isNull());
        }

        // case 2
        {
            QMdmmPacket p(QMdmmProtocol::TypeRequest, QMdmmProtocol::RequestStoneScissorsCloth, {1});
            QJsonValue t = p.value();
            QVERIFY(!t.isNull());
            QCOMPARE(t.toInt(), 1);
        }
    }

    void QMdmmPacketserialize()
    {
        {
            QMdmmPacket p(QMdmmProtocol::TypeRequest, QMdmmProtocol::RequestStoneScissorsCloth, {1});
            QByteArray arr = p;

            // The Json object created by Qt is sorted by key
            QByteArray d = R"json({"notifyId":0,"requestId":1,"type":1,"value":1})json";

            QCOMPARE(arr, d);
        }
    }

    void QMdmmPacketfromJsonhasError_data()
    {
        QTest::addColumn<QByteArray>("input");
        QTest::addColumn<QString>("errorString");

        QTest::newRow("invalid") << QByteArray("some_invalid") << QStringLiteral("Json error: illegal value");
        QTest::newRow("not-object") << QByteArray("[1,2,3]") << QStringLiteral("Document is not object");
        QTest::newRow("type-notexist") << QByteArray("{}") << QStringLiteral("'type' is non-existent");
        QTest::newRow("type-invalid") << QByteArray(R"json({"type": "Fsu0413"})json") << QStringLiteral("'type' is not number");
        QTest::newRow("requestid-notexist") << QByteArray(R"json({"type": 1})json") << QStringLiteral("'requestId' is non-existent");
        QTest::newRow("requestid-invalid") << QByteArray(R"json({"type": 1, "requestId": "Fsu0413"})json") << QStringLiteral("'requestId' is not number");
        QTest::newRow("notifyid-notexist") << QByteArray(R"json({"type": 1, "requestId": 2})json") << QStringLiteral("'notifyId' is non-existent");
        QTest::newRow("notifyid-invalid") << QByteArray(R"json({"type": 1, "requestId": 2, "notifyId": "Fsu0413"})json") << QStringLiteral("'notifyId' is not number");
        QTest::newRow("value-notexist") << QByteArray(R"json({"type": 1, "requestId": 2, "notifyId": 8193})json") << QStringLiteral("'value' is non-existent");
        QTest::newRow("valid") << QByteArray(R"json({"type": 1, "requestId": 2, "notifyId": 8193, "value": "Fsu0413"})json") << QString {};
    }
    void QMdmmPacketfromJsonhasError()
    {
        QFETCH(QByteArray, input);
        QFETCH(QString, errorString);

        QString actualErrorString;
        QString actualErrorString2;

        QMdmmPacket p = QMdmmPacket::fromJson(input, &actualErrorString);

        QCOMPARE(errorString, actualErrorString);
        QCOMPARE(p.hasError(&actualErrorString2), !actualErrorString.isEmpty());
        QCOMPARE(actualErrorString, actualErrorString2);

        if (actualErrorString.isEmpty()) {
            QCOMPARE(p.type(), QMdmmProtocol::TypeRequest);
            QCOMPARE(p.requestId(), QMdmmProtocol::RequestActionOrder);
            QCOMPARE(p.value(), QStringLiteral("Fsu0413"));

            // This packet is TypeRequest so notifyId should get invalid
            QCOMPARE(p.notifyId(), QMdmmProtocol::NotifyInvalid);
        }
    }
    void QMdmmPacketfromJsonhasError2()
    {
        {
            // coverage for errorString = nullptr
            (void)QMdmmPacket::fromJson({}, nullptr).hasError();
        }
        {
            // Check of a notify JSON
            QByteArray input = R"json({"type": 3, "requestId": 2, "notifyId": 8193, "value": "Fsu0413"})json";
            QString actualErrorString;
            QString actualErrorString2;

            QMdmmPacket p = QMdmmPacket::fromJson(input, &actualErrorString);

            QVERIFY(actualErrorString.isEmpty());
            QCOMPARE(p.hasError(&actualErrorString2), !actualErrorString.isEmpty());
            QCOMPARE(actualErrorString, actualErrorString2);

            if (actualErrorString.isEmpty()) {
                QCOMPARE(p.type(), QMdmmProtocol::TypeNotify);
                QCOMPARE(p.notifyId(), QMdmmProtocol::NotifyLogicConfiguration);
                QCOMPARE(p.value(), QStringLiteral("Fsu0413"));

                // This packet is TypeNotify so requestId should get invalid
                QCOMPARE(p.requestId(), QMdmmProtocol::RequestInvalid);
            }
        }
    }

    void QMdmmPacketQ_DECLARE_METATYPE()
    {
        // coverage for Q_DECLARE_METATYPE

        QMdmmPacket p(QMdmmProtocol::TypeRequest, QMdmmProtocol::RequestStoneScissorsCloth, {});
        QVariant v = QVariant::fromValue<QMdmmPacket>(p);
        QCOMPARE(v.value<QMdmmPacket>().serialize(), p.serialize());
    }
};

namespace {
RegisterTestObject<tst_QMdmmProtocol> _;
}
#include "tst_qmdmmprotocol.moc"
