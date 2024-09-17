// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSERVER_H
#define QMDMMSERVER_H

#include "qmdmmserverglobal.h"

#include <QMdmmCore/QMdmmProtocol>

#include <QObject>

#include <cstdint>

struct QMdmmServerConfiguration
{
    uint16_t port = 6366U;
};

// Note:
// QTcpSocket created by QTcpServer can't be child of QMdmmSocket since it is managed by QTcpServer
// QMdmmSocket should be a wrapper for QIODevice, and do serialize / deserialize work of received data
class QMdmmSocketPrivate;

class QMDMMSERVER_EXPORT QMdmmSocket : public QObject
{
    Q_OBJECT

public:
    enum Type
    {
        TypeQTcpSocket,
    };

    QMdmmSocket(QIODevice *socket, Type type, QObject *parent = nullptr);
    ~QMdmmSocket() override;

    void setHasError(bool hasError);
    [[nodiscard]] bool hasError() const;

signals:
    void sendPacket(QMdmmPacket);
    void packetReceived(QMdmmPacket, QPrivateSignal);

    void disconnected();

private:
    friend class QMdmmSocketPrivate;
    QMdmmSocketPrivate *const d;
};

class QMdmmServerPrivate;

class QMDMMSERVER_EXPORT QMdmmServer : public QObject
{
    Q_OBJECT

public:
    QMdmmServer(const QMdmmServerConfiguration &serverConfiguration, QObject *parent = nullptr);
    ~QMdmmServer() override;

private:
    QMdmmServerPrivate *const d;
};

#endif // QMDMMSERVER_H
