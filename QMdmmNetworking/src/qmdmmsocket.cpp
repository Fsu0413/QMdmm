// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmsocket.h"
#include "qmdmmsocket_p.h"

#include <QLocalSocket>
#include <QTcpSocket>

namespace QMdmmNetworking {
namespace p {

namespace SocketPFactory {
SocketP *create(QTcpSocket *t, Socket *p)
{
    return new SocketP_QTcpSocket(t, p);
}
SocketP *create(QLocalSocket *l, Socket *p)
{
    return new SocketP_QLocalSocket(l, p);
}
SocketP *create(QWebSocket *w, Socket *p)
{
    return new SocketP_QWebSocket(w, p);
}

SocketP *create(Socket::Type type, Socket *p)
{
    switch (type) {
    case Socket::TypeQTcpSocket:
        return new SocketP_QTcpSocket(p);
    case Socket::TypeQLocalSocket:
        return new SocketP_QLocalSocket(p);
    case Socket::TypeQWebSocket:
        return new SocketP_QWebSocket(p);
    default:
        break;
    }

    return nullptr;
}
} // namespace SocketPFactory

Socket::Type SocketP::typeByConnectAddr(const QString &addr)
{
    QUrl u(addr);
    if (!u.isValid())
        return Socket::TypeQLocalSocket;
    if (u.scheme() == QStringLiteral("qmdmm") || u.scheme() == QStringLiteral("qmdmms"))
        return Socket::TypeQTcpSocket;
    if (u.scheme() == QStringLiteral("ws") || u.scheme() == QStringLiteral("wss"))
        return Socket::TypeQWebSocket;

    return Socket::TypeUnknown;
}

SocketP::SocketP(Socket *q)
    : QObject(q)
    , q(q)
    , hasError(false)
{
    connect(q, &Socket::sendPacket, this, &SocketP::sendPacket);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
bool SocketP::packetReceived(const QByteArray &arr)
{
    QString packetError;
    QMdmmCore::Packet packet = QMdmmCore::Packet::fromJson(arr, &packetError);

    if (packet.hasError()) {
        // TODO: make use of this error string
        (void)packetError;

        // Don't process more package for this connection. It is not guaranteed to be the desired client
        q->setHasError(true);
        return false;
    }

    emit q->packetReceived(packet, Socket::QPrivateSignal());
    return !hasError;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SocketP::socketDisconnected()
{
    emit q->socketDisconnected(Socket::QPrivateSignal());
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SocketP::errorOccurred(const QString &errorString)
{
    emit q->socketErrorOccurred(errorString, Socket::QPrivateSignal());
}

SocketP_QTcpSocket::SocketP_QTcpSocket(QTcpSocket *socket, Socket *q)
    : SocketP(q)
    , socket(socket)
{
    if (socket != nullptr)
        setupSocket();
}

SocketP_QTcpSocket::SocketP_QTcpSocket(Socket *q)
    : SocketP_QTcpSocket(nullptr, q)
{
}

bool SocketP_QTcpSocket::connectToHost(const QString &addr)
{
    if (socket != nullptr)
        socket->deleteLater();

    socket = new QTcpSocket(this);
    setupSocket();
    QUrl u(addr);
    QString host = u.host();
    uint16_t port = u.port(6366);

    socket->connectToHost(host, port);
    return true;
}

bool SocketP_QTcpSocket::disconnectFromHost()
{
    if (socket != nullptr) {
        socket->disconnectFromHost();
        socket->deleteLater();
        return true;
    }
    return false;
}

void SocketP_QTcpSocket::setupSocket()
{
    connect(socket, &QTcpSocket::readyRead, this, &SocketP_QTcpSocket::readyRead);
    connect(this, &SocketP_QTcpSocket::destroyed, socket, &QTcpSocket::deleteLater);
    connect(socket, &QTcpSocket::errorOccurred, this, &SocketP_QTcpSocket::errorOccurredTcpSocket);
    connect(socket, &QTcpSocket::disconnected, this, &SocketP_QTcpSocket::socketDisconnected);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}

void SocketP_QTcpSocket::sendPacket(QMdmmCore::Packet packet)
{
    if (socket != nullptr) {
        socket->write(QByteArray(packet).append("\n"));
        socket->flush();
    }
}

void SocketP_QTcpSocket::readyRead()
{
    if (socket != nullptr) {
        while (socket->canReadLine()) {
            QByteArray arr = socket->readLine();
            if (!packetReceived(arr))
                break;
        }
    }
}

void SocketP_QTcpSocket::errorOccurredTcpSocket(QAbstractSocket::SocketError /*e*/)
{
    if (socket != nullptr)
        errorOccurred(socket->errorString());
}

SocketP_QLocalSocket::SocketP_QLocalSocket(QLocalSocket *socket, Socket *q)
    : SocketP(q)
    , socket(socket)
{
    if (socket != nullptr)
        setupSocket();
}

SocketP_QLocalSocket::SocketP_QLocalSocket(Socket *q)
    : SocketP_QLocalSocket(nullptr, q)
{
}

bool SocketP_QLocalSocket::connectToHost(const QString &addr)
{
    if (socket != nullptr)
        socket->deleteLater();

    socket = new QLocalSocket(this);
    setupSocket();
    socket->connectToServer(addr);
    return true;
}

bool SocketP_QLocalSocket::disconnectFromHost()
{
    if (socket != nullptr) {
        socket->disconnectFromServer();
        socket->deleteLater();
        return true;
    }
    return false;
}

void SocketP_QLocalSocket::setupSocket()
{
    connect(socket, &QLocalSocket::readyRead, this, &SocketP_QLocalSocket::readyRead);
    connect(this, &SocketP_QLocalSocket::destroyed, socket, &QLocalSocket::deleteLater);
    connect(socket, &QLocalSocket::errorOccurred, this, &SocketP_QLocalSocket::errorOccurredLocalSocket);
    connect(socket, &QLocalSocket::disconnected, this, &SocketP_QLocalSocket::socketDisconnected);
    connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
}

void SocketP_QLocalSocket::sendPacket(QMdmmCore::Packet packet)
{
    if (socket != nullptr) {
        socket->write(QByteArray(packet).append("\n"));
        socket->flush();
    }
}

void SocketP_QLocalSocket::readyRead()
{
    if (socket != nullptr) {
        while (socket->canReadLine()) {
            QByteArray arr = socket->readLine();
            if (!packetReceived(arr))
                break;
        }
    }
}

void SocketP_QLocalSocket::errorOccurredLocalSocket(QLocalSocket::LocalSocketError /*e*/)
{
    if (socket != nullptr)
        errorOccurred(socket->errorString());
}

SocketP_QWebSocket::SocketP_QWebSocket(QWebSocket *socket, Socket *q)
    : SocketP(q)
    , socket(socket)
{
    if (socket != nullptr)
        setupSocket();
}

SocketP_QWebSocket::SocketP_QWebSocket(Socket *q)
    : SocketP_QWebSocket(nullptr, q)
{
}

bool SocketP_QWebSocket::connectToHost(const QString &addr)
{
    if (socket != nullptr)
        socket->deleteLater();

    socket = new QWebSocket(QStringLiteral("qmdmm.com"), QWebSocketProtocol::VersionLatest, this);
    setupSocket();
    QUrl url(addr);
    socket->open(url);

    return true;
}

bool SocketP_QWebSocket::disconnectFromHost()
{
    if (socket != nullptr) {
        socket->close();
        socket->deleteLater();
        return true;
    }
    return false;
}

void SocketP_QWebSocket::setupSocket()
{
    connect(socket, &QWebSocket::binaryMessageReceived, this, &SocketP_QWebSocket::packetReceived);
    connect(this, &SocketP_QWebSocket::destroyed, socket, &QWebSocket::deleteLater);
    connect(socket, &QWebSocket::errorOccurred, this, &SocketP_QWebSocket::errorOccurredWebSocket);
    connect(socket, &QWebSocket::disconnected, this, &SocketP_QWebSocket::socketDisconnected);
    connect(socket, &QWebSocket::disconnected, socket, &QWebSocket::deleteLater);
}

void SocketP_QWebSocket::sendPacket(QMdmmCore::Packet packet)
{
    if (socket != nullptr)
        socket->sendBinaryMessage(packet);
}

void SocketP_QWebSocket::errorOccurredWebSocket(QAbstractSocket::SocketError /*e*/)
{
    if (socket != nullptr)
        errorOccurred(socket->errorString());
}
} // namespace p
namespace v0 {

Socket::Socket(QTcpSocket *t, QObject *parent)
    : QObject(parent)
    , d(p::SocketPFactory::create(t, this))
{
}

Socket::Socket(QLocalSocket *l, QObject *parent)
    : QObject(parent)
    , d(p::SocketPFactory::create(l, this))
{
}

Socket::Socket(QWebSocket *w, QObject *parent)
    : QObject(parent)
    , d(p::SocketPFactory::create(w, this))
{
}

Socket::Socket(QObject *parent)
    : QObject(parent)
    , d(nullptr)
{
}

// No need to delete d.
Socket::~Socket() = default;

void Socket::setHasError(bool hasError)
{
    if (d != nullptr) {
        d->hasError = hasError;
        if (hasError)
            d->disconnectFromHost();
    }
}

bool Socket::hasError() const
{
    if (d != nullptr)
        return d->hasError;

    return true;
}

bool Socket::connectToHost(const QString &host)
{
    Type t = p::SocketP::typeByConnectAddr(host);
    if (t == TypeUnknown)
        return false;

    if (d != nullptr) {
        d->disconnectFromHost();
        d->deleteLater();
    }

    d = p::SocketPFactory::create(t, this);
    if (d == nullptr)
        return false;
    return d->connectToHost(host);
}

} // namespace v0
} // namespace QMdmmNetworking
