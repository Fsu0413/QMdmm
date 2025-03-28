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

namespace QMdmmProtocol = QMdmmCore::Protocol;

class QMDMMNETWORKING_PRIVATE_EXPORT QMdmmServerPrivate final : public QObject
{
    Q_OBJECT

    static QHash<QMdmmCore::Protocol::NotifyId, void (QMdmmServerPrivate::*)(QMdmmSocket *, const QJsonValue &)> notifyCallback;

public:
    QMdmmServerPrivate(QMdmmServerConfiguration serverConfiguration, QMdmmCore::LogicConfiguration logicConfiguration, QMdmmServer *q);

    // callbacks
    void pingServer(QMdmmSocket *socket, const QJsonValue &packetValue);
    void signIn(QMdmmSocket *socket, const QJsonValue &packetValue);
    void observe(QMdmmSocket *socket, const QJsonValue &packetValue);

    void introduceSocket(QMdmmSocket *socket);

public slots: // NOLINT(readability-redundant-access-specifiers)
    void tcpServerNewConnection();
    void localServerNewConnection();
    void websocketServerNewConnection();
    void socketPacketReceived(const QMdmmCore::Packet &packet);

    void logicRunnerGameOver();

public: // NOLINT(readability-redundant-access-specifiers)
    // variables
    QMdmmServerConfiguration serverConfiguration;
    QMdmmCore::LogicConfiguration logicConfiguration;
    QMdmmServer *q;
    QTcpServer *t;
    QLocalServer *l;
    QWebSocketServer *w;
    QMdmmLogicRunner *current;
};

// NOLINTEND(misc-non-private-member-variables-in-classes): This is private header

#endif
