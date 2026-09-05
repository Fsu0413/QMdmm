#include "test.h"

#include <QMdmmCore/QMdmmProtocol>

#include <QJsonObject>
#include <QTest>

// NOLINTBEGIN

using namespace QMdmmCore;

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
        p::PacketDataP e = ob;
        QJsonObject ob2;
        ob2.insert(QStringLiteral("test"), QJsonValue());
        e = ob2;
    }

    void QMdmmProtocolprotocolVersion()
    {
        int r = Protocol::version();
        QCOMPARE(r, 0);
    }

    void QMdmmPackettype()
    {
        // case 1
        {
            Packet p;
            Protocol::PacketType t = p.type();
            QCOMPARE(t, Protocol::TypeInvalid);
        }

        // case 2
        {
            Packet p(Protocol::TypeRequest, Protocol::RequestRockPaperScissors, {});
            Protocol::PacketType t = p.type();
            QCOMPARE(t, Protocol::TypeRequest);
        }

        // case 3
        {
            Packet p(Protocol::NotifyVersion, {});
            Protocol::PacketType t = p.type();
            QCOMPARE(t, Protocol::TypeNotify);
        }
    }

    void QMdmmPacketrequestId()
    {
        // case 1
        {
            Packet p;
            Protocol::RequestId t = p.requestId();
            QCOMPARE(t, Protocol::RequestInvalid);
        }

        // case 2
        {
            Packet p(Protocol::TypeRequest, Protocol::RequestRockPaperScissors, {});
            Protocol::RequestId t = p.requestId();
            QCOMPARE(t, Protocol::RequestRockPaperScissors);
        }

        // case 3
        {
            Packet p(Protocol::NotifyVersion, {});
            Protocol::RequestId t = p.requestId();
            QCOMPARE(t, Protocol::RequestInvalid);
        }
    }

    void QMdmmPacketnotifyId()
    {
        // case 1
        {
            Packet p;
            Protocol::NotifyId t = p.notifyId();
            QCOMPARE(t, Protocol::NotifyInvalid);
        }

        // case 2
        {
            Packet p(Protocol::TypeRequest, Protocol::RequestRockPaperScissors, {});
            Protocol::NotifyId t = p.notifyId();
            QCOMPARE(t, Protocol::NotifyInvalid);
        }

        // case 3
        {
            Packet p(Protocol::NotifyVersion, {});
            Protocol::NotifyId t = p.notifyId();
            QCOMPARE(t, Protocol::NotifyVersion);
        }
    }

    void QMdmmPacketvalue()
    {
        // case 1
        {
            Packet p;
            QJsonValue t = p.value();
            QVERIFY(t.isNull());
        }

        // case 2
        {
            Packet p(Protocol::TypeRequest, Protocol::RequestRockPaperScissors, {1});
            QJsonValue t = p.value();
            QVERIFY(!t.isNull());
            QCOMPARE(t.toInt(), 1);
        }
    }

    void QMdmmPacketserialize()
    {
        {
            Packet p(Protocol::TypeRequest, Protocol::RequestRockPaperScissors, {1});
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

        QTest::newRow("not-object") << QByteArray("[1,2,3]") << QStringLiteral("Document is not object");
        QTest::newRow("type-notexist") << QByteArray("{}") << QStringLiteral("'type' is non-existent");
        QTest::newRow("type-invalid") << QByteArray(R"json({"type": "Fsu0413"})json") << QStringLiteral("'type' is not number");
        QTest::newRow("type-fractional") << QByteArray(R"json({"type": 1.5})json") << QStringLiteral("'type' is not an integer");
        QTest::newRow("type-outofrange") << QByteArray(R"json({"type": 99})json") << QStringLiteral("'type' is out of range");
        QTest::newRow("requestid-notexist") << QByteArray(R"json({"type": 1})json") << QStringLiteral("'requestId' is non-existent");
        QTest::newRow("requestid-invalid") << QByteArray(R"json({"type": 1, "requestId": "Fsu0413"})json") << QStringLiteral("'requestId' is not number");
        QTest::newRow("requestid-fractional") << QByteArray(R"json({"type": 1, "requestId": 2.5})json") << QStringLiteral("'requestId' is not an integer");
        QTest::newRow("requestid-outofrange") << QByteArray(R"json({"type": 1, "requestId": 99})json") << QStringLiteral("'requestId' is out of range");
        QTest::newRow("notifyid-notexist") << QByteArray(R"json({"type": 1, "requestId": 2})json") << QStringLiteral("'notifyId' is non-existent");
        QTest::newRow("notifyid-invalid") << QByteArray(R"json({"type": 1, "requestId": 2, "notifyId": "Fsu0413"})json") << QStringLiteral("'notifyId' is not number");
        QTest::newRow("notifyid-fractional") << QByteArray(R"json({"type": 1, "requestId": 2, "notifyId": 8193.5})json") << QStringLiteral("'notifyId' is not an integer");
        QTest::newRow("notifyid-outofrange") << QByteArray(R"json({"type": 1, "requestId": 2, "notifyId": 99})json") << QStringLiteral("'notifyId' is out of range");
        QTest::newRow("request-requestid-invalid") << QByteArray(R"json({"type": 1, "requestId": 0, "notifyId": 0, "value": null})json")
                                                   << QStringLiteral("'requestId' is invalid for a request/reply packet");
        QTest::newRow("request-notifyid-notinvalid") << QByteArray(R"json({"type": 1, "requestId": 1, "notifyId": 8193, "value": null})json")
                                                     << QStringLiteral("'notifyId' should be invalid for a request/reply packet");
        QTest::newRow("notify-notifyid-invalid") << QByteArray(R"json({"type": 3, "requestId": 0, "notifyId": 0, "value": null})json")
                                                 << QStringLiteral("'notifyId' is invalid for a notify packet");
        QTest::newRow("notify-requestid-notinvalid") << QByteArray(R"json({"type": 3, "requestId": 1, "notifyId": 8193, "value": null})json")
                                                     << QStringLiteral("'requestId' should be invalid for a notify packet");
        QTest::newRow("value-notexist") << QByteArray(R"json({"type": 1, "requestId": 2, "notifyId": 8193})json") << QStringLiteral("'value' is non-existent");
        QTest::newRow("valid") << QByteArray(R"json({"type": 1, "requestId": 2, "notifyId": 0, "value": "Fsu0413"})json") << QString {};
    }
    void QMdmmPacketfromJsonhasError()
    {
        QFETCH(QByteArray, input);
        QFETCH(QString, errorString);

        QString actualErrorString;

        Packet p = Packet::fromJson(input);

        QCOMPARE(p.hasError(&actualErrorString), !errorString.isEmpty());
        QCOMPARE(errorString, actualErrorString);

        if (actualErrorString.isEmpty()) {
            QCOMPARE(p.type(), Protocol::TypeRequest);
            QCOMPARE(p.requestId(), Protocol::RequestActionOrder);
            QCOMPARE(p.value(), QStringLiteral("Fsu0413"));

            // This packet is TypeRequest so notifyId should get invalid
            QCOMPARE(p.notifyId(), Protocol::NotifyInvalid);
        }
    }
    void QMdmmPacketfromJsonhasError2()
    {
        {
            // coverage for the no-errorString overload of hasError()
            (void)Packet::fromJson(QByteArray("some_invalid")).hasError();
        }
        {
            QString actualErrorString;
            (void)Packet::fromJson(QByteArray("some_invalid")).hasError(&actualErrorString);

            bool r = actualErrorString.startsWith(QStringLiteral("Json error: "));
            QVERIFY(r);
        }
        {
            // Check of a notify JSON
            QByteArray input = R"json({"type": 3, "requestId": 0, "notifyId": 8193, "value": "Fsu0413"})json";
            QString actualErrorString;

            Packet p = Packet::fromJson(input);

            QCOMPARE(p.hasError(&actualErrorString), false);
            QVERIFY(actualErrorString.isEmpty());

            if (actualErrorString.isEmpty()) {
                QCOMPARE(p.type(), Protocol::TypeNotify);
                QCOMPARE(p.notifyId(), Protocol::NotifyLogicConfiguration);
                QCOMPARE(p.value(), QStringLiteral("Fsu0413"));

                // This packet is TypeNotify so requestId should get invalid
                QCOMPARE(p.requestId(), Protocol::RequestInvalid);
            }
        }
    }

    void QMdmmPacketQ_DECLARE_METATYPE()
    {
        // coverage for Q_DECLARE_METATYPE

        Packet p(Protocol::TypeRequest, Protocol::RequestRockPaperScissors, {});
        QVariant v = QVariant::fromValue<Packet>(p);
        QCOMPARE(v.value<Packet>().serialize(), p.serialize());
    }
};

namespace {
RegisterTestObject<tst_QMdmmProtocol> _;
}
#include "tst_qmdmmprotocol.moc"
