#include "qmdmmserverplayer.h"
#include "qmdmmserversocket.h"
#include <random>

using std::random_device;

struct QMdmmServerPlayerPrivate
{
    QMdmmServerPlayerPrivate()
        : connectId(0)
        , socket(nullptr)
    {
    }

    int connectId;
    QMdmmServerSocket *socket;
};

QMdmmServerPlayer::QMdmmServerPlayer()
    : d_ptr(new QMdmmServerPlayerPrivate)
{
    QMDMMD(QMdmmServerPlayer);
    random_device rd;
    d->connectId = (rd() % 89999) + 10000;
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

void QMdmmServerPlayer::request(QMdmmProtocol::QMdmmRequestId requestId, const string &requestData)
{
    QMDMMD(QMdmmServerPlayer);
    d->socket->request(requestId, requestData);
}

void QMdmmServerPlayer::notify(QMdmmProtocol::QMdmmNotifyId notifyId, const string &notifyData)
{
    QMDMMD(QMdmmServerPlayer);
    d->socket->notify(notifyId, notifyData);
}

int QMdmmServerPlayer::connectId() const
{
    QMDMMD(const QMdmmServerPlayer);
    return d->connectId;
}
