#include "qmdmmprotocol.h"
#include "json/json.h"

#include <sstream>
using std::ostringstream;

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
    : d_ptr(new QMdmmPacketPrivate)
{
    QMDMMD(QMdmmPacket);
    d->type = type;
    d->v = value;
    d->requestId = requestId;
}

QMdmmPacket::QMdmmPacket(QMdmmProtocol::QMdmmNotifyId notifyId, const Json::Value &value)
    : d_ptr(new QMdmmPacketPrivate)
{
    QMDMMD(QMdmmPacket);
    d->type = QMdmmPacket::TypeNotify;
    d->v = value;
    d->notifyId = notifyId;
}

QMdmmPacket::QMdmmPacket(const QMdmmPacket &package)
    : d_ptr(new QMdmmPacketPrivate)
{
    QMDMMD(QMdmmPacket);
    d->type = package.d_func()->type;
    d->v = package.d_func()->v;
    d->requestId = package.d_func()->requestId;
    d->notifyId = package.d_func()->notifyId;
}

QMdmmPacket::QMdmmPacket(const string &str)
    : d_ptr(new QMdmmPacketPrivate)
{
    QMDMMD(QMdmmPacket);
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
    QMDMMD(QMdmmPacket);
    d->type = package.d_func()->type;
    d->v = package.d_func()->v;
    d->requestId = package.d_func()->requestId;
    d->notifyId = package.d_func()->notifyId;

    return *this;
}

QMdmmPacket::Type QMdmmPacket::type() const
{
    QMDMMD(const QMdmmPacket);
    return d->type;
}

QMdmmProtocol::QMdmmRequestId QMdmmPacket::requestId() const
{
    QMDMMD(const QMdmmPacket);
    return d->requestId;
}

QMdmmProtocol::QMdmmNotifyId QMdmmPacket::notifyId() const
{
    QMDMMD(const QMdmmPacket);
    return d->notifyId;
}

Json::Value QMdmmPacket::value() const
{
    QMDMMD(const QMdmmPacket);
    return d->v;
}

std::string QMdmmPacket::toString() const
{
    QMDMMD(const QMdmmPacket);
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
    QMDMMD(const QMdmmPacket);
    if (errorString != nullptr)
        *errorString = d->error;

    return !d->error.empty();
}
