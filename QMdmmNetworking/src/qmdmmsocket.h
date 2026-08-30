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

#ifndef DOXYGEN
namespace p {

class SocketP;
} // namespace p
#endif

#ifndef DOXYGEN
namespace v0 {
#endif

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

    Q_DISABLE_COPY_MOVE(Socket);

    explicit Socket(QTcpSocket *t, QObject *parent = nullptr);
    explicit Socket(QLocalSocket *l, QObject *parent = nullptr);
    explicit Socket(QWebSocket *w, QObject *parent = nullptr);

    explicit Socket(QObject *parent = nullptr);
    ~Socket() override;

    void setHasError(bool hasError);
    [[nodiscard]] bool hasError() const;

    bool connectToHost(const QString &host);
    void disconnectFromHost();

signals:
    void sendPacket(QMdmmCore::Packet);

    void packetReceived(QMdmmCore::Packet, QPrivateSignal);
    void socketErrorOccurred(const QString &errorString, QPrivateSignal);
    void socketDisconnected(QPrivateSignal);

#ifndef DOXYGEN
private:
    friend class p::SocketP;
    // non-const d-ptr.
    // SocketP is pv class and its internal implementation varies by socket type.
    // function connectToHost alters this d-ptr with proper implementation.
    // SocketP is QObject. QPointer can't be used since it is incomplete here
    p::SocketP *d;
#endif
};

#ifndef DOXYGEN
} // namespace v0

inline namespace v1 {
using v0::Socket;
}
#endif

} // namespace QMdmmNetworking

#endif
