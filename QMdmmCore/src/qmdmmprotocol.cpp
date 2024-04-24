// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmprotocol.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

struct QMdmmPacketPrivate
{
    QMdmmPacketPrivate()
        : type(QMdmmPacket::TypeInvalid)
        , requestId(QMdmmProtocol::RequestInvalid)
        , notifyId(QMdmmProtocol::NotifyInvalid)
    {
    }

    QMdmmPacket::Type type;
    QJsonValue v;
    QMdmmProtocol::QMdmmRequestId requestId;
    QMdmmProtocol::QMdmmNotifyId notifyId;

    QString error;

    // static Json::StreamWriter *getJsonWriter();
    // static Json::CharReader *getJsonReader();
};

// Json::StreamWriter *QMdmmPacketPrivate::getJsonWriter()
// {
//     static std::unique_ptr<Json::StreamWriter> theWriter([]() {
//         Json::StreamWriterBuilder builder;
//         Json::StreamWriterBuilder::setDefaults(&builder.settings_);
//         builder["emitUTF8"] = true;
//         builder["indentation"] = "";
//         return builder.newStreamWriter();
//     }());
//     return theWriter.get();
// }

// Json::CharReader *QMdmmPacketPrivate::getJsonReader()
// {
//     static std::unique_ptr<Json::CharReader> theReader([]() {
//         Json::CharReaderBuilder builder;
//         Json::CharReaderBuilder::strictMode(&builder.settings_);
//         builder["failIfExtra"] = false;

//         return builder.newCharReader();
//     }());
//     return theReader.get();
// }

QMdmmPacket::QMdmmPacket(Type type, QMdmmProtocol::QMdmmRequestId requestId, const QJsonValue &value)
    : d(new QMdmmPacketPrivate)
{
    d->type = type;
    d->v = value;
    d->requestId = requestId;
}

QMdmmPacket::QMdmmPacket(QMdmmProtocol::QMdmmNotifyId notifyId, const QJsonValue &value)
    : d(new QMdmmPacketPrivate)
{
    d->type = QMdmmPacket::TypeNotify;
    d->v = value;
    d->notifyId = notifyId;
}

QMdmmPacket::QMdmmPacket(const QMdmmPacket &package)
    : d(new QMdmmPacketPrivate)
{
    d->type = package.d->type;
    d->v = package.d->v;
    d->requestId = package.d->requestId;
    d->notifyId = package.d->notifyId;
}

QMdmmPacket::QMdmmPacket(const QByteArray &serialized)
    : d(new QMdmmPacketPrivate)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(serialized, &err);

    if (err.error != QJsonParseError::NoError) {
        d->error = err.errorString();
        return;
    }

    if (!doc.isObject()) {
        d->error = QStringLiteral("Document is not object");
        return;
    }

    QJsonObject ob = doc.object();

    d->type = static_cast<Type>(ob.value(QStringLiteral("type")).toInt());
    d->requestId = static_cast<QMdmmProtocol::QMdmmRequestId>(ob.value(QStringLiteral("requestId")).toInt());
    d->notifyId = static_cast<QMdmmProtocol::QMdmmNotifyId>(ob.value(QStringLiteral("notifyId")).toInt());
    d->v = ob.value(QStringLiteral("value"));
}

QMdmmPacket &QMdmmPacket::operator=(const QMdmmPacket &package)
{
    d->type = package.d->type;
    d->v = package.d->v;
    d->requestId = package.d->requestId;
    d->notifyId = package.d->notifyId;

    return *this;
}

QMdmmPacket::Type QMdmmPacket::type() const
{
    return d->type;
}

QMdmmProtocol::QMdmmRequestId QMdmmPacket::requestId() const
{
    return d->requestId;
}

QMdmmProtocol::QMdmmNotifyId QMdmmPacket::notifyId() const
{
    return d->notifyId;
}

QJsonValue QMdmmPacket::value() const
{
    return d->v;
}

QByteArray QMdmmPacket::serialize() const
{
    QJsonObject ob;
    ob.insert(QStringLiteral("type"), static_cast<int>(d->type));
    ob.insert(QStringLiteral("requestId"), static_cast<int>(d->requestId));
    ob.insert(QStringLiteral("notifyId"), static_cast<int>(d->notifyId));
    ob.insert(QStringLiteral("value"), d->v);

    QJsonDocument doc(ob);
    return doc.toJson(QJsonDocument::Compact);
}

bool QMdmmPacket::hasError(QString *errorString) const
{
    if (errorString != nullptr)
        *errorString = d->error;

    return !d->error.isEmpty();
}
