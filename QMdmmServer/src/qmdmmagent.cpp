// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmagent.h"
#include "qmdmmserver.h"

#include <QMdmmCore/QMdmmProtocol>

#include <QPointer>

class QMdmmAgentPrivate : public QObject
{
    Q_OBJECT

public:
    QMdmmAgentPrivate(QMdmmAgent *a)
        : QObject(a)
        , a(a)
        , ready(false)
        , socket(nullptr)
        , isReconnect(false)
    {
    }

    void setSocket(QMdmmSocket *_socket)
    {
        if (socket != nullptr) {
            // not allowed but...

            socket->deleteLater();
        }

        socket = _socket;
        connect(socket, &QMdmmSocket::packetReceived, this, &QMdmmAgentPrivate::packetReceived);
        connect(socket, &QMdmmSocket::disconnected, this, &QMdmmAgentPrivate::socketDisconnected);
        connect(a, &QMdmmAgent::sendPacket, socket, &QMdmmSocket::sendPacket);
    }

    QMdmmAgent *a;
    bool ready;
    QString screenName;
    QPointer<QMdmmSocket> socket;
    bool isReconnect;

public slots: // NOLINT(readability-redundant-access-specifiers)
    void packetReceived(QMdmmPacket packet)
    {
        (void)packet;
    }

    void socketDisconnected()
    {
        isReconnect = true;
        socket->deleteLater();
        socket = nullptr;
    }

    void socketReconnected()
    {
        isReconnect = false;
    }
};

QMdmmAgent::QMdmmAgent(QString name, QObject *parent)
    : QObject(parent)
    , d(new QMdmmAgentPrivate(this))
{
    setObjectName(name);
}

QMdmmAgent::~QMdmmAgent()
{
    // no need
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

void QMdmmAgent::setSocket(QMdmmSocket *socket)
{
    d->setSocket(socket);
}

#include "qmdmmagent.moc"
