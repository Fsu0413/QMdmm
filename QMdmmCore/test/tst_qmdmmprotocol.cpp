#include "test.h"

#include <QMdmmCore/QMdmmProtocol>

#include <QJsonObject>
#include <QTest>

using namespace Qt::StringLiterals;

// NOLINTBEGIN
// Exempt from clang-tidy by policy; see AGENTS.md.

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
        ob2.insert(u"test"_s, QJsonValue());
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
            QByteArray d = R"json({"notifyId":0,"requestId":1,"type":1,"value":1})json"_ba;

            QCOMPARE(arr, d);
        }
    }

    void QMdmmPacketfromJsonhasError_data()
    {
        QTest::addColumn<QByteArray>("input");
        QTest::addColumn<QString>("errorString");

        QTest::newRow("not-object") << "[1,2,3]"_ba << u"Document is not object"_s;
        QTest::newRow("type-notexist") << "{}"_ba << u"'type' is non-existent"_s;
        QTest::newRow("type-invalid") << R"json({"type": "Fsu0413"})json"_ba << u"'type' is not number"_s;
        QTest::newRow("type-fractional") << R"json({"type": 1.5})json"_ba << u"'type' is not an integer"_s;
        QTest::newRow("type-outofrange") << R"json({"type": 99})json"_ba << u"'type' is out of range"_s;
        QTest::newRow("requestid-notexist") << R"json({"type": 1})json"_ba << u"'requestId' is non-existent"_s;
        QTest::newRow("requestid-invalid") << R"json({"type": 1, "requestId": "Fsu0413"})json"_ba << u"'requestId' is not number"_s;
        QTest::newRow("requestid-fractional") << R"json({"type": 1, "requestId": 2.5})json"_ba << u"'requestId' is not an integer"_s;
        QTest::newRow("requestid-outofrange") << R"json({"type": 1, "requestId": 99})json"_ba << u"'requestId' is out of range"_s;
        QTest::newRow("notifyid-notexist") << R"json({"type": 1, "requestId": 2})json"_ba << u"'notifyId' is non-existent"_s;
        QTest::newRow("notifyid-invalid") << R"json({"type": 1, "requestId": 2, "notifyId": "Fsu0413"})json"_ba << u"'notifyId' is not number"_s;
        QTest::newRow("notifyid-fractional") << R"json({"type": 1, "requestId": 2, "notifyId": 8193.5})json"_ba << u"'notifyId' is not an integer"_s;
        QTest::newRow("notifyid-outofrange") << R"json({"type": 1, "requestId": 2, "notifyId": 99})json"_ba << u"'notifyId' is out of range"_s;
        QTest::newRow("request-requestid-invalid") << R"json({"type": 1, "requestId": 0, "notifyId": 0, "value": null})json"_ba
                                                   << u"'requestId' is invalid for a request/reply packet"_s;
        QTest::newRow("request-notifyid-notinvalid") << R"json({"type": 1, "requestId": 1, "notifyId": 8193, "value": null})json"_ba
                                                     << u"'notifyId' should be invalid for a request/reply packet"_s;
        QTest::newRow("notify-notifyid-invalid") << R"json({"type": 3, "requestId": 0, "notifyId": 0, "value": null})json"_ba << u"'notifyId' is invalid for a notify packet"_s;
        QTest::newRow("notify-requestid-notinvalid") << R"json({"type": 3, "requestId": 1, "notifyId": 8193, "value": null})json"_ba
                                                     << u"'requestId' should be invalid for a notify packet"_s;
        QTest::newRow("value-notexist") << R"json({"type": 1, "requestId": 2, "notifyId": 8193})json"_ba << u"'value' is non-existent"_s;
        QTest::newRow("valid") << R"json({"type": 1, "requestId": 2, "notifyId": 0, "value": "Fsu0413"})json"_ba << QString {};
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
            QCOMPARE(p.value(), u"Fsu0413"_s);

            // This packet is TypeRequest so notifyId should get invalid
            QCOMPARE(p.notifyId(), Protocol::NotifyInvalid);
        }
    }
    void QMdmmPacketfromJsonhasError2()
    {
        {
            // the no-errorString overload of hasError() reports a parse error
            QVERIFY(Packet::fromJson("some_invalid"_ba).hasError());
        }
        {
            QString actualErrorString;
            QVERIFY(Packet::fromJson("some_invalid"_ba).hasError(&actualErrorString));

            bool r = actualErrorString.startsWith(u"Json error: "_s);
            QVERIFY(r);
        }
        {
            // Check of a notify JSON
            QByteArray input = R"json({"type": 3, "requestId": 0, "notifyId": 8193, "value": "Fsu0413"})json"_ba;
            QString actualErrorString;

            Packet p = Packet::fromJson(input);

            QCOMPARE(p.hasError(&actualErrorString), false);
            QVERIFY(actualErrorString.isEmpty());
            // the no-errorString overload agrees with the errorString overload
            QCOMPARE(p.hasError(), false);

            if (actualErrorString.isEmpty()) {
                QCOMPARE(p.type(), Protocol::TypeNotify);
                QCOMPARE(p.notifyId(), Protocol::NotifyLogicConfiguration);
                QCOMPARE(p.value(), u"Fsu0413"_s);

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

// NOLINTEND
