// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMCLIENT_P
#define QMDMMCLIENT_P

#include "qmdmmclient.h"

#include "qmdmmagent.h"
#include "qmdmmsocket.h"

#include <QMdmmProtocol>
#include <QMdmmRoom>

#include <QLocalSocket>
#include <QPointer>
#include <QTcpSocket>
#include <QTimer>

// NOLINTBEGIN(misc-non-private-member-variables-in-classes): This is private header

namespace QMdmmProtocol = QMdmmCore::Protocol;

class QMDMMNETWORKING_PRIVATE_EXPORT QMdmmClientPrivate final : public QObject
{
    Q_OBJECT

public:
    static QHash<QMdmmCore::Protocol::RequestId, void (QMdmmClientPrivate::*)(const QJsonValue &)> requestCallback;
    static QHash<QMdmmCore::Protocol::NotifyId, void (QMdmmClientPrivate::*)(const QJsonValue &)> notifyCallback;

    QMdmmClientPrivate(QMdmmClientConfiguration clientConfiguration, QMdmmClient *q);

    QMdmmClient *q;
    QMdmmClientConfiguration clientConfiguration;
    QPointer<QMdmmSocket> socket;
    QMdmmCore::Room *room;
    QHash<QString, QMdmmAgent *> agents;

    QTimer *heartbeatTimer;

    QMdmmCore::Protocol::RequestId currentRequest;
    QMdmmCore::Data::AgentState initialState;

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

    bool applyAction(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace);
    bool applyUpgrade(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades);

public slots: // NOLINT(readability-redundant-access-specifiers)
    void socketPacketReceived(const QMdmmCore::Packet &packet);
    void socketErrorOccurred(const QString &errorString);
    void socketDisconnected();

    void heartbeatTimeout();
};

// NOLINTEND(misc-non-private-member-variables-in-classes): This is private header

#endif
