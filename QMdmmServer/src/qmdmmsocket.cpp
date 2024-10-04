// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmsocket.h"
#include "qmdmmsocket_p.h"

#include <QLocalSocket>
#include <QTcpSocket>

namespace QMdmmSocketPrivateFactory {
QMdmmSocketPrivate *create(QTcpSocket *t, QMdmmSocket *p)
{
    return new QMdmmSocketPrivateQTcpSocket(t, p);
}
QMdmmSocketPrivate *create(QLocalSocket *l, QMdmmSocket *p)
{
    return new QMdmmSocketPrivateQLocalSocket(l, p);
}
QMdmmSocketPrivate *create(QWebSocket *w, QMdmmSocket *p)
{
    return new QMdmmSocketPrivateQWebSocket(w, p);
}

QMdmmSocketPrivate *create(QMdmmSocket::Type type, QMdmmSocket *p)
{
    switch (type) {
    case QMdmmSocket::TypeQTcpSocket:
        return new QMdmmSocketPrivateQTcpSocket(p);
    case QMdmmSocket::TypeQLocalSocket:
        return new QMdmmSocketPrivateQLocalSocket(p);
    case QMdmmSocket::TypeQWebSocket:
        return new QMdmmSocketPrivateQWebSocket(p);
    default:
        break;
    }

    return nullptr;
}
} // namespace QMdmmSocketPrivateFactory

QMdmmSocket::Type QMdmmSocketPrivate::typeByConnectAddr(const QString &addr)
{
    QUrl u(addr);
    if (!u.isValid())
        return QMdmmSocket::TypeQLocalSocket;
    if (u.scheme() == QStringLiteral("qmdmm") || u.scheme() == QStringLiteral("qmdmms"))
        return QMdmmSocket::TypeQTcpSocket;
    if (u.scheme() == QStringLiteral("ws") || u.scheme() == QStringLiteral("wss"))
        return QMdmmSocket::TypeQWebSocket;

    return QMdmmSocket::TypeUnknown;
}

QMdmmSocketPrivate::QMdmmSocketPrivate(QMdmmSocket *p)
    : QObject(p)
    , p(p)
    , hasError(false)
{
    connect(p, &QMdmmSocket::sendPacket, this, &QMdmmSocketPrivate::sendPacket);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
bool QMdmmSocketPrivate::packetReceived(const QByteArray &arr)
{
    QMdmmPacket packet(arr);
    QString packetError;

    if (packet.hasError(&packetError)) {
        // TODO: make use of this error string
        (void)packetError;

        // Don't process more package for this connection. It is not guaranteed to be the desired client
        p->setHasError(true);
        return false;
    }

    emit p->packetReceived(packet, QMdmmSocket::QPrivateSignal());
    return !hasError;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmSocketPrivate::socketDisconnected()
{
    emit p->socketDisconnected(QMdmmSocket::QPrivateSignal());
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmSocketPrivate::errorOccurred(const QString &errorString)
{
    emit p->socketErrorOccurred(errorString, QMdmmSocket::QPrivateSignal());
}

QMdmmSocketPrivateQTcpSocket::QMdmmSocketPrivateQTcpSocket(QTcpSocket *socket, QMdmmSocket *p)
    : QMdmmSocketPrivate(p)
    , socket(socket)
{
    if (socket != nullptr)
        setupSocket();
}

QMdmmSocketPrivateQTcpSocket::QMdmmSocketPrivateQTcpSocket(QMdmmSocket *p)
    : QMdmmSocketPrivateQTcpSocket(nullptr, p)
{
}

bool QMdmmSocketPrivateQTcpSocket::connectToHost(const QString &addr)
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

bool QMdmmSocketPrivateQTcpSocket::disconnectFromHost()
{
    if (socket != nullptr) {
        socket->disconnectFromHost();
        socket->deleteLater();
        return true;
    }
    return false;
}

void QMdmmSocketPrivateQTcpSocket::setupSocket()
{
    connect(socket, &QTcpSocket::readyRead, this, &QMdmmSocketPrivateQTcpSocket::readyRead);
    connect(this, &QMdmmSocketPrivateQTcpSocket::destroyed, socket, &QTcpSocket::deleteLater);
    connect(socket, &QTcpSocket::errorOccurred, this, &QMdmmSocketPrivateQTcpSocket::errorOccurredTcpSocket);
    connect(socket, &QTcpSocket::disconnected, this, &QMdmmSocketPrivateQTcpSocket::socketDisconnected);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}

void QMdmmSocketPrivateQTcpSocket::sendPacket(QMdmmPacket packet)
{
    if (socket != nullptr) {
        socket->write(packet);
        socket->flush();
    }
}

void QMdmmSocketPrivateQTcpSocket::readyRead()
{
    if (socket != nullptr) {
        while (socket->canReadLine()) {
            QByteArray arr = socket->readLine();
            if (!packetReceived(arr))
                break;
        }
    }
}

void QMdmmSocketPrivateQTcpSocket::errorOccurredTcpSocket(QAbstractSocket::SocketError /*e*/)
{
    if (socket != nullptr)
        errorOccurred(socket->errorString());
}

QMdmmSocketPrivateQLocalSocket::QMdmmSocketPrivateQLocalSocket(QLocalSocket *socket, QMdmmSocket *p)
    : QMdmmSocketPrivate(p)
    , socket(socket)
{
    if (socket != nullptr)
        setupSocket();
}

QMdmmSocketPrivateQLocalSocket::QMdmmSocketPrivateQLocalSocket(QMdmmSocket *p)
    : QMdmmSocketPrivateQLocalSocket(nullptr, p)
{
}

bool QMdmmSocketPrivateQLocalSocket::connectToHost(const QString &addr)
{
    if (socket != nullptr)
        socket->deleteLater();

    socket = new QLocalSocket(this);
    setupSocket();
    socket->connectToServer(addr);
    return true;
}

bool QMdmmSocketPrivateQLocalSocket::disconnectFromHost()
{
    if (socket != nullptr) {
        socket->disconnectFromServer();
        socket->deleteLater();
        return true;
    }
    return false;
}

void QMdmmSocketPrivateQLocalSocket::setupSocket()
{
    connect(socket, &QLocalSocket::readyRead, this, &QMdmmSocketPrivateQLocalSocket::readyRead);
    connect(this, &QMdmmSocketPrivateQLocalSocket::destroyed, socket, &QLocalSocket::deleteLater);
    connect(socket, &QLocalSocket::errorOccurred, this, &QMdmmSocketPrivateQLocalSocket::errorOccurredLocalSocket);
    connect(socket, &QLocalSocket::disconnected, this, &QMdmmSocketPrivateQLocalSocket::socketDisconnected);
    connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
}

void QMdmmSocketPrivateQLocalSocket::sendPacket(QMdmmPacket packet)
{
    if (socket != nullptr) {
        socket->write(packet);
        socket->flush();
    }
}

void QMdmmSocketPrivateQLocalSocket::readyRead()
{
    if (socket != nullptr) {
        while (socket->canReadLine()) {
            QByteArray arr = socket->readLine();
            if (!packetReceived(arr))
                break;
        }
    }
}

void QMdmmSocketPrivateQLocalSocket::errorOccurredLocalSocket(QLocalSocket::LocalSocketError /*e*/)
{
    if (socket != nullptr)
        errorOccurred(socket->errorString());
}

QMdmmSocketPrivateQWebSocket::QMdmmSocketPrivateQWebSocket(QWebSocket *socket, QMdmmSocket *p)
    : QMdmmSocketPrivate(p)
    , socket(socket)
{
    if (socket != nullptr)
        setupSocket();
}

QMdmmSocketPrivateQWebSocket::QMdmmSocketPrivateQWebSocket(QMdmmSocket *p)
    : QMdmmSocketPrivateQWebSocket(nullptr, p)
{
}

bool QMdmmSocketPrivateQWebSocket::connectToHost(const QString &addr)
{
    if (socket != nullptr)
        socket->deleteLater();

    socket = new QWebSocket(QStringLiteral("qmdmm.com"), QWebSocketProtocol::VersionLatest, this);
    setupSocket();
    QUrl url(addr);
    socket->open(url);

    return true;
}

bool QMdmmSocketPrivateQWebSocket::disconnectFromHost()
{
    if (socket != nullptr) {
        socket->close();
        socket->deleteLater();
        return true;
    }
    return false;
}

void QMdmmSocketPrivateQWebSocket::setupSocket()
{
    connect(socket, &QWebSocket::binaryMessageReceived, this, &QMdmmSocketPrivateQWebSocket::packetReceived);
    connect(this, &QMdmmSocketPrivateQWebSocket::destroyed, socket, &QWebSocket::deleteLater);
    connect(socket,
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
            // QTcpSocket and QLocalSocket introduced 'errorOccurred' signal in 5.15 and removed 'error' as a signal in 6.0
            // but QWebSocket introduced 'errorOccurred' in 6.5 so overloaded 'error' can only be used back then.
            qOverload<QAbstractSocket::SocketError>(&QWebSocket::error),
#else
            &QWebSocket::errorOccurred,
#endif
            this, &QMdmmSocketPrivateQWebSocket::errorOccurredWebSocket);
    connect(socket, &QWebSocket::disconnected, this, &QMdmmSocketPrivateQWebSocket::socketDisconnected);
    connect(socket, &QWebSocket::disconnected, socket, &QWebSocket::deleteLater);
}

void QMdmmSocketPrivateQWebSocket::sendPacket(QMdmmPacket packet)
{
    if (socket != nullptr)
        socket->sendBinaryMessage(packet);
}

void QMdmmSocketPrivateQWebSocket::errorOccurredWebSocket(QAbstractSocket::SocketError /*e*/)
{
    if (socket != nullptr)
        errorOccurred(socket->errorString());
}

QMdmmSocket::QMdmmSocket(QTcpSocket *t, QObject *parent)
    : QObject(parent)
    , d(QMdmmSocketPrivateFactory::create(t, this))
{
}

QMdmmSocket::QMdmmSocket(QLocalSocket *l, QObject *parent)
    : QObject(parent)
    , d(QMdmmSocketPrivateFactory::create(l, this))
{
}

QMdmmSocket::QMdmmSocket(QWebSocket *w, QObject *parent)
    : QObject(parent)
    , d(QMdmmSocketPrivateFactory::create(w, this))
{
}

QMdmmSocket::QMdmmSocket(QObject *parent)
    : QObject(parent)
    , d(nullptr)
{
}

// No need to delete d.
QMdmmSocket::~QMdmmSocket() = default;

void QMdmmSocket::setHasError(bool hasError)
{
    if (d != nullptr) {
        d->hasError = hasError;
        if (hasError)
            d->disconnectFromHost();
    }
}

bool QMdmmSocket::hasError() const
{
    if (d != nullptr)
        return d->hasError;

    return true;
}

bool QMdmmSocket::connectToHost(const QString &host)
{
    Type t = QMdmmSocketPrivate::typeByConnectAddr(host);
    if (t == TypeUnknown)
        return false;

    if (d != nullptr) {
        d->disconnectFromHost();
        d->deleteLater();
    }

    d = QMdmmSocketPrivateFactory::create(t, this);
    return d->connectToHost(host);
}
