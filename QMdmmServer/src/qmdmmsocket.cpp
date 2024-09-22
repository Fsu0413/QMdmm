// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmsocket.h"
#include "qmdmmsocket_p.h"

#include <QLocalSocket>
#include <QTcpSocket>

QMdmmSocketPrivate::QMdmmSocketPrivate(QIODevice *socket, QMdmmSocket::Type type, QMdmmSocket *p)
    : QObject(p)
    , socket(socket)
    , p(p)
    , hasError(false)
    , type(type)
{
    connect(socket, &QIODevice::readyRead, this, &QMdmmSocketPrivate::messageReceived);
    connect(p, &QMdmmSocket::sendPacket, this, &QMdmmSocketPrivate::sendPacket);
    connect(this, &QMdmmSocketPrivate::destroyed, socket, &QIODevice::deleteLater);

    switch (type) {
    case QMdmmSocket::TypeQTcpSocket: {
        QTcpSocket *tcpSocket = static_cast<QTcpSocket *>(socket);
        connect(tcpSocket, &QTcpSocket::disconnected, p, &QMdmmSocket::disconnected);
        break;
    }
    case QMdmmSocket::TypeQLocalSocket: {
        QLocalSocket *localSocket = static_cast<QLocalSocket *>(socket);
        connect(localSocket, &QLocalSocket::disconnected, p, &QMdmmSocket::disconnected);

        break;
    }
    default:
        break;
    }
}

void QMdmmSocketPrivate::sendPacket(QMdmmPacket packet) // NOLINT(readability-make-member-function-const)
{
    if (socket == nullptr)
        return;

    socket->write(packet);

    switch (type) {
    case QMdmmSocket::TypeQTcpSocket: {
        static_cast<QTcpSocket *>(socket.data())->flush();
        break;
    }
    case QMdmmSocket::TypeQLocalSocket: {
        static_cast<QLocalSocket *>(socket.data())->flush();
        break;
    }
    default:
        break;
    }
}

void QMdmmSocketPrivate::messageReceived() // NOLINT(readability-make-member-function-const)
{
    if (socket == nullptr)
        return;

    while (socket->canReadLine()) {
        QByteArray arr = socket->readLine();
        QMdmmPacket packet(arr);
        QString packetError;
        bool packetHasError = packet.hasError(&packetError);

        if (packetHasError) {
            // TODO: make use of this error string
            (void)packetError;

            // Don't process more package for this connection. It is not guaranteed to be the desired client
            p->setHasError(true);
            break;
        }

        emit p->packetReceived(packet, QMdmmSocket::QPrivateSignal());
        if (hasError)
            break;
    }
}

QMdmmSocket::QMdmmSocket(QIODevice *socket, Type type, QObject *parent)
    : QObject(parent)
    , d(new QMdmmSocketPrivate(socket, type, this))
{
}

// No need to delete d.
QMdmmSocket::~QMdmmSocket() = default;

void QMdmmSocket::setHasError(bool hasError)
{
    d->hasError = hasError;

    // QIODevice::close() disconnects socket - it is virtual and inherited in QAbstractSocket!
    if (hasError && d->socket != nullptr)
        d->socket->close();
}

bool QMdmmSocket::hasError() const
{
    return d->hasError;
}
