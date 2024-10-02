// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmprotocol.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSharedData>

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

QMdmmPacket::QMdmmPacket(const QByteArray &serialized)
    : d(new QMdmmPacketData)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(serialized, &err);

    if (err.error != QJsonParseError::NoError) {
        d->error = QStringLiteral("Json error: ").append(err.errorString());
        return;
    }

    if (!doc.isObject()) {
        d->error = QStringLiteral("Document is not object");
        return;
    }

    *d = doc.object();

    if (!d->contains(QStringLiteral("type"))) {
        d->error = QStringLiteral("'type' is non-existent");
        return;
    }
    if (!d->value(QStringLiteral("type")).isDouble()) {
        d->error = QStringLiteral("'type' is not number");
        return;
    }

    if (!d->contains(QStringLiteral("requestId"))) {
        d->error = QStringLiteral("'requestId' is non-existent");
        return;
    }
    if (!d->value(QStringLiteral("requestId")).isDouble()) {
        d->error = QStringLiteral("'requestId' is not number");
        return;
    }

    if (!d->contains(QStringLiteral("notifyId"))) {
        d->error = QStringLiteral("'notifyId' is non-existent");
        return;
    }
    if (!d->value(QStringLiteral("notifyId")).isDouble()) {
        d->error = QStringLiteral("'notifyId' is not number");
        return;
    }

    if (!d->contains(QStringLiteral("value"))) {
        d->error = QStringLiteral("'value' is non-existent");
        return;
    }
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
