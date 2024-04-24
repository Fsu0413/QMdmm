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

    static Json::StreamWriter *getJsonWriter();
    static Json::CharReader *getJsonReader();
};

Json::StreamWriter *QMdmmPacketPrivate::getJsonWriter()
{
    static std::unique_ptr<Json::StreamWriter> theWriter([]() {
        Json::StreamWriterBuilder builder;
        Json::StreamWriterBuilder::setDefaults(&builder.settings_);
        builder["emitUTF8"] = true;
        builder["indentation"] = "";
        return builder.newStreamWriter();
    }());
    return theWriter.get();
}

Json::CharReader *QMdmmPacketPrivate::getJsonReader()
{
    static std::unique_ptr<Json::CharReader> theReader([]() {
        Json::CharReaderBuilder builder;
        Json::CharReaderBuilder::strictMode(&builder.settings_);
        builder["failIfExtra"] = false;

        return builder.newCharReader();
    }());
    return theReader.get();
}

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
    Json::CharReader *reader = d->getJsonReader();
    std::string errs;
    Json::Value value;
    if (reader->parse(str.c_str(), str.c_str() + str.length(), &value, &errs)) {
        d->type = static_cast<Type>(value["type"].asInt());
        d->requestId = static_cast<QMdmmProtocol::QMdmmRequestId>(value["requestId"].asInt());
        d->notifyId = static_cast<QMdmmProtocol::QMdmmNotifyId>(value["notifyId"].asInt());
        d->v = value["value"];
    } else {
        d->error = errs;
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
    std::ostringstream oss;
    Json::StreamWriter *writer = d->getJsonWriter();
    if (writer->write(root, &oss) == 0)
        return oss.str();

    d->error = "Error when writing packet to string";
    return {};
}

bool QMdmmPacket::hasError(std::string *errorString) const
{
    if (errorString != nullptr)
        *errorString = d->error;

    return !d->error.empty();
}
