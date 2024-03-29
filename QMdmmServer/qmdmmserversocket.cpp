// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmserversocket.h"
#include "qmdmmserver.h"
#include <QMdmmCore/QMdmmProtocol>

using std::string;

struct QMdmmServerSocketPrivate
{
    QMdmmServer *server;

    QMdmmServerSocketPrivate()
        : server(nullptr)
    {
    }
};

QMdmmServerSocket::QMdmmServerSocket()
    : d(new QMdmmServerSocketPrivate)
{
}

QMdmmServerSocket::~QMdmmServerSocket()
{
    delete d;
}

void QMdmmServerSocket::setServer(QMdmmServer *server)
{
    d->server = server;
}

QMdmmServer *QMdmmServerSocket::getServer() const
{
    return d->server;
}

void QMdmmServerSocket::request(QMdmmProtocol::QMdmmRequestId requestId, const Json::Value &requestData)
{
    QMdmmPacket packet(QMdmmPacket::TypeRequest, requestId, requestData);
    if (send(packet.toString())) {
        // do nothing
    } else {
        // ???
    }
}

void QMdmmServerSocket::replyed(QMdmmProtocol::QMdmmRequestId requestId, const Json::Value &replyData)
{
    if (d->server != nullptr)
        d->server->replyToServer(this, requestId, replyData);
}

void QMdmmServerSocket::notify(QMdmmProtocol::QMdmmNotifyId notifyId, const Json::Value &notifyData)
{
    QMdmmPacket packet(notifyId, notifyData);
    if (send(packet.toString())) {
        // do nothing
    } else {
        // ???
    }
}

void QMdmmServerSocket::notified(QMdmmProtocol::QMdmmNotifyId notifyId, const Json::Value &notifyData)
{
    if (d->server != nullptr)
        d->server->notifyServer(this, notifyId, notifyData);
}

void QMdmmServerSocket::received(const string &data)
{
    QMdmmPacket packet(data);
    if (packet.hasError())
        return;

    if (packet.type() == QMdmmPacket::TypeReply)
        replyed(packet.requestId(), packet.value());
    else if (packet.type() == QMdmmPacket::TypeNotify)
        notified(packet.notifyId(), packet.value());
    else {
        // ???
    }
}
