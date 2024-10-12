// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMCLIENT_P
#define QMDMMCLIENT_P

#include "qmdmmclient.h"

#include "qmdmmagent.h"
#include "qmdmmsocket.h"

#include <QMdmmProtocol>

#include <QLocalSocket>
#include <QPointer>
#include <QTcpSocket>
#include <QTimer>

class QMdmmRoom;

class QMDMMNETWORKING_PRIVATE_EXPORT QMdmmClientPrivate final : public QObject
{
    Q_OBJECT

public:
    static QHash<QMdmmProtocol::RequestId, void (QMdmmClientPrivate::*)(const QJsonValue &)> requestCallback;
    static QHash<QMdmmProtocol::NotifyId, void (QMdmmClientPrivate::*)(const QJsonValue &)> notifyCallback;

    QMdmmClientPrivate(QMdmmClient *q);

    QMdmmClient *q;
    QPointer<QMdmmSocket> socket;
    QMdmmRoom *room;
    QHash<QString, QMdmmAgent *> agents;

    QTimer *heartbeatTimer;

    QMdmmProtocol::RequestId currentRequest;

    void requestStoneScissorsCloth(const QJsonValue &value);
    void requestActionOrder(const QJsonValue &value);
    void requestAction(const QJsonValue &value);
    void requestUpgrade(const QJsonValue &value);

    void notifyPongServer(const QJsonValue &value);
    void notifyVersion(const QJsonValue &value);
    void notifyLogicConfiguration(const QJsonValue &value);
    void notifyAgentStateChanged(const QJsonValue &value);
    void notifyPlayerAdded(const QJsonValue &value);
    void notifyPlayerRemoved(const QJsonValue &value);
    void notifyGameStart(const QJsonValue &value);
    void notifyRoundStart(const QJsonValue &value);
    void notifyStoneScissorsCloth(const QJsonValue &value);
    void notifyActionOrder(const QJsonValue &value);
    void notifyAction(const QJsonValue &value);
    void notifyRoundOver(const QJsonValue &value);
    void notifyUpgrade(const QJsonValue &value);
    void notifyGameOver(const QJsonValue &value);
    void notifySpoken(const QJsonValue &value);
    void notifyOperated(const QJsonValue &value);

    bool applyAction(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPlace);
    bool applyUpgrade(const QHash<QString, QList<QMdmmData::UpgradeItem>> &upgrades);

public slots: // NOLINT(readability-redundant-access-specifiers)
    void socketPacketReceived(QMdmmPacket packet);
    void socketErrorOccurred(const QString &errorString);
    void socketDisconnected();

    void heartbeatTimeout();
};

#endif
