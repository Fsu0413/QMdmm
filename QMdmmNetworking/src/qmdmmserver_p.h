// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSERVER_P
#define QMDMMSERVER_P

#include "qmdmmlogicrunner.h"
#include "qmdmmserver.h"
#include "qmdmmsocket.h"

#include <QMdmmPacket>

#include <QHash>
#include <QIODevice>
#include <QList>
#include <QLocalServer>
#include <QObject>
#include <QPointer>
#include <QTcpServer>
#include <QWebSocketServer>

// NOLINTBEGIN(misc-non-private-member-variables-in-classes): This is private header

namespace QMdmmNetworking {
namespace p {

class QMDMMNETWORKING_PRIVATE_EXPORT ServerP final : public QObject
{
    Q_OBJECT

    static QHash<QMdmmCore::Protocol::NotifyId, void (ServerP::*)(Socket *, const QJsonValue &)> notifyCallback;

public:
    ServerP(ServerConfiguration serverConfiguration, QMdmmCore::LogicConfiguration logicConfiguration, Server *q);

    // callbacks
    void pingServer(Socket *socket, const QJsonValue &packetValue);
    void signIn(Socket *socket, const QJsonValue &packetValue);
    void observe(Socket *socket, const QJsonValue &packetValue);

    void introduceSocket(Socket *socket);

public slots: // NOLINT(readability-redundant-access-specifiers)
    void tcpServerNewConnection();
    void localServerNewConnection();
    void websocketServerNewConnection();
    void socketPacketReceived(const QMdmmCore::Packet &packet);

    void logicRunnerGameOver();

public: // NOLINT(readability-redundant-access-specifiers)
    // variables
    ServerConfiguration serverConfiguration;
    QMdmmCore::LogicConfiguration logicConfiguration;
    Server *q;
    QTcpServer *t;
    QLocalServer *l;
    QWebSocketServer *w;
    LogicRunner *current;
};

} // namespace p
} // namespace QMdmmNetworking

// NOLINTEND(misc-non-private-member-variables-in-classes): This is private header

#endif
