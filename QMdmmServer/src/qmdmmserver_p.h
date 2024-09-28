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

class QMDMMSERVER_EXPORT QMdmmServerPrivate final : public QObject
{
    Q_OBJECT

    static QHash<QMdmmProtocol::NotifyId, bool (QMdmmServerPrivate::*)(QMdmmSocket *, const QJsonValue &)> cb;

public:
    QMdmmServerPrivate(const QMdmmServerConfiguration &serverConfiguration, QMdmmServer *p);

    // callbacks
    bool pongClient(QMdmmSocket *socket, const QJsonValue &packetValue);
    bool pingServer(QMdmmSocket *socket, const QJsonValue &packetValue);
    bool signIn(QMdmmSocket *socket, const QJsonValue &packetValue);
    bool observe(QMdmmSocket *socket, const QJsonValue &packetValue);

    void introduceSocket(QMdmmSocket *socket);

public slots: // NOLINT(readability-redundant-access-specifiers)
    void tcpServerNewConnection();
    void localServerNewConnection();
    void websocketServerNewConnection();
    void socketPacketReceived(QMdmmPacket packet);

public: // NOLINT(readability-redundant-access-specifiers)
    // variables
    QMdmmServerConfiguration serverConfiguration;
    QMdmmServer *p;
    QTcpServer *t;
    QLocalServer *l;
    QWebSocketServer *w;
    QList<QMdmmLogicRunner *> runners;
    QMdmmLogicRunner *current;
};

#endif
