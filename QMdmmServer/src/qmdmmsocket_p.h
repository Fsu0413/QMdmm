// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSOCKET_P
#define QMDMMSOCKET_P

#include "qmdmmsocket.h"

#include <QObject>
#include <QPointer>

class QMDMMSERVER_PRIVATE_EXPORT QMdmmSocketPrivate : public QObject
{
    Q_OBJECT

public:
    static QMdmmSocket::Type typeByConnectAddr(const QString &addr);

    explicit QMdmmSocketPrivate(QMdmmSocket *p);
    [[nodiscard]] virtual QMdmmSocket::Type type() const = 0;

    virtual bool connectToHost(const QString &addr) = 0;
    virtual bool disconnectFromHost() = 0;

    QMdmmSocket *p;
    bool hasError;

public slots: // NOLINT(readability-redundant-access-specifiers)
    virtual void sendPacket(QMdmmPacket packet) = 0;
    bool packetReceived(const QByteArray &arr);
    void socketDisconnected();
    void errorOccurred(const QString &errorString);
};

class QMDMMSERVER_PRIVATE_EXPORT QMdmmSocketPrivateQTcpSocket : public QMdmmSocketPrivate
{
    Q_OBJECT

public:
    QMdmmSocketPrivateQTcpSocket(QTcpSocket *socket, QMdmmSocket *p);
    explicit QMdmmSocketPrivateQTcpSocket(QMdmmSocket *p);

    [[nodiscard]] QMdmmSocket::Type type() const override
    {
        return QMdmmSocket::TypeQTcpSocket;
    }

    bool connectToHost(const QString &addr) override;
    bool disconnectFromHost() override;

    QPointer<QTcpSocket> socket;
    void setupSocket();

public slots: // NOLINT(readability-redundant-access-specifiers)
    void sendPacket(QMdmmPacket packet) override;
    void readyRead();
    void errorOccurredTcpSocket(QAbstractSocket::SocketError e);
};

class QMDMMSERVER_PRIVATE_EXPORT QMdmmSocketPrivateQLocalSocket : public QMdmmSocketPrivate
{
    Q_OBJECT

public:
    QMdmmSocketPrivateQLocalSocket(QLocalSocket *socket, QMdmmSocket *p);
    explicit QMdmmSocketPrivateQLocalSocket(QMdmmSocket *p);

    [[nodiscard]] QMdmmSocket::Type type() const override
    {
        return QMdmmSocket::TypeQLocalSocket;
    }

    bool connectToHost(const QString &addr) override;
    bool disconnectFromHost() override;

    QPointer<QLocalSocket> socket;
    void setupSocket();

public slots: // NOLINT(readability-redundant-access-specifiers)
    void sendPacket(QMdmmPacket packet) override;
    void readyRead();
    void errorOccurredLocalSocket(QLocalSocket::LocalSocketError e);
};

class QMDMMSERVER_PRIVATE_EXPORT QMdmmSocketPrivateQWebSocket : public QMdmmSocketPrivate
{
    Q_OBJECT

public:
    QMdmmSocketPrivateQWebSocket(QWebSocket *socket, QMdmmSocket *p);
    explicit QMdmmSocketPrivateQWebSocket(QMdmmSocket *p);

    [[nodiscard]] QMdmmSocket::Type type() const override
    {
        return QMdmmSocket::TypeQWebSocket;
    }

    bool connectToHost(const QString &addr) override;
    bool disconnectFromHost() override;

    QPointer<QWebSocket> socket;
    void setupSocket();

public slots: // NOLINT(readability-redundant-access-specifiers)
    void sendPacket(QMdmmPacket packet) override;
    void errorOccurredWebSocket(QAbstractSocket::SocketError e);
};

namespace QMdmmSocketPrivateFactory {
QMDMMSERVER_PRIVATE_EXPORT QMdmmSocketPrivate *create(QLocalSocket *l, QMdmmSocket *p);
QMDMMSERVER_PRIVATE_EXPORT QMdmmSocketPrivate *create(QTcpSocket *t, QMdmmSocket *p);
QMDMMSERVER_PRIVATE_EXPORT QMdmmSocketPrivate *create(QWebSocket *w, QMdmmSocket *p);
QMDMMSERVER_PRIVATE_EXPORT QMdmmSocketPrivate *create(QMdmmSocket::Type type, QMdmmSocket *p);
} // namespace QMdmmSocketPrivateFactory

#endif
