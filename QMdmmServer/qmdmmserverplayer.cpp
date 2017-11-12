#include "qmdmmserverplayer.h"
#include "qmdmmsocket.h"
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
    QMdmmSocket *socket;
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

void QMdmmServerPlayer::setSocket(QMdmmSocket *socket)
{
    QMDMMD(QMdmmServerPlayer);
    d->socket = socket;
}

QMdmmSocket *QMdmmServerPlayer::socket() const
{
    QMDMMD(const QMdmmServerPlayer);
    return d->socket;
}

int QMdmmServerPlayer::connectId() const
{
    QMDMMD(const QMdmmServerPlayer);
    return d->connectId;
}
