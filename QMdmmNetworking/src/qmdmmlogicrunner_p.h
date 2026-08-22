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
#include <utility>

// NOLINTBEGIN(misc-non-private-member-variables-in-classes): This is private header

namespace QMdmmNetworking {
namespace p {

// The server-side plumbing for one connected player: the socket, the request timer, the current
// request state, the protocol dispatch tables, and the round-event log. It is a *companion* to an
// Agent (composition, not inheritance): the Agent owns the player identity (name / screen name /
// state), while the ServerConnection owns everything tied to the wire. This split lets a
// socket-less local agent exist later without dragging socket machinery into the Agent type.
class QMDMMNETWORKING_PRIVATE_EXPORT ServerConnection : public QObject
{
    Q_OBJECT

    static QHash<QMdmmCore::Protocol::NotifyId, void (ServerConnection::*)(const QJsonValue &)> notifyCallback;
    static QHash<QMdmmCore::Protocol::RequestId, void (ServerConnection::*)(const QJsonValue &)> replyCallback;
    static QHash<QMdmmCore::Protocol::RequestId, void (ServerConnection::*)()> defaultReplyCallback;

    static int requestTimeoutGracePeriod;

public:
    ServerConnection(Agent *agent, const QMdmmCore::LogicConfiguration &logicConfiguration, QObject *parent = nullptr);
    ~ServerConnection() override;

    void setSocket(Socket *_socket);

    QPointer<Socket> socket;
    Agent *agent;
    QMdmmCore::LogicConfiguration conf;

    QMdmmCore::Protocol::RequestId currentRequest;
    QJsonValue currentRequestValue;
    QTimer *requestTimer;

    // Round-event log: every round-event packet this connection has broadcast (ssc / action-order
    // / action / upgrade), in send order. The list index IS the round-event sequence number (events
    // are appended in broadcast order, and the client counts one per event it receives), so no
    // explicit sequence is stored and no sorting is needed. Used for the per-agent precise
    // catch-up on reconnect (backlog "precise catch-up"): a reconnecting client is replayed only
    // the events it missed, in send order.
    QList<QMdmmCore::Packet> roundEventLog;

    void clearRoundEventLog();
    void replayMissedRoundEvents(int lastRoundEventSeq);

    void addRequest(QMdmmCore::Protocol::RequestId requestId, const QJsonValue &value);

    // reply decode callbacks: validate the wire value and hand the strong-typed reply to the Agent
    // (which then forwards it as the corresponding replyXxx signal). These keep only the JSON
    // validation / type conversion, which is the wire's job; the controller logic lives on Agent.
    void decodeStoneScissorsClothReply(const QJsonValue &value);
    void decodeActionOrderReply(const QJsonValue &value);
    void decodeActionReply(const QJsonValue &value);
    void decodeUpgradeReply(const QJsonValue &value);

    void defaultReplyStoneScissorsCloth();
    void defaultReplyActionOrder();
    void defaultReplyAction();
    void defaultReplyUpgrade();

    // player speech / operation: hand the wire value to the Agent (which then forwards it as the
    // spoken / operated signal). The wire only strips the value; the controller logic lives on Agent.
    void receiveSpeak(const QJsonValue &value);
    void receiveOperate(const QJsonValue &value);

signals:
    void sendPacket(QMdmmCore::Packet packet);
    void agentDisconnected(Agent *agent);

public slots: // NOLINT(readability-redundant-access-specifiers)
    void packetReceived(const QMdmmCore::Packet &packet);
    void onSocketDisconnected();

    // requests (encode + send): these slots listen to the Agent's xxxRequested signals and turn
    // each request into a wire packet. The controller methods themselves (requestXxx) now live on
    // the Agent.
    void sendStoneScissorsClothRequested(const QStringList &playerNames, int strivedOrder);
    void sendActionOrderRequested(const QList<int> &remainedOrders, int maximumOrder, int selectionNum);
    void sendActionRequested(int currentOrder);
    void sendUpgradeRequested(int remainingTimes);

    // notifications (encode + send): these slots listen to the Agent's xxxNotified signals and
    // turn each notification into a wire packet. The controller methods themselves (notifyXxx)
    // now live on the Agent.
    void sendLogicConfigurationNotified();
    void sendAgentStateChangeNotified(const QString &playerName, const QMdmmCore::Data::AgentState &agentState);
    void sendPlayerAddNotified(const QString &playerName, const QString &screenName, const QMdmmCore::Data::AgentState &agentState);
    void sendPlayerRemoveNotified(const QString &playerName);
    void sendGameStartNotified();
    void sendRoundStartNotified();
    void sendStoneScissorsClothNotified(const QHash<QString, QMdmmCore::Data::StoneScissorsCloth> &replies);
    void sendActionOrderNotified(const QHash<int, QString> &result);
    void sendActionNotified(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace);
    void sendRoundOverNotified();
    void sendUpgradeNotified(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades);
    void sendGameOverNotified(const QStringList &playerNames);
    void sendSpeakNotified(const QString &playerName, const QString &content);
    void sendOperateNotified(const QString &playerName, const QJsonValue &todo);

    void requestTimeout();
    void executeDefaultReply();
};

class QMDMMNETWORKING_PRIVATE_EXPORT LogicRunnerP : public QObject
{
    Q_OBJECT

public:
    LogicRunnerP(QMdmmCore::LogicConfiguration logicConfiguration, LogicRunner *q);
    ~LogicRunnerP() override;

    LogicRunner *q;

    QHash<QString, Agent *> agents;
    QHash<QString, ServerConnection *> connections;

    QThread *logicThread;
    QPointer<QMdmmCore::Logic> logic;

    QMdmmCore::LogicConfiguration conf;

    // No qRegisterMetaType<>() is needed for the queued signals / slots below: their argument
    // types are QMdmmCore::Data enums / flags (auto-registered via Q_ENUM_NS / Q_FLAG_NS) plus
    // Qt's built-in container metatypes, and the connections use the function-pointer syntax.
    // Verified by the in-process smoke test, which drives a full game across the logic thread.

public slots: // NOLINT(readability-redundant-access-specifiers)
    // slots called from agent
    void agentStateChanged(const QMdmmCore::Data::AgentState &state);
    void agentSpoken(const QString &content);
    void agentOperated(const QJsonValue &value);
    void agentStoneScissorsClothReplied(QMdmmCore::Data::StoneScissorsCloth ssc);
    void agentActionOrderReplied(const QList<int> &order);
    void agentActionReplied(QMdmmCore::Data::Action act, const QString &toPlayer, int toPlace);
    void agentUpgradeReplied(const QList<QMdmmCore::Data::UpgradeItem> &items);
    void agentDisconnected(Agent *agent);

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
    void roundOver();
    void gameOver(const QStringList &winners);

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

} // namespace p
} // namespace QMdmmNetworking

// NOLINTEND(misc-non-private-member-variables-in-classes): This is private header

#endif
