#include "qmdmmserversocket.h"
#include "qmdmmserver.h"

struct QMdmmServerSocketPrivate
{
    QMdmmServer *server;

    QMdmmServerSocketPrivate()
        : server(nullptr)
    {
    }
};

QMdmmServerSocket::QMdmmServerSocket()
    : d_ptr(new QMdmmServerSocketPrivate)
{
}

QMdmmServerSocket::~QMdmmServerSocket()
{
    QMDMMD(QMdmmServerSocket);
    delete d;
}

void QMdmmServerSocket::setServer(QMdmmServer *server)
{
    QMDMMD(QMdmmServerSocket);
    d->server = server;
}

QMdmmServer *QMdmmServerSocket::getServer() const
{
    QMDMMD(const QMdmmServerSocket);
    return d->server;
}

void QMdmmServerSocket::request(QMdmmProtocol::QMdmmRequestId requestId, const string &requestData)
{
    sendRequest(requestId, requestData);
}

void QMdmmServerSocket::replyed(QMdmmProtocol::QMdmmRequestId requestId, const string &replyData)
{
    QMDMMD(QMdmmServerSocket);
    if (d->server != nullptr)
        d->server->replyToServer(this, requestId, replyData);
}

void QMdmmServerSocket::notify(QMdmmProtocol::QMdmmNotifyId notifyId, const string &notifyData)
{
    sendNotify(notifyId, notifyData);
}

void QMdmmServerSocket::notified(QMdmmProtocol::QMdmmNotifyId notifyId, const string &notifyData)
{
    QMDMMD(QMdmmServerSocket);
    if (d->server != nullptr)
        d->server->notifyServer(this, notifyId, notifyData);
}
