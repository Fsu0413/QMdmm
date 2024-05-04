#include "qmdmmagent.h"

#include <QMdmmCore/QMdmmProtocol>

#include <QIODevice>

struct QMdmmAgentPrivate
{
    bool ready;
    QString screenName;

    QMdmmAgentPrivate()
        : ready(false)
    {
    }
};

QMdmmAgent::QMdmmAgent(QString name, QObject *parent)
    : QObject(parent)
    , d(new QMdmmAgentPrivate)
{
    setObjectName(name);
}

QMdmmAgent::~QMdmmAgent()
{
    delete d;
}

QString QMdmmAgent::screenName() const
{
    return d->screenName;
}

void QMdmmAgent::setScreenName(const QString &name)
{
    if (name != screenName()) {
        d->screenName = name;
        emit screenNameChanged(name, QPrivateSignal());
    }
}

bool QMdmmAgent::ready() const
{
    return d->ready;
}

void QMdmmAgent::setReady(bool ready)
{
    if (ready != d->ready) {
        d->ready = ready;
        emit readyChanged(ready, QPrivateSignal());
    }
}

void QMdmmAgent::packetReceived(QMdmmPacket packet)
{
    (void)packet;
}

void QMdmmAgent::socketDisconnected()
{
}

void QMdmmAgent::socketReconnected()
{
}
