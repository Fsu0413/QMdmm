// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmserverplayer.h"
#include "qmdmmserversocket.h"
#include <random>

using std::random_device;

struct QMdmmServerPlayerPrivate
{
    QMdmmServerPlayerPrivate()
        : connectionId(0)
        , socket(nullptr)
    {
    }

    int connectionId;
    QMdmmServerSocket *socket;
};

QMdmmServerPlayer::QMdmmServerPlayer()
    : d_ptr(new QMdmmServerPlayerPrivate)
{
    QMDMMD(QMdmmServerPlayer);
    random_device rd;
    d->connectionId = (rd() % 89999) + 10000;
}

QMdmmServerPlayer::~QMdmmServerPlayer()
{
    QMDMMD(QMdmmServerPlayer);
    delete d;
}

void QMdmmServerPlayer::setSocket(QMdmmServerSocket *socket)
{
    QMDMMD(QMdmmServerPlayer);
    d->socket = socket;
}

QMdmmServerSocket *QMdmmServerPlayer::socket() const
{
    QMDMMD(const QMdmmServerPlayer);
    return d->socket;
}

void QMdmmServerPlayer::request(QMdmmProtocol::QMdmmRequestId requestId, const Json::Value &requestData)
{
    QMDMMD(QMdmmServerPlayer);
    d->socket->request(requestId, requestData);
}

void QMdmmServerPlayer::notify(QMdmmProtocol::QMdmmNotifyId notifyId, const Json::Value &notifyData)
{
    QMDMMD(QMdmmServerPlayer);
    d->socket->notify(notifyId, notifyData);
}

int QMdmmServerPlayer::connectionId() const
{
    QMDMMD(const QMdmmServerPlayer);
    return d->connectionId;
}
