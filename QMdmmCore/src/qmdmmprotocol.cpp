// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmprotocol.h"
#include "json/json.h"

#include <sstream>
using std::ostringstream;
using std::string;

struct QMdmmPacketPrivate
{
    QMdmmPacketPrivate()
        : type(QMdmmPacket::TypeInvalid)
        , requestId(QMdmmProtocol::RequestInvalid)
        , notifyId(QMdmmProtocol::NotifyInvalid)
    {
    }

    QMdmmPacket::Type type;
    Json::Value v;
    QMdmmProtocol::QMdmmRequestId requestId;
    QMdmmProtocol::QMdmmNotifyId notifyId;

    string error;
};

QMdmmPacket::QMdmmPacket(Type type, QMdmmProtocol::QMdmmRequestId requestId, const Json::Value &value)
    : d(new QMdmmPacketPrivate)
{
    d->type = type;
    d->v = value;
    d->requestId = requestId;
}

QMdmmPacket::QMdmmPacket(QMdmmProtocol::QMdmmNotifyId notifyId, const Json::Value &value)
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

QMdmmPacket::QMdmmPacket(const string &str)
    : d(new QMdmmPacketPrivate)
{
    Json::Reader reader;
    Json::Value value;
    if (reader.parse(str, value, false)) {
        d->type = static_cast<Type>(value["type"].asInt());
        d->requestId = static_cast<QMdmmProtocol::QMdmmRequestId>(value["requestId"].asInt());
        d->notifyId = static_cast<QMdmmProtocol::QMdmmNotifyId>(value["notifyId"].asInt());
        d->v = value["value"];
    } else {
        d->error = reader.getFormattedErrorMessages();
    }
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

Json::Value QMdmmPacket::value() const
{
    return d->v;
}

std::string QMdmmPacket::toString() const
{
    Json::Value root;
    root["type"] = static_cast<int>(d->type);
    root["requestId"] = static_cast<int>(d->requestId);
    root["notifyId"] = static_cast<int>(d->notifyId);
    root["value"] = d->v;
    Json::FastWriter writer;
    return writer.write(root);
}

bool QMdmmPacket::hasError(std::string *errorString) const
{
    if (errorString != nullptr)
        *errorString = d->error;

    return !d->error.empty();
}
