// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmserver.h"
#include "qmdmmserver_p.h"

#include "qmdmmagent.h"

#include <QLocalSocket>
#include <QTcpSocket>

QHash<QMdmmProtocol::NotifyId, bool (QMdmmServerPrivate::*)(QMdmmSocket *, const QJsonValue &)> QMdmmServerPrivate::cb {
    std::make_pair(QMdmmProtocol::NotifyPongClient, &QMdmmServerPrivate::pongClient),
    std::make_pair(QMdmmProtocol::NotifyPingServer, &QMdmmServerPrivate::pingServer),
    std::make_pair(QMdmmProtocol::NotifySignIn, &QMdmmServerPrivate::signIn),
    std::make_pair(QMdmmProtocol::NotifyObserve, &QMdmmServerPrivate::observe),
};

QMdmmServerPrivate::QMdmmServerPrivate(const QMdmmServerConfiguration &serverConfiguration, QMdmmServer *p)
    : QObject(p)
    , serverConfiguration(serverConfiguration)
    , p(p)
    , t(new QTcpServer(this))
    , l(new QLocalServer(this))
    , current(nullptr)
{
    // Local
    {
        l->setSocketOptions(QLocalServer::WorldAccessOption);
        connect(l, &QLocalServer::newConnection, this, &QMdmmServerPrivate::serverNewConnection);
        l->listen(serverConfiguration.localSocketName);
    }

    // Tcp
    {
        connect(t,
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
                // Qt pre-6.3: old behavior
                &QTcpServer::newConnection,
#else
                // Qt post-6.4: new pendingConnectionAvailable signal, emitted after connection is added to pending connection queue instead of the connection is estabilished
                &QTcpServer::pendingConnectionAvailable,
#endif
                this, &QMdmmServerPrivate::serverNewConnection);

        t->listen(QHostAddress::Any, serverConfiguration.tcpPort);
    }
}

bool QMdmmServerPrivate::pongClient(QMdmmSocket *socket, const QJsonValue &packetValue)
{
    return true;
}

bool QMdmmServerPrivate::pingServer(QMdmmSocket *socket, const QJsonValue &packetValue)
{
    return true;
}

bool QMdmmServerPrivate::signIn(QMdmmSocket *socket, const QJsonValue &packetValue)
{
    return false;
}

bool QMdmmServerPrivate::observe(QMdmmSocket *socket, const QJsonValue &packetValue)
{
    // not implemented by now
    return false;
}

void QMdmmServerPrivate::introduceSocket(QMdmmSocket *socket) // NOLINT(readability-make-member-function-const)
{
    connect(socket, &QMdmmSocket::packetReceived, this, &QMdmmServerPrivate::socketPacketReceived);

    QJsonObject ob;
    ob.insert(QStringLiteral("versionNumber"), QMdmmGlobal::version().toString());
    QMdmmPacket packet(QMdmmProtocol::NotifyVersion, ob);
    emit socket->sendPacket(packet);
}

void QMdmmServerPrivate::serverNewConnection() // NOLINT(readability-make-member-function-const)
{
    while (t->hasPendingConnections()) {
        QTcpSocket *socket = t->nextPendingConnection();
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
        QMdmmSocket *mdmmSocket = new QMdmmSocket(socket, QMdmmSocket::TypeQTcpSocket, this);
        introduceSocket(mdmmSocket);
    }

    while (l->hasPendingConnections()) {
        QLocalSocket *socket = l->nextPendingConnection();
        connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
        QMdmmSocket *mdmmSocket = new QMdmmSocket(socket, QMdmmSocket::TypeQLocalSocket, this);
        introduceSocket(mdmmSocket);
    }
}

void QMdmmServerPrivate::socketPacketReceived(QMdmmPacket packet)
{
    QMdmmSocket *socket = qobject_cast<QMdmmSocket *>(sender());

    if (socket == nullptr)
        return;

    if (packet.type() == QMdmmProtocol::TypeNotify) {
        if ((packet.notifyId() | QMdmmProtocol::NotifyToServerMask) != 0) {
            // These packages should be processed in Server
            bool (QMdmmServerPrivate::*call)(QMdmmSocket *, const QJsonValue &) = cb.value(packet.notifyId(), nullptr);
            if (call != nullptr)
                (this->*call)(socket, packet.value());
            else
                socket->setHasError(true);
        }
    }
}

QMdmmServer::QMdmmServer(const QMdmmServerConfiguration &serverConfiguration, QObject *parent)
    : QObject(parent)
    , d(new QMdmmServerPrivate(serverConfiguration, this))
{
}

// No need to delete d.
// It will always be deleted by QObject dtor
QMdmmServer::~QMdmmServer() = default;
