// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSOCKET_H
#define QMDMMSOCKET_H

#include "qmdmmnetworkingglobal.h"

#include <QMdmmPacket>

#include <QLocalSocket>
#include <QObject>
#include <QTcpSocket>
#include <QWebSocket>

// QMdmmSocket should be a wrapper for QObject, and do serialize / deserialize work of received data
class QMdmmSocketPrivate;

// For Server: This QMdmmSocket should be created when socket is in Open state!
class QMDMMNETWORKING_EXPORT QMdmmSocket : public QObject
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
    explicit QMdmmSocket(QTcpSocket *t, QObject *parent = nullptr);
    explicit QMdmmSocket(QLocalSocket *l, QObject *parent = nullptr);
    explicit QMdmmSocket(QWebSocket *w, QObject *parent = nullptr);

    // ctor for Client: pass type here
    explicit QMdmmSocket(QObject *parent = nullptr);
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
    // non-const d-ptr.
    // QMdmmSocketPrivate is pv class and its internal implementation varies by socket type.
    // function connectToHost alters this d-ptr with proper implementation.
    QMdmmSocketPrivate *d;
    Q_DISABLE_COPY_MOVE(QMdmmSocket);
};

#endif
