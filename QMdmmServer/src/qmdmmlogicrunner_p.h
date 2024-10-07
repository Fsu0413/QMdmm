// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMLOGICRUNNER_P
#define QMDMMLOGICRUNNER_P

#include "qmdmmlogicrunner.h"

#include "qmdmmagent.h"
#include "qmdmmsocket.h"

#include <QMdmmLogic>

#include <QPointer>
#include <QThread>
#include <QTimer>

class QMDMMSERVER_PRIVATE_EXPORT QMdmmServerAgentPrivate : public QMdmmAgent
{
    Q_OBJECT

    static QHash<QMdmmProtocol::NotifyId, void (QMdmmServerAgentPrivate::*)(const QJsonValue &)> notifyCallback;
    static QHash<QMdmmProtocol::RequestId, void (QMdmmServerAgentPrivate::*)(const QJsonValue &)> replyCallback;
    static QHash<QMdmmProtocol::RequestId, void (QMdmmServerAgentPrivate::*)()> defaultReplyCallback;

    static int requestTimeoutGracePeriod;

public:
    QMdmmServerAgentPrivate(const QString &name, QMdmmLogicRunnerPrivate *parent);
    ~QMdmmServerAgentPrivate() override;

    void setSocket(QMdmmSocket *_socket);

    QPointer<QMdmmSocket> socket;
    QMdmmLogicRunnerPrivate *p;

    QMdmmProtocol::RequestId currentRequest;
    QJsonValue currentRequestValue;
    QTimer *requestTimer;

    void addRequest(QMdmmProtocol::RequestId requestId, const QJsonValue &value);

    // callbacks
    void replyStoneScissorsCloth(const QJsonValue &value);
    void replyActionOrder(const QJsonValue &value);
    void replyAction(const QJsonValue &value);
    void replyUpdate(const QJsonValue &value);

    void defaultReplyStoneScissorsCloth();
    void defaultReplyActionOrder();
    void defaultReplyAction();
    void defaultReplyUpdate();

signals:
    void notifySpeak(const QJsonValue &value);
    void notifyOperate(const QJsonValue &value);

    void sendPacket(QMdmmPacket packet);
    void socketDisconnected();

public slots: // NOLINT(readability-redundant-access-specifiers)
    void packetReceived(QMdmmPacket packet);

    // requests
    void requestStoneScissorsCloth(const QStringList &playerNames, int strivedOrder);
    void requestActionOrder(const QList<int> &remainedOrders, int maximumOrder, int selectionNum);
    void requestAction(int currentOrder);
    void requestUpdate(int remainingTimes);

    // notifications
    void notifyLogicConfiguration();
    void notifyAgentStateChanged(const QString &playerName, const QMdmmData::AgentState &agentState);
    void notifyPlayerAdded(const QString &playerName, const QString &screenName, const QMdmmData::AgentState &agentState);
    void notifyPlayerRemoved(const QString &playerName);
    void notifyGameStart();
    void notifyRoundStart();
    void notifyStoneScissorsCloth(const QHash<QString, QMdmmData::StoneScissorsCloth> &replies);
    void notifyActionOrder(const QHash<int, QString> &result);
    void notifyAction(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPlace);
    void notifyRoundOver();
    void notifyUpgrade(const QHash<QString, QList<QMdmmData::UpgradeItem>> &upgrades);
    void notifyGameOver(const QStringList &playerNames);
    void notifySpoken(const QString &playerName, const QString &content);
    void notifyOperated(const QString &playerName, const QJsonValue &todo);

    void requestTimeout();
    void executeDefaultReply();
};

class QMDMMSERVER_PRIVATE_EXPORT QMdmmLogicRunnerPrivate : public QObject
{
    Q_OBJECT

public:
    QMdmmLogicRunnerPrivate(const QMdmmLogicConfiguration &logicConfiguration, QMdmmLogicRunner *parent);
    ~QMdmmLogicRunnerPrivate() override;

    QMdmmLogicRunner *p;

    QHash<QString, QMdmmServerAgentPrivate *> agents;

    QThread *logicThread;
    QPointer<QMdmmLogic> logic;

    QMdmmLogicConfiguration conf;

    // TODO: check if these queue connected signals / slots works with or without qRegisterMetaType<>()
    // If not, a new global function is needed in QMdmmCore for doing this work
    // I did a test of these combinations and found that all these types work without qRegisterMetaType<>() but it needs further testing

public slots: // NOLINT(readability-redundant-access-specifiers)
    // slots called from agent
    void agentStateChanged(const QMdmmData::AgentState &state);
    void agentSpoken(const QJsonValue &value);
    void agentOperated(const QJsonValue &value);
    void socketDisconnected();

    // These slots are called from Logic
    void requestSscForAction(const QStringList &playerNames);
    void sscResult(const QHash<QString, QMdmmData::StoneScissorsCloth> &replies);
    void requestActionOrder(const QString &playerName, const QList<int> &availableOrders, int maximumOrderNum, int selections);
    void actionOrderResult(const QHash<int, QString> &result);
    void requestSscForActionOrder(const QStringList &playerNames, int strivedOrder);
    void requestAction(const QString &playerName, int actionOrder);
    void actionResult(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPlace);
    void requestUpgrade(const QString &playerName, int upgradePoint);
    void upgradeResult(const QHash<QString, QList<QMdmmData::UpgradeItem>> &upgrades);

signals: // NOLINT(readability-redundant-access-specifiers)
    // These signals are emitted to Logic
    void addPlayer(const QString &playerName);
    void removePlayer(const QString &playerName);
    void roundStart();
    void sscReply(const QString &playerName, QMdmmData::StoneScissorsCloth ssc);
    void actionOrderReply(const QString &playerName, const QList<int> &desiredOrder);
    void actionReply(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPlace);
    void upgradeReply(const QString &playerName, const QList<QMdmmData::UpgradeItem> &items);
};

#endif
