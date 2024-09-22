// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSOCKET_H
#define QMDMMSOCKET_H

#include "qmdmmserverglobal.h"

#include <QMdmmPacket>

#include <QIODevice>
#include <QObject>

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
        TypeQLocalSocket,
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

#endif
