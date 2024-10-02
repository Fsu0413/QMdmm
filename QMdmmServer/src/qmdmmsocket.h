// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSOCKET_H
#define QMDMMSOCKET_H

#include "qmdmmserverglobal.h"

#include <QMdmmPacket>

#include <QLocalSocket>
#include <QObject>
#include <QTcpSocket>
#include <QWebSocket>

// QMdmmSocket should be a wrapper for QObject, and do serialize / deserialize work of received data
class QMdmmSocketPrivate;

// For Server: This QMdmmSocket should be created when socket is in Open state!
class QMDMMSERVER_EXPORT QMdmmSocket : public QObject
{
    Q_OBJECT

public:
    enum Type
    {
        TypeUnknown,
        TypeQTcpSocket,
        TypeQLocalSocket,
        TypeQWebSocket,
    };

    // ctor for Server: pass an already-open socket here
    QMdmmSocket(QTcpSocket *t, QObject *parent = nullptr);
    QMdmmSocket(QLocalSocket *l, QObject *parent = nullptr);
    QMdmmSocket(QWebSocket *w, QObject *parent = nullptr);

    // ctor for Client: pass type here
    QMdmmSocket(QObject *parent = nullptr);
    ~QMdmmSocket() override;

    void setHasError(bool hasError);
    [[nodiscard]] bool hasError() const;

    bool connectToHost(const QString &host);

signals:
    void sendPacket(QMdmmPacket);

    void packetReceived(QMdmmPacket, QPrivateSignal);
    void socketErrorOccurred(const QString &errorString, QPrivateSignal);
    void socketDisconnected(QPrivateSignal);

private:
    friend class QMdmmSocketPrivate;
    QMdmmSocketPrivate *d;
};

#endif
