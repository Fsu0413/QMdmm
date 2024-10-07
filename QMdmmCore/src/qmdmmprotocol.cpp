// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmprotocol.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSharedData>

int QMdmmProtocol::protocolVersion()
{
    return 0;
}

QMdmmPacketData::QMdmmPacketData()
{
    insert(QStringLiteral("type"), static_cast<int>(QMdmmProtocol::TypeInvalid));
    insert(QStringLiteral("requestId"), static_cast<int>(QMdmmProtocol::RequestInvalid));
    insert(QStringLiteral("notifyId"), static_cast<int>(QMdmmProtocol::NotifyInvalid));
    insert(QStringLiteral("value"), QJsonValue());
}

QMdmmPacketData::QMdmmPacketData(QMdmmProtocol::PacketType type, QMdmmProtocol::RequestId requestId, QMdmmProtocol::NotifyId notifyId, const QJsonValue &v)
{
    insert(QStringLiteral("type"), static_cast<int>(type));
    insert(QStringLiteral("requestId"), static_cast<int>(requestId));
    insert(QStringLiteral("notifyId"), static_cast<int>(notifyId));
    insert(QStringLiteral("value"), v);
}

QMdmmPacketData::QMdmmPacketData(const QJsonObject &ob)
    : QJsonObject(ob)
{
}

QMdmmPacketData &QMdmmPacketData::operator=(const QJsonObject &ob)
{
    QJsonObject::operator=(ob);
    return *this;
}

QMdmmPacket::QMdmmPacket()
    : d(new QMdmmPacketData)
{
}

QMdmmPacket::QMdmmPacket(QMdmmProtocol::PacketType type, QMdmmProtocol::RequestId requestId, const QJsonValue &value)
    : d(new QMdmmPacketData(type, requestId, QMdmmProtocol::NotifyInvalid, value))
{
}

QMdmmPacket::QMdmmPacket(QMdmmProtocol::NotifyId notifyId, const QJsonValue &value)
    : d(new QMdmmPacketData(QMdmmProtocol::TypeNotify, QMdmmProtocol::RequestInvalid, notifyId, value))
{
}

QMdmmProtocol::PacketType QMdmmPacket::type() const
{
    return static_cast<QMdmmProtocol::PacketType>(d->value(QStringLiteral("type")).toInt(QMdmmProtocol::TypeInvalid));
}

QMdmmProtocol::RequestId QMdmmPacket::requestId() const
{
    return static_cast<QMdmmProtocol::RequestId>(d->value(QStringLiteral("requestId")).toInt(QMdmmProtocol::RequestInvalid));
}

QMdmmProtocol::NotifyId QMdmmPacket::notifyId() const
{
    return static_cast<QMdmmProtocol::NotifyId>(d->value(QStringLiteral("notifyId")).toInt(QMdmmProtocol::NotifyInvalid));
}

QJsonValue QMdmmPacket::value() const
{
    return d->value(QStringLiteral("value"));
}

QByteArray QMdmmPacket::serialize() const
{
    QJsonDocument doc(*d);
    return doc.toJson(QJsonDocument::Compact).append('\n');
}

bool QMdmmPacket::hasError(QString *errorString) const
{
    if (errorString != nullptr)
        *errorString = d->error;

    return !d->error.isEmpty();
}

QMdmmPacket QMdmmPacket::fromJson(const QByteArray &serialized, QString *errorString)
{
    if (errorString == nullptr) {
        static QString _errorString;
        errorString = &_errorString;
    }

    errorString->clear();

    QMdmmPacket ret;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(serialized, &err);

    if (err.error != QJsonParseError::NoError) {
        *errorString = QStringLiteral("Json error: ").append(err.errorString());
        ret.d->error = *errorString;
        return ret;
    }

    if (!doc.isObject()) {
        *errorString = QStringLiteral("Document is not object");
        ret.d->error = *errorString;
        return ret;
    }

    *(ret.d) = doc.object();

    if (!ret.d->contains(QStringLiteral("type"))) {
        *errorString = QStringLiteral("'type' is non-existent");
        ret.d->error = *errorString;
        return ret;
    }
    if (!ret.d->value(QStringLiteral("type")).isDouble()) {
        *errorString = QStringLiteral("'type' is not number");
        ret.d->error = *errorString;
        return ret;
    }

    if (!ret.d->contains(QStringLiteral("requestId"))) {
        *errorString = QStringLiteral("'requestId' is non-existent");
        ret.d->error = *errorString;
        return ret;
    }
    if (!ret.d->value(QStringLiteral("requestId")).isDouble()) {
        *errorString = QStringLiteral("'requestId' is not number");
        ret.d->error = *errorString;
        return ret;
    }

    if (!ret.d->contains(QStringLiteral("notifyId"))) {
        *errorString = QStringLiteral("'notifyId' is non-existent");
        ret.d->error = *errorString;
        return ret;
    }
    if (!ret.d->value(QStringLiteral("notifyId")).isDouble()) {
        *errorString = QStringLiteral("'notifyId' is not number");
        ret.d->error = *errorString;
        return ret;
    }

    if (!ret.d->contains(QStringLiteral("value"))) {
        *errorString = QStringLiteral("'value' is non-existent");
        ret.d->error = *errorString;
        return ret;
    }

    return ret;
}
