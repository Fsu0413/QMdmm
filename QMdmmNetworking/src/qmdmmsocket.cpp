// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmsocket.h"
#include "qmdmmsocket_p.h"

#include <QLocalSocket>
#include <QTcpSocket>

/**
 * @file qmdmmsocket.h
 * @brief This is the file where the networking Socket is defined.
 */

namespace QMdmmNetworking {
#ifndef DOXYGEN
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
{
    connect(q, &Socket::sendPacket, this, &SocketP::sendPacket);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
bool SocketP::packetReceived(const QByteArray &arr)
{
    QMdmmCore::Packet packet = QMdmmCore::Packet::fromJson(arr);

    QString packetError;
    if (packet.hasError(&packetError)) {
        // Don't process more package for this connection. It is not guaranteed to be the desired client
        const Socket::Error error {Socket::ProtocolError, packetError};
        errorOccurred(error);
        q->setError(error);
        return false;
    }

    emit q->packetReceived(packet, Socket::QPrivateSignal());
    return !error.has_value();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SocketP::socketDisconnected()
{
    emit q->socketDisconnected(Socket::QPrivateSignal());
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SocketP::errorOccurred(Socket::Error error)
{
    this->error = error;
    emit q->socketErrorOccurred(error, Socket::QPrivateSignal());
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
        errorOccurred(Socket::Error {Socket::TransportError, socket->errorString()});
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
        errorOccurred(Socket::Error {Socket::TransportError, socket->errorString()});
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
        errorOccurred(Socket::Error {Socket::TransportError, socket->errorString()});
}
} // namespace p
#endif

#ifndef DOXYGEN
namespace v0 {
#endif

/**
 * @class Socket
 * @brief A wrapper around a concrete socket that serializes / deserializes packets.
 *
 * Socket abstracts over TCP socket, local socket and WebSocket transports. It exposes
 * packet-based signals so that the upper layers (@c Server, @c Client, @c Agent) can
 * work with @c QMdmmCore::Packet without caring about the underlying transport.
 */

/**
 * @enum Socket::Type
 * @brief The type of the underlying transport.
 */

/**
 * @var Socket::Type Socket::TypeUnknown
 * @brief Unknown or invalid type.

 * @var Socket::Type Socket::TypeQTcpSocket
 * @brief TCP socket transport.

 * @var Socket::Type Socket::TypeQLocalSocket
 * @brief Local socket transport.

 * @var Socket::Type Socket::TypeQWebSocket
 * @brief WebSocket transport.
 */

/**
 * @enum Socket::ErrorCode
 * @brief The kind of error a Socket can be in. Kept as a stable, transport-agnostic code rather than
 * a mapping of each transport's native error enums, so it does not change across Qt versions.
 */

/**
 * @var Socket::ErrorCode Socket::NoError
 * @brief No error occurred.

 * @var Socket::ErrorCode Socket::TransportError
 * @brief The underlying transport reported an error.

 * @var Socket::ErrorCode Socket::ProtocolError
 * @brief A malformed / invalid packet was received, or a protocol violation.
 */

/**
 * @struct Socket::Error
 * @brief A value type describing a socket error: a stable code plus a human-readable description.
 */

/**
 * @var Socket::Error::code
 * @brief The stable error code.

 * @var Socket::Error::errorString
 * @brief A human-readable description of the error.
 */

/**
 * @brief ctor for server side, wrapping an already-open TCP socket
 * @param t the TCP socket
 * @param parent QObject parent.
 */
Socket::Socket(QTcpSocket *t, QObject *parent)
    : QObject(parent)
    , d(p::SocketPFactory::create(t, this))
{
}

/**
 * @brief ctor for server side, wrapping an already-open local socket
 * @param l the local socket
 * @param parent QObject parent.
 */
Socket::Socket(QLocalSocket *l, QObject *parent)
    : QObject(parent)
    , d(p::SocketPFactory::create(l, this))
{
}

/**
 * @brief ctor for server side, wrapping an already-open WebSocket
 * @param w the WebSocket
 * @param parent QObject parent.
 */
Socket::Socket(QWebSocket *w, QObject *parent)
    : QObject(parent)
    , d(p::SocketPFactory::create(w, this))
{
}

/**
 * @brief ctor for client side
 * @param parent QObject parent.
 *
 * The underlying transport is created lazily by @c connectToHost() based on the address.
 */
Socket::Socket(QObject *parent)
    : QObject(parent)
    , d(nullptr)
{
}

// No need to delete d.
/**
 * @brief dtor.
 */
Socket::~Socket() = default;

/**
 * @brief The transport type of this socket.
 * @return the underlying transport type, or @c TypeUnknown if no transport is active yet (a
 *         client-side socket that has not connected).
 */
Socket::Type Socket::type() const
{
    if (d != nullptr)
        return d->type();

    return TypeUnknown;
}

/**
 * @brief Mark the socket as errored and disconnect the underlying transport.
 * @param error the error to record
 *
 * Stores the error and disconnects. Unlike the transport error path, it does not emit
 * @c socketErrorOccurred: upper layers detect protocol violations through this and learn of the
 * drop via @c socketDisconnected (mirroring the previous @c setHasError behaviour).
 */
void Socket::setError(Error error)
{
    if (d != nullptr) {
        d->error = error;
        d->disconnectFromHost();
    }
}

/**
 * @brief The recorded error, if any.
 * @return the socket's error, or @c std::nullopt if no error has occurred (including before any
 *         transport has been created)
 */
std::optional<Socket::Error> Socket::error() const
{
    if (d != nullptr)
        return d->error;

    return std::nullopt;
}

/**
 * @brief if the socket is in error state
 * @return @c true if the socket has an error
 */
bool Socket::hasError() const
{
    return error().has_value();
}

/**
 * @brief Connect to a host
 * @param host the address to connect to. The scheme decides the transport: @c qmdmm /
 *             @c qmdmms for TCP, @c ws / @c wss for WebSocket, and a plain (non-URL)
 *             string for local socket.
 * @return @c true if the connection is initiated successfully, @c false if the address
 *         cannot be parsed to a known transport
 */
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

/**
 * @brief Actively disconnect the underlying transport.
 *
 * Disconnects the underlying socket (and releases it) if one is currently active. The
 * wrapper itself stays usable: a later @c connectToHost() creates a fresh transport.
 */
void Socket::disconnectFromHost()
{
    if (d != nullptr)
        d->disconnectFromHost();
}

/**
 * @fn Socket::sendPacket(QMdmmCore::Packet packet)
 * @brief emitted when a packet should be sent to the peer
 * @param packet the packet to be sent
 */

/**
 * @fn Socket::packetReceived(QMdmmCore::Packet packet, QPrivateSignal)
 * @brief emitted when a complete packet is received from the peer
 * @param packet the received packet
 */

/**
 * @fn Socket::socketErrorOccurred(Error error, QPrivateSignal)
 * @brief emitted when a socket error occurs
 * @param error the error, carrying a stable code and a human-readable description
 */

/**
 * @fn Socket::socketDisconnected(QPrivateSignal)
 * @brief emitted when the socket is disconnected
 */

#ifndef DOXYGEN
} // namespace v0
#endif
} // namespace QMdmmNetworking
