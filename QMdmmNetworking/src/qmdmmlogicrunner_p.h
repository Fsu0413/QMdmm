// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMLOGICRUNNER_P
#define QMDMMLOGICRUNNER_P

#include "qmdmmlogicrunner.h"

#include "qmdmmagent.h"
#include "qmdmmsocket.h"

#include <QMdmmLogic>
#include <QMdmmRoom>

#include <QPointer>
#include <QThread>
#include <QTimer>

namespace QMdmmProtocol = QMdmmCore::Protocol;
using QMdmmLogic = QMdmmCore::Logic;

// NOLINTBEGIN(misc-non-private-member-variables-in-classes): This is private header

class QMDMMNETWORKING_PRIVATE_EXPORT QMdmmServerAgentPrivate : public QMdmmAgent
{
    Q_OBJECT

    static QHash<QMdmmCore::Protocol::NotifyId, void (QMdmmServerAgentPrivate::*)(const QJsonValue &)> notifyCallback;
    static QHash<QMdmmCore::Protocol::RequestId, void (QMdmmServerAgentPrivate::*)(const QJsonValue &)> replyCallback;
    static QHash<QMdmmCore::Protocol::RequestId, void (QMdmmServerAgentPrivate::*)()> defaultReplyCallback;

    static int requestTimeoutGracePeriod;

public:
    QMdmmServerAgentPrivate(const QString &name, QMdmmLogicRunnerPrivate *parent);
    ~QMdmmServerAgentPrivate() override;

    void setSocket(QMdmmSocket *_socket);

    QPointer<QMdmmSocket> socket;
    QMdmmLogicRunnerPrivate *p;

    QMdmmCore::Protocol::RequestId currentRequest;
    QJsonValue currentRequestValue;
    QTimer *requestTimer;

    void addRequest(QMdmmCore::Protocol::RequestId requestId, const QJsonValue &value);

    // callbacks
    void replyStoneScissorsCloth(const QJsonValue &value);
    void replyActionOrder(const QJsonValue &value);
    void replyAction(const QJsonValue &value);
    void replyUpgrade(const QJsonValue &value);

    void defaultReplyStoneScissorsCloth();
    void defaultReplyActionOrder();
    void defaultReplyAction();
    void defaultReplyUpgrade();

signals:
    void notifySpeak(const QJsonValue &value);
    void notifyOperate(const QJsonValue &value);

    void sendPacket(QMdmmCore::Packet packet);
    void socketDisconnected();

public slots: // NOLINT(readability-redundant-access-specifiers)
    void packetReceived(const QMdmmCore::Packet &packet);

    // requests
    void requestStoneScissorsCloth(const QStringList &playerNames, int strivedOrder);
    void requestActionOrder(const QList<int> &remainedOrders, int maximumOrder, int selectionNum);
    void requestAction(int currentOrder);
    void requestUpgrade(int remainingTimes);

    // notifications
    void notifyLogicConfiguration();
    void notifyAgentStateChanged(const QString &playerName, const QMdmmCore::Data::AgentState &agentState);
    void notifyPlayerAdded(const QString &playerName, const QString &screenName, const QMdmmCore::Data::AgentState &agentState);
    void notifyPlayerRemoved(const QString &playerName);
    void notifyGameStart();
    void notifyRoundStart();
    void notifyStoneScissorsCloth(const QHash<QString, QMdmmCore::Data::StoneScissorsCloth> &replies);
    void notifyActionOrder(const QHash<int, QString> &result);
    void notifyAction(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace);
    void notifyRoundOver();
    void notifyUpgrade(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades);
    void notifyGameOver(const QStringList &playerNames);
    void notifySpoken(const QString &playerName, const QString &content);
    void notifyOperated(const QString &playerName, const QJsonValue &todo);

    void requestTimeout();
    void executeDefaultReply();
};

class QMDMMNETWORKING_PRIVATE_EXPORT QMdmmLogicRunnerPrivate : public QObject
{
    Q_OBJECT

public:
    QMdmmLogicRunnerPrivate(QMdmmCore::LogicConfiguration logicConfiguration, QMdmmLogicRunner *q);
    ~QMdmmLogicRunnerPrivate() override;

    QMdmmLogicRunner *q;

    QHash<QString, QMdmmServerAgentPrivate *> agents;

    QThread *logicThread;
    QPointer<QMdmmLogic> logic;

    QMdmmCore::LogicConfiguration conf;

    // TODO: check if these queue connected signals / slots works with or without qRegisterMetaType<>()
    // If not, a new global function is needed in QMdmmCore for doing this work
    // I did a test of these combinations and found that all these types work without qRegisterMetaType<>() but it needs further testing

public slots: // NOLINT(readability-redundant-access-specifiers)
    // slots called from agent
    void agentStateChanged(const QMdmmCore::Data::AgentState &state);
    void agentSpoken(const QJsonValue &value);
    void agentOperated(const QJsonValue &value);
    void socketDisconnected();

    // These slots are called from Logic
    void requestSscForAction(const QStringList &playerNames);
    void sscResult(const QHash<QString, QMdmmCore::Data::StoneScissorsCloth> &replies);
    void requestActionOrder(const QString &playerName, const QList<int> &availableOrders, int maximumOrderNum, int selections);
    void actionOrderResult(const QHash<int, QString> &result);
    void requestSscForActionOrder(const QStringList &playerNames, int strivedOrder);
    void requestAction(const QString &playerName, int actionOrder);
    void actionResult(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace);
    void requestUpgrade(const QString &playerName, int upgradePoint);
    void upgradeResult(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades);

signals: // NOLINT(readability-redundant-access-specifiers)
    // These signals are emitted to Logic
    void addPlayer(const QString &playerName);
    void removePlayer(const QString &playerName);
    void roundStart();
    void sscReply(const QString &playerName, QMdmmCore::Data::StoneScissorsCloth ssc);
    void actionOrderReply(const QString &playerName, const QList<int> &desiredOrder);
    void actionReply(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace);
    void upgradeReply(const QString &playerName, const QList<QMdmmCore::Data::UpgradeItem> &items);
};

// NOLINTEND(misc-non-private-member-variables-in-classes): This is private header

#endif
