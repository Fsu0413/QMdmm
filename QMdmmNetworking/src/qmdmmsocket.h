// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSOCKET_H
#define QMDMMSOCKET_H

#include "qmdmmnetworkingglobal.h"

#include <QMdmmPacket>

#include <QLocalSocket>
#include <QObject>
#include <QTcpSocket>
#include <QWebSocket>

#include <cstdint>

QMDMM_EXPORT_NAME(QMdmmSocket)

namespace QMdmmNetworking {
namespace p {

// QMdmmSocket should be a wrapper for QObject, and do serialize / deserialize work of received data
class SocketP;
} // namespace p

namespace v0 {

// For Server: This QMdmmSocket should be created when socket is in Open state!
class QMDMMNETWORKING_EXPORT Socket : public QObject
{
    Q_OBJECT

public:
    enum Type : uint8_t
    {
        TypeUnknown,
        TypeQTcpSocket,
        TypeQLocalSocket,
        TypeQWebSocket,
    };

    // ctor for Server: pass an already-open socket here
    explicit Socket(QTcpSocket *t, QObject *parent = nullptr);
    explicit Socket(QLocalSocket *l, QObject *parent = nullptr);
    explicit Socket(QWebSocket *w, QObject *parent = nullptr);

    // ctor for Client: pass type here
    explicit Socket(QObject *parent = nullptr);
    ~Socket() override;

    void setHasError(bool hasError);
    [[nodiscard]] bool hasError() const;

    bool connectToHost(const QString &host);

signals:
    void sendPacket(QMdmmCore::Packet);

    void packetReceived(QMdmmCore::Packet, QPrivateSignal);
    void socketErrorOccurred(const QString &errorString, QPrivateSignal);
    void socketDisconnected(QPrivateSignal);

private:
    friend class p::SocketP;
    // non-const d-ptr.
    // SocketP is pv class and its internal implementation varies by socket type.
    // function connectToHost alters this d-ptr with proper implementation.
    // SocketP is QObject. QPointer can't be used since it is incomplete here
    p::SocketP *d;
    Q_DISABLE_COPY_MOVE(Socket);
};
} // namespace v0

inline namespace v1 {
using v0::Socket;
}

} // namespace QMdmmNetworking

#endif
