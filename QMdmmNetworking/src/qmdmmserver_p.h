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

class QMDMMNETWORKING_PRIVATE_EXPORT QMdmmServerPrivate final : public QObject
{
    Q_OBJECT

    static QHash<QMdmmProtocol::NotifyId, void (QMdmmServerPrivate::*)(QMdmmSocket *, const QJsonValue &)> notifyCallback;

public:
    QMdmmServerPrivate(const QMdmmServerConfiguration &serverConfiguration, QMdmmServer *p);

    // callbacks
    void pingServer(QMdmmSocket *socket, const QJsonValue &packetValue);
    void signIn(QMdmmSocket *socket, const QJsonValue &packetValue);
    void observe(QMdmmSocket *socket, const QJsonValue &packetValue);

    void introduceSocket(QMdmmSocket *socket);

public slots: // NOLINT(readability-redundant-access-specifiers)
    void tcpServerNewConnection();
    void localServerNewConnection();
    void websocketServerNewConnection();
    void socketPacketReceived(QMdmmPacket packet);

    void logicRunnerGameOver();

public: // NOLINT(readability-redundant-access-specifiers)
    // variables
    QMdmmServerConfiguration serverConfiguration;
    QMdmmServer *p;
    QTcpServer *t;
    QLocalServer *l;
    QWebSocketServer *w;
    QMdmmLogicRunner *current;
};

#endif
