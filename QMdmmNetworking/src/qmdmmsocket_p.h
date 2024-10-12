// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSOCKET_P
#define QMDMMSOCKET_P

#include "qmdmmsocket.h"

#include <QObject>
#include <QPointer>

class QMDMMNETWORKING_PRIVATE_EXPORT QMdmmSocketPrivate : public QObject
{
    Q_OBJECT

public:
    static QMdmmSocket::Type typeByConnectAddr(const QString &addr);

    explicit QMdmmSocketPrivate(QMdmmSocket *q);
    [[nodiscard]] virtual QMdmmSocket::Type type() const = 0;

    virtual bool connectToHost(const QString &addr) = 0;
    virtual bool disconnectFromHost() = 0;

    QMdmmSocket *q;
    bool hasError;

public slots: // NOLINT(readability-redundant-access-specifiers)
    virtual void sendPacket(QMdmmPacket packet) = 0;
    bool packetReceived(const QByteArray &arr);
    void socketDisconnected();
    void errorOccurred(const QString &errorString);
};

class QMDMMNETWORKING_PRIVATE_EXPORT QMdmmSocketPrivateQTcpSocket : public QMdmmSocketPrivate
{
    Q_OBJECT

public:
    QMdmmSocketPrivateQTcpSocket(QTcpSocket *socket, QMdmmSocket *q);
    explicit QMdmmSocketPrivateQTcpSocket(QMdmmSocket *q);

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

class QMDMMNETWORKING_PRIVATE_EXPORT QMdmmSocketPrivateQLocalSocket : public QMdmmSocketPrivate
{
    Q_OBJECT

public:
    QMdmmSocketPrivateQLocalSocket(QLocalSocket *socket, QMdmmSocket *q);
    explicit QMdmmSocketPrivateQLocalSocket(QMdmmSocket *q);

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

class QMDMMNETWORKING_PRIVATE_EXPORT QMdmmSocketPrivateQWebSocket : public QMdmmSocketPrivate
{
    Q_OBJECT

public:
    QMdmmSocketPrivateQWebSocket(QWebSocket *socket, QMdmmSocket *q);
    explicit QMdmmSocketPrivateQWebSocket(QMdmmSocket *q);

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
QMDMMNETWORKING_PRIVATE_EXPORT QMdmmSocketPrivate *create(QLocalSocket *l, QMdmmSocket *p);
QMDMMNETWORKING_PRIVATE_EXPORT QMdmmSocketPrivate *create(QTcpSocket *t, QMdmmSocket *p);
QMDMMNETWORKING_PRIVATE_EXPORT QMdmmSocketPrivate *create(QWebSocket *w, QMdmmSocket *p);
QMDMMNETWORKING_PRIVATE_EXPORT QMdmmSocketPrivate *create(QMdmmSocket::Type type, QMdmmSocket *p);
} // namespace QMdmmSocketPrivateFactory

#endif
