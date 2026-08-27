// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmlogicrunner.h"
#include "qmdmmlogicrunner_p.h"

#include <QJsonArray>
#include <QMetaType>
#include <QRandomGenerator>
#include <utility>

/**
 * @file qmdmmlogicrunner.h
 * @brief This is the file where the networking LogicRunner is defined.
 */

namespace QMdmmNetworking {
#ifndef DOXYGEN
namespace p {

QHash<QMdmmCore::Protocol::NotifyId, void (ServerConnection::*)(const QJsonValue &)> ServerConnection::notifyCallback {
    std::make_pair(QMdmmCore::Protocol::NotifySpeak, &ServerConnection::receiveSpeak),
    std::make_pair(QMdmmCore::Protocol::NotifyOperate, &ServerConnection::receiveOperate),
};

QHash<QMdmmCore::Protocol::RequestId, void (ServerConnection::*)(const QJsonValue &)> ServerConnection::replyCallback {
    std::make_pair(QMdmmCore::Protocol::RequestStoneScissorsCloth, &ServerConnection::decodeStoneScissorsClothReply),
    std::make_pair(QMdmmCore::Protocol::RequestActionOrder, &ServerConnection::decodeActionOrderReply),
    std::make_pair(QMdmmCore::Protocol::RequestAction, &ServerConnection::decodeActionReply),
    std::make_pair(QMdmmCore::Protocol::RequestUpgrade, &ServerConnection::decodeUpgradeReply),
};

QHash<QMdmmCore::Protocol::RequestId, void (ServerConnection::*)()> ServerConnection::defaultReplyCallback {
    std::make_pair(QMdmmCore::Protocol::RequestStoneScissorsCloth, &ServerConnection::defaultReplyStoneScissorsCloth),
    std::make_pair(QMdmmCore::Protocol::RequestActionOrder, &ServerConnection::defaultReplyActionOrder),
    std::make_pair(QMdmmCore::Protocol::RequestAction, &ServerConnection::defaultReplyAction),
    std::make_pair(QMdmmCore::Protocol::RequestUpgrade, &ServerConnection::defaultReplyUpgrade),
};

// Extra tolerance in milliseconds added on top of ServerConfiguration::requestTimeout
// (which is in seconds) for the request timer. The timer only backstops abnormal cases
// (D-020): a healthy client replies or gives up on its own, so the grace period just
// absorbs ordinary network jitter.
int ServerConnection::requestTimeoutGracePeriod = 60;

ServerConnection::ServerConnection(Agent *agent, const QMdmmCore::LogicConfiguration &logicConfiguration, int requestTimeout, QObject *parent)
    : QObject(parent)
    , agent(agent)
    , conf(logicConfiguration)
    , currentRequest(QMdmmCore::Protocol::RequestInvalid)
    , requestTimer(new QTimer(this))
{
    requestTimer->setInterval(requestTimeout * 1000 + requestTimeoutGracePeriod);
    requestTimer->setSingleShot(true);
    connect(requestTimer, &QTimer::timeout, this, &ServerConnection::requestTimeout);

    // Wire the Agent's notification signals to this connection's encode-and-send slots. The
    // Agent forwards a notifyXxx() call as the corresponding xxxNotified signal; this connection
    // turns the strong-typed notification back into a wire packet and sends it.
    connect(agent, &Agent::logicConfigurationNotified, this, &ServerConnection::sendLogicConfigurationNotified);
    connect(agent, &Agent::agentStateChangeNotified, this, &ServerConnection::sendAgentStateChangeNotified);
    connect(agent, &Agent::playerAddNotified, this, &ServerConnection::sendPlayerAddNotified);
    connect(agent, &Agent::playerRemoveNotified, this, &ServerConnection::sendPlayerRemoveNotified);
    connect(agent, &Agent::gameStartNotified, this, &ServerConnection::sendGameStartNotified);
    connect(agent, &Agent::roundStartNotified, this, &ServerConnection::sendRoundStartNotified);
    connect(agent, &Agent::stoneScissorsClothNotified, this, &ServerConnection::sendStoneScissorsClothNotified);
    connect(agent, &Agent::actionOrderNotified, this, &ServerConnection::sendActionOrderNotified);
    connect(agent, &Agent::actionNotified, this, &ServerConnection::sendActionNotified);
    connect(agent, &Agent::roundOverNotified, this, &ServerConnection::sendRoundOverNotified);
    connect(agent, &Agent::upgradeNotified, this, &ServerConnection::sendUpgradeNotified);
    connect(agent, &Agent::gameOverNotified, this, &ServerConnection::sendGameOverNotified);
    connect(agent, &Agent::speakNotified, this, &ServerConnection::sendSpeakNotified);
    connect(agent, &Agent::operateNotified, this, &ServerConnection::sendOperateNotified);

    // Wire the Agent's request signals to this connection's encode-and-send slots. The Agent
    // forwards a requestXxx() call as the corresponding xxxRequested signal; this connection turns
    // the strong-typed request back into a wire packet and sends it.
    connect(agent, &Agent::stoneScissorsClothRequested, this, &ServerConnection::sendStoneScissorsClothRequested);
    connect(agent, &Agent::actionOrderRequested, this, &ServerConnection::sendActionOrderRequested);
    connect(agent, &Agent::actionRequested, this, &ServerConnection::sendActionRequested);
    connect(agent, &Agent::upgradeRequested, this, &ServerConnection::sendUpgradeRequested);
}

ServerConnection::~ServerConnection() = default;

void ServerConnection::setSocket(Socket *_socket)
{
    if (socket != nullptr)
        socket->deleteLater();

    socket = _socket;
    if (socket != nullptr) {
        connect(socket, &Socket::packetReceived, this, &ServerConnection::packetReceived);
        // The socket drop is handled by onSocketDisconnected, which translates it into an
        // Agent event (see there). The room only ever deals with agents, never sockets.
        connect(socket, &Socket::socketDisconnected, this, &ServerConnection::onSocketDisconnected);
        connect(this, &ServerConnection::sendPacket, socket, &Socket::sendPacket);
    }
}

void ServerConnection::onSocketDisconnected()
{
    // Translate the socket drop into an Agent event (D-018: the socket is digested at the
    // server/wire layer, and the room only ever sees agents). Clean up the socket, mark the
    // agent offline, auto-reply any in-flight request so the logic keeps advancing, then emit
    // the disconnect as an agent event -- the room then decides whether to drop the agent
    // (not full) or preserve its seat for a reconnect (full).
    if (socket != nullptr) {
        socket->deleteLater();
        socket = nullptr;
    }

    QMdmmCore::Data::AgentState state = agent->state();
    state.setFlag(QMdmmCore::Data::StateMaskOnline, false).setFlag(QMdmmCore::Data::StateMaskTrust, false);
    agent->setState(state);

    // If there is an active request, answer it with the default reply so the logic is not left
    // waiting on the gone player. executeDefaultReply handles the no-active-request case as a
    // no-op (the not-full case, where the game has not started yet).
    executeDefaultReply();

    emit agentDisconnected(agent);
}

void ServerConnection::addRequest(QMdmmCore::Protocol::RequestId requestId, const QJsonValue &value)
{
    currentRequest = requestId;
    currentRequestValue = value;

    if (socket != nullptr) {
        emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::TypeRequest, requestId, value));
        requestTimer->start();
    } else {
        // No socket: the request cannot go over the wire, so the connection answers it itself
        // with the default reply. This reply must be *asynchronous*, never synchronous -- the
        // request/reply flow is event-driven by design:
        // 1. addRequest is reached from a sendXxxRequested slot triggered by the Agent's
        //    xxxRequested signal (i.e. from inside LogicRunnerP's request handler). Doing the
        //    default reply synchronously there re-enters the request handler and stalls the
        //    event loop with the reply's work.
        // 2. The reply is designed to be delivered asynchronously; a synchronous reply would
        //    reach the logic side before the request handler returns, breaking the ordering.
        QTimer::singleShot(0, Qt::CoarseTimer, this, &ServerConnection::executeDefaultReply);
    }
}

void ServerConnection::decodeStoneScissorsClothReply(const QJsonValue &value)
{
#define DEFAULTREPLY                      \
    do {                                  \
        defaultReplyStoneScissorsCloth(); \
        return;                           \
    } while (0)

    if (!value.isDouble())
        DEFAULTREPLY;

    QMdmmCore::Data::StoneScissorsCloth ssc = static_cast<QMdmmCore::Data::StoneScissorsCloth>(value.toInt());
    switch (ssc) {
    case QMdmmCore::Data::Stone:
    case QMdmmCore::Data::Scissors:
    case QMdmmCore::Data::Cloth:
        break;
    default:
        DEFAULTREPLY;
    }

    agent->stoneScissorsCloth(ssc);

#undef DEFAULTREPLY
}

void ServerConnection::decodeActionOrderReply(const QJsonValue &value)
{
#define DEFAULTREPLY               \
    do {                           \
        defaultReplyActionOrder(); \
        return;                    \
    } while (0)

    if (!value.isArray())
        DEFAULTREPLY;

    QJsonArray arr = value.toArray();
    QList<int> ao;
    for (QJsonArray::const_iterator it = arr.constBegin(); it != arr.constEnd(); ++it) {
        if (!it->isDouble())
            DEFAULTREPLY;
        ao << it->toInt();
    }

    agent->actionOrder(ao);

#undef DEFAULTREPLY
}

void ServerConnection::decodeActionReply(const QJsonValue &value)
{
#define DEFAULTREPLY          \
    do {                      \
        defaultReplyAction(); \
        return;               \
    } while (0)

    if (!value.isObject())
        DEFAULTREPLY;

    QJsonObject arr = value.toObject();

    if (!arr.contains(QStringLiteral("action")))
        DEFAULTREPLY;
    QJsonValue vaction = arr.value(QStringLiteral("action"));
    if (!vaction.isDouble())
        DEFAULTREPLY;
    QMdmmCore::Data::Action act = static_cast<QMdmmCore::Data::Action>(vaction.toInt());
    switch (act) {
    case QMdmmCore::Data::DoNothing:
    case QMdmmCore::Data::BuyKnife:
    case QMdmmCore::Data::BuyHorse:
    case QMdmmCore::Data::Slash:
    case QMdmmCore::Data::Kick:
    case QMdmmCore::Data::Move:
    case QMdmmCore::Data::LetMove:
        break;
    default:
        DEFAULTREPLY;
    }

    QString toPlayer;
    switch (act) {
    case QMdmmCore::Data::Slash:
    case QMdmmCore::Data::Kick:
    case QMdmmCore::Data::LetMove: {
        if (!arr.contains(QStringLiteral("toPlayer")))
            DEFAULTREPLY;
        QJsonValue vtoPlayer = arr.value(QStringLiteral("toPlayer"));
        if (!vtoPlayer.isString())
            DEFAULTREPLY;
        toPlayer = vtoPlayer.toString();
        break;
    }
    default:
        break;
    }

    int toPlace = 0;
    switch (act) {
    case QMdmmCore::Data::Move:
    case QMdmmCore::Data::LetMove: {
        if (!arr.contains(QStringLiteral("toPlace")))
            DEFAULTREPLY;
        QJsonValue vtoPlace = arr.value(QStringLiteral("toPlace"));
        if (!vtoPlace.isDouble())
            DEFAULTREPLY;
        toPlace = vtoPlace.toInt();
        break;
    }
    default:
        break;
    }

    agent->action(act, toPlayer, toPlace);

#undef DEFAULTREPLY
}

void ServerConnection::decodeUpgradeReply(const QJsonValue &value)
{
#define DEFAULTREPLY           \
    do {                       \
        defaultReplyUpgrade(); \
        return;                \
    } while (0)

    if (!value.isArray())
        DEFAULTREPLY;

    QJsonArray arr = value.toArray();
    QList<QMdmmCore::Data::UpgradeItem> ups;
    for (QJsonArray::const_iterator it = arr.constBegin(); it != arr.constEnd(); ++it) {
        if (!it->isDouble())
            DEFAULTREPLY;
        QMdmmCore::Data::UpgradeItem up = static_cast<QMdmmCore::Data::UpgradeItem>(it->toInt());
        switch (up) {
        case QMdmmCore::Data::UpgradeKnife:
        case QMdmmCore::Data::UpgradeHorse:
        case QMdmmCore::Data::UpgradeMaxHp:
            break;
        default:
            DEFAULTREPLY;
        }
        ups << up;
    }

    agent->upgrade(ups);

#undef DEFAULTREPLY
}

void ServerConnection::defaultReplyStoneScissorsCloth()
{
    agent->stoneScissorsCloth(static_cast<QMdmmCore::Data::StoneScissorsCloth>(QRandomGenerator::global()->generate() % 3));
}

void ServerConnection::defaultReplyActionOrder()
{
    QJsonObject ob = currentRequestValue.toObject();
    QJsonArray arr = ob.value(QStringLiteral("remainedOrders")).toArray();
    int num = ob.value(QStringLiteral("selectionNum")).toInt();
    QList<int> ao;
    ao.reserve(num);
    while ((num--) != 0)
        ao.append(arr.takeAt(0).toInt());

    agent->actionOrder(ao);
}

void ServerConnection::defaultReplyAction()
{
    agent->action(QMdmmCore::Data::DoNothing, {}, 0);
}

void ServerConnection::defaultReplyUpgrade()
{
    int times = currentRequestValue.toInt(1);
    QList<QMdmmCore::Data::UpgradeItem> ups;
    ups.reserve(times);
    while ((times--) != 0)
        ups << QMdmmCore::Data::UpgradeMaxHp;
    agent->upgrade(ups);
}

void ServerConnection::packetReceived(const QMdmmCore::Packet &packet)
{
    if (socket == nullptr)
        return;

    if (packet.type() == QMdmmCore::Protocol::TypeNotify) {
        if ((packet.notifyId() & QMdmmCore::Protocol::NotifyToAgentMask) != 0) {
            void (ServerConnection::*call)(const QJsonValue &) = notifyCallback.value(packet.notifyId(), nullptr);
            if (call != nullptr)
                (this->*call)(packet.value());
            else
                socket->setHasError(true);
        }
    } else if (packet.type() == QMdmmCore::Protocol::TypeReply) {
        if (currentRequest == packet.requestId()) {
            requestTimer->stop();
            // A null reply value is the protocol's "give up" marker (every legal reply value is
            // non-null), so it means the client has no answer and the default reply applies. Note
            // executeDefaultReply reads currentRequest before clearing it, so it must be called
            // *before* resetting currentRequest below.
            if (packet.value().isNull()) {
                executeDefaultReply();
            } else {
                currentRequest = QMdmmCore::Protocol::RequestInvalid;
                void (ServerConnection::*call)(const QJsonValue &) = replyCallback.value(packet.requestId(), nullptr);
                if (call != nullptr)
                    (this->*call)(packet.value());
                else
                    socket->setHasError(true);
            }
        }
    }
}

void ServerConnection::sendStoneScissorsClothRequested(const QStringList &playerNames, int strivedOrder)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerNames"), QJsonArray::fromStringList(playerNames));
    ob.insert(QStringLiteral("strivedOrder"), strivedOrder);
    addRequest(QMdmmCore::Protocol::RequestStoneScissorsCloth, ob);
}

void ServerConnection::sendActionOrderRequested(const QList<int> &remainedOrders, int maximumOrder, int selectionNum)
{
    QJsonObject ob;
    QJsonArray arr;
    foreach (int remainedOrder, remainedOrders)
        arr.append(remainedOrder);
    ob.insert(QStringLiteral("remainedOrders"), arr);
    ob.insert(QStringLiteral("maximumOrder"), maximumOrder);
    ob.insert(QStringLiteral("selectionNum"), selectionNum);
    addRequest(QMdmmCore::Protocol::RequestActionOrder, ob);
}

void ServerConnection::sendActionRequested(int currentOrder)
{
    addRequest(QMdmmCore::Protocol::RequestAction, currentOrder);
}

void ServerConnection::sendUpgradeRequested(int remainingTimes)
{
    addRequest(QMdmmCore::Protocol::RequestUpgrade, remainingTimes);
}

void ServerConnection::sendLogicConfigurationNotified()
{
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyLogicConfiguration, conf));
}

void ServerConnection::sendAgentStateChangeNotified(const QString &playerName, const QMdmmCore::Data::AgentState &agentState)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerName"), playerName);
    ob.insert(QStringLiteral("agentState"), static_cast<int>(QMdmmCore::Data::AgentState::Int(agentState)));
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyAgentStateChanged, ob));
}

void ServerConnection::sendPlayerAddNotified(const QString &playerName, const QString &screenName, const QMdmmCore::Data::AgentState &agentState)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerName"), playerName);
    ob.insert(QStringLiteral("screenName"), screenName);
    ob.insert(QStringLiteral("agentState"), static_cast<int>(QMdmmCore::Data::AgentState::Int(agentState)));
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyPlayerAdded, ob));
}

void ServerConnection::sendPlayerRemoveNotified(const QString &playerName)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerName"), playerName);
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyPlayerRemoved, ob));
}

void ServerConnection::sendGameStartNotified()
{
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyGameStart, {}));
}

void ServerConnection::sendRoundStartNotified()
{
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyRoundStart, {}));
}

void ServerConnection::sendStoneScissorsClothNotified(const QHash<QString, QMdmmCore::Data::StoneScissorsCloth> &replies)
{
    QJsonObject ob;
    for (QHash<QString, QMdmmCore::Data::StoneScissorsCloth>::const_iterator it = replies.constBegin(); it != replies.constEnd(); ++it)
        ob.insert(it.key(), static_cast<int>(it.value()));
    QMdmmCore::Packet packet(QMdmmCore::Protocol::NotifyStoneScissorsCloth, ob);
    roundEventLog.append(packet);
    emit sendPacket(packet);
}

void ServerConnection::sendActionOrderNotified(const QHash<int, QString> &result)
{
    QJsonArray arr;
    for (int i = 1; i <= result.count(); ++i)
        arr.append(result.value(i));
    QMdmmCore::Packet packet(QMdmmCore::Protocol::NotifyActionOrder, arr);
    roundEventLog.append(packet);
    emit sendPacket(packet);
}

void ServerConnection::sendActionNotified(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerName"), playerName);
    ob.insert(QStringLiteral("action"), static_cast<int>(action));

    switch (action) {
    case QMdmmCore::Data::Slash:
    case QMdmmCore::Data::Kick:
    case QMdmmCore::Data::LetMove:
        ob.insert(QStringLiteral("toPlayer"), toPlayer);
        break;
    default:
        break;
    }

    switch (action) {
    case QMdmmCore::Data::Move:
    case QMdmmCore::Data::LetMove:
        ob.insert(QStringLiteral("toPlace"), toPlace);
        break;
    default:
        break;
    }

    QMdmmCore::Packet packet(QMdmmCore::Protocol::NotifyAction, ob);
    roundEventLog.append(packet);
    emit sendPacket(packet);
}

void ServerConnection::sendRoundOverNotified()
{
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyRoundOver, {}));
}

void ServerConnection::sendUpgradeNotified(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades)
{
    QJsonObject ob;
    for (QHash<QString, QList<QMdmmCore::Data::UpgradeItem>>::const_iterator it = upgrades.constBegin(); it != upgrades.constEnd(); ++it) {
        QJsonArray arr;
        foreach (QMdmmCore::Data::UpgradeItem up, it.value())
            arr.append(static_cast<int>(up));
        ob.insert(it.key(), arr);
    }
    QMdmmCore::Packet packet(QMdmmCore::Protocol::NotifyUpgrade, ob);
    roundEventLog.append(packet);
    emit sendPacket(packet);
}

void ServerConnection::clearRoundEventLog()
{
    roundEventLog.clear();
}

void ServerConnection::replayMissedRoundEvents(int lastRoundEventSeq)
{
    // roundEventLog is ordered by send order, and its index IS the round-event sequence number
    // (see roundEventLog in qmdmmlogicrunner_p.h), which equals the client's received-event
    // counter. Replaying in-place needs no sorting: skip the first `lastRoundEventSeq` events the
    // client already got and re-send the rest, in order.
    for (int i = lastRoundEventSeq; i < roundEventLog.size(); ++i)
        emit sendPacket(roundEventLog.at(i));
}

void ServerConnection::reconnect(Socket *socket, int lastRoundEventSeq)
{
    // Rebind the socket. setSocket is safe because onSocketDisconnected already set the old
    // socket to nullptr before this point, so nothing gets double-deleted.
    setSocket(socket);

    // Precise catch-up: replay the round events the client missed. The client reported how many
    // round events it received before the drop (lastRoundEventSeq); every event this connection
    // broadcast while the client was gone is replayed in send order, skipping the first
    // `lastRoundEventSeq` (already received). See replayMissedRoundEvents.
    replayMissedRoundEvents(lastRoundEventSeq);
}

void ServerConnection::sendGameOverNotified(const QStringList &playerNames)
{
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyGameOver, QJsonArray::fromStringList(playerNames)));
}

void ServerConnection::sendSpeakNotified(const QString &playerName, const QString &content)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerName"), playerName);
    ob.insert(QStringLiteral("content"), content);
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifySpoken, ob));
}

void ServerConnection::sendOperateNotified(const QString &playerName, const QJsonValue &todo)
{
    Q_UNIMPLEMENTED();
    Q_UNUSED(playerName);
    Q_UNUSED(todo);
}

void ServerConnection::requestTimeout()
{
    if (socket != nullptr)
        socket->setHasError(true);
    executeDefaultReply();
}

void ServerConnection::executeDefaultReply()
{
    if (currentRequest != QMdmmCore::Protocol::RequestInvalid) {
        void (ServerConnection::*call)() = defaultReplyCallback.value(currentRequest, nullptr);
        currentRequest = QMdmmCore::Protocol::RequestInvalid;
        if (call != nullptr)
            (this->*call)();
    }
}

void ServerConnection::receiveSpeak(const QJsonValue &value)
{
    // The value is the Base64-encoded content sent by Client::notifySpeak. The server forwards it
    // verbatim (it does not decode); the receiving client decodes it in ClientP::notifySpoken.
    agent->speak(value.toString());
}

void ServerConnection::receiveOperate(const QJsonValue &value)
{
    agent->operate(value);
}

LogicRunnerP::LogicRunnerP(QMdmmCore::LogicConfiguration logicConfiguration, int playerNumPerRoom, LogicRunner *q)
    : QObject(q)
    , q(q)
    , conf(std::move(logicConfiguration))
    , playerNumPerRoom(playerNumPerRoom)
{
    logicThread = new QThread(this);
    logic = new QMdmmCore::Logic(conf);
    logic->moveToThread(logicThread);
    connect(logicThread, &QThread::finished, logic, &QMdmmCore::Logic::deleteLater);
    logicThread->start();

#define CONNECTRUNNERTOLOGIC(signalName) connect(this, &LogicRunnerP::signalName, logic, &QMdmmCore::Logic::signalName, Qt::QueuedConnection)

    CONNECTRUNNERTOLOGIC(addPlayer);
    CONNECTRUNNERTOLOGIC(removePlayer);
    CONNECTRUNNERTOLOGIC(roundStart);
    CONNECTRUNNERTOLOGIC(sscReply);
    CONNECTRUNNERTOLOGIC(actionOrderReply);
    CONNECTRUNNERTOLOGIC(actionReply);
    CONNECTRUNNERTOLOGIC(upgradeReply);

#undef CONNECTRUNNERTOLOGIC

#define CONNECTLOGICTORUNNER(signalName) connect(logic, &QMdmmCore::Logic::signalName, this, &LogicRunnerP::signalName, Qt::QueuedConnection)

    CONNECTLOGICTORUNNER(requestSscForAction);
    CONNECTLOGICTORUNNER(sscResult);
    CONNECTLOGICTORUNNER(requestActionOrder);
    CONNECTLOGICTORUNNER(actionOrderResult);
    CONNECTLOGICTORUNNER(requestSscForActionOrder);
    CONNECTLOGICTORUNNER(requestAction);
    CONNECTLOGICTORUNNER(actionResult);
    CONNECTLOGICTORUNNER(requestUpgrade);
    CONNECTLOGICTORUNNER(upgradeResult);
    CONNECTLOGICTORUNNER(roundOver);
    CONNECTLOGICTORUNNER(gameOver);

#undef CONNECTLOGICTORUNNER
}

void LogicRunnerP::agentStateChanged(const QMdmmCore::Data::AgentState &state)
{
    Agent *changedAgent = qobject_cast<Agent *>(sender());
    if (changedAgent == nullptr)
        return;

    foreach (Agent *agent, agents)
        agent->notifyAgentStateChange(changedAgent->objectName(), state);
}

void LogicRunnerP::agentSpoken(const QString &content)
{
    Agent *speakAgent = qobject_cast<Agent *>(sender());
    if (speakAgent == nullptr)
        return;

    if (!content.isEmpty()) {
        foreach (Agent *agent, agents)
            agent->notifySpeak(speakAgent->objectName(), content);
    }
}

void LogicRunnerP::agentOperated(const QJsonValue &value)
{
    Agent *operateAgent = qobject_cast<Agent *>(sender());
    if (operateAgent == nullptr)
        return;

    foreach (Agent *agent, agents)
        agent->notifyOperate(operateAgent->objectName(), value);
}

void LogicRunnerP::agentStoneScissorsClothReplied(QMdmmCore::Data::StoneScissorsCloth ssc)
{
    Agent *repliedAgent = qobject_cast<Agent *>(sender());
    if (repliedAgent == nullptr)
        return;

    emit sscReply(repliedAgent->objectName(), ssc);
}

void LogicRunnerP::agentActionOrderReplied(const QList<int> &order)
{
    Agent *repliedAgent = qobject_cast<Agent *>(sender());
    if (repliedAgent == nullptr)
        return;

    emit actionOrderReply(repliedAgent->objectName(), order);
}

void LogicRunnerP::agentActionReplied(QMdmmCore::Data::Action act, const QString &toPlayer, int toPlace)
{
    Agent *repliedAgent = qobject_cast<Agent *>(sender());
    if (repliedAgent == nullptr)
        return;

    emit actionReply(repliedAgent->objectName(), act, toPlayer, toPlace);
}

void LogicRunnerP::agentUpgradeReplied(const QList<QMdmmCore::Data::UpgradeItem> &items)
{
    Agent *repliedAgent = qobject_cast<Agent *>(sender());
    if (repliedAgent == nullptr)
        return;

    emit upgradeReply(repliedAgent->objectName(), items);
}

void LogicRunnerP::agentDisconnected(Agent *disconnectedAgent)
{
    if (disconnectedAgent == nullptr)
        return;

    if (q->full()) {
        // case 1: room is full, so game has started.
        // ServerConnection::onSocketDisconnected already marked the agent offline and
        // auto-replied the active request. The seat is preserved so the player can reconnect;
        // if they do not reconnect before the round is over, the game is abandoned:
        // LogicRunnerP::upgradeResult detects the still-offline agent and ends the game (see
        // the round-over check there).

        // if all agents are disconnected, terminate the game.
        bool allDisconnected = true;
        foreach (Agent *agent, agents) {
            if (agent->state().testFlag(QMdmmCore::Data::StateMaskOnline)) {
                allDisconnected = false;
                break;
            }
        }

        if (allDisconnected)
            emit q->gameOver(LogicRunner::QPrivateSignal());
    } else {
        // case 2: room is not full, so game hasn't started
        // Agent should be deleted.
        const QString playerName = disconnectedAgent->objectName();
        Agent *takenAgent = agents.take(playerName);
        ServerConnection *takenConn = connections.take(playerName);
        Q_UNUSED(takenAgent);
        Q_ASSERT(takenAgent == disconnectedAgent);
        Q_ASSERT(takenConn != nullptr);

        foreach (Agent *agent, agents)
            agent->notifyPlayerRemove(playerName);
        emit removePlayer(playerName);

        disconnectedAgent->deleteLater();
        takenConn->deleteLater();
    }
}

LogicRunnerP::~LogicRunnerP()
{
    // The logic lives in logicThread. Tear it down cleanly so the thread is no
    // longer running when this object (and thus logicThread, its child) is
    // destroyed -- otherwise Qt's QThread destructor hits
    // qFatal("QThread: Destroyed while thread ... is still running") and aborts.
    if (logic)
        logic->deleteLater();
    logicThread->quit();
    logicThread->wait();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogicRunnerP::requestSscForAction(const QStringList &playerNames)
{
    foreach (const QString &playerName, playerNames) {
        Agent *agent = agents.value(playerName);
        agent->requestStoneScissorsCloth(playerNames, 0);
    }
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogicRunnerP::sscResult(const QHash<QString, QMdmmCore::Data::StoneScissorsCloth> &replies)
{
    // Each agent records the round event it broadcasts in its connection's roundEventLog (see
    // ServerConnection::sendStoneScissorsClothNotified), for the per-agent reconnect catch-up.
    foreach (Agent *agent, agents)
        agent->notifyStoneScissorsCloth(replies);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogicRunnerP::requestActionOrder(const QString &playerName, const QList<int> &availableOrders, int maximumOrderNum, int selections)
{
    Agent *agent = agents.value(playerName);
    agent->requestActionOrder(availableOrders, maximumOrderNum, selections);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogicRunnerP::actionOrderResult(const QHash<int, QString> &result)
{
    foreach (Agent *agent, agents)
        agent->notifyActionOrder(result);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogicRunnerP::requestSscForActionOrder(const QStringList &playerNames, int strivedOrder)
{
    foreach (const QString &playerName, playerNames) {
        Agent *agent = agents.value(playerName);
        agent->requestStoneScissorsCloth(playerNames, strivedOrder);
    }
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogicRunnerP::requestAction(const QString &playerName, int actionOrder)
{
    Agent *agent = agents.value(playerName);
    agent->requestAction(actionOrder);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogicRunnerP::actionResult(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace)
{
    foreach (Agent *agent, agents)
        agent->notifyAction(playerName, action, toPlayer, toPlace);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogicRunnerP::requestUpgrade(const QString &playerName, int upgradePoint)
{
    Agent *agent = agents.value(playerName);
    agent->requestUpgrade(upgradePoint);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogicRunnerP::upgradeResult(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades)
{
    // Round-over abandonment check (see the comment in agentDisconnected): a player whose socket
    // dropped mid-round is auto-replied so the logic keeps advancing, but if they have not
    // reconnected by the time the round is over, the game cannot continue. End it as a game over
    // (winners = the still-online agents) instead of notifying the upgrade result, matching
    // QMdmmCore::Logic's game-over behavior.
    foreach (Agent *agent, agents) {
        if (!agent->state().testFlag(QMdmmCore::Data::StateMaskOnline)) {
            QStringList winners;
            foreach (Agent *onlineAgent, agents) {
                if (onlineAgent->state().testFlag(QMdmmCore::Data::StateMaskOnline))
                    winners << onlineAgent->objectName();
            }
            gameOver(winners); // broadcasts notifyGameOver and emits q->gameOver (D-023)
            return;
        }
    }

    foreach (Agent *agent, agents)
        agent->notifyUpgrade(upgrades);

    // The upgrade phase finished without a game over. Advance to the next round.
    // This mirrors the initial kick-off in addAgent(): announce the new round to
    // every agent (so clients reset their local room via notifyRoundStart) and
    // then start it. Without this, the match stalls after the very first round.
    foreach (Agent *agent, agents)
        agent->notifyRoundStart();

    // A new round begins: drop the previous round's events so the next round's log restarts empty
    // (the client resets its per-round event counter on notifyRoundStart, mirroring this).
    foreach (ServerConnection *conn, connections)
        conn->clearRoundEventLog();

    emit roundStart();
}

void LogicRunnerP::roundOver()
{
    // The round's action phase is over: drop each connection's round-event log. A reconnect from
    // here on is in the upgrade phase or the next round, where the old round's events are no longer
    // needed.
    foreach (ServerConnection *conn, connections)
        conn->clearRoundEventLog();

    foreach (Agent *agent, agents)
        agent->notifyRoundOver();
}

void LogicRunnerP::gameOver(const QStringList &winners)
{
    foreach (Agent *agent, agents)
        agent->notifyGameOver(winners);

    // A natural game over (a player maxed out all three stats) must tear down the room the same
    // way the round-over abandonment path does: emit LogicRunner::gameOver so the Server resets
    // `current` and deletes this runner (D-023). Without this, a naturally finished room leaks and
    // `current` keeps pointing at it.
    emit q->gameOver(LogicRunner::QPrivateSignal());
}
} // namespace p
#endif

#ifndef DOXYGEN
namespace v0 {
#endif

/**
 * @class LogicRunner
 * @brief The server-side object that runs a single complete game.
 *
 * A LogicRunner owns the agents (server-side representations of connected clients) and
 * runs a @c QMdmmCore::Logic on a separate thread. It handles exactly one complete game:
 * when the game is over, the LogicRunner should be destroyed and all agents disconnected,
 * and players who want to continue playing need to rejoin.
 *
 * @note This class is designed for one game only. Lobby / multi-room support is not
 * implemented yet.
 * @note All methods must be called in the Server thread, where the LogicRunner instance
 * lives.
 */

/**
 * @brief ctor.
 * @param logicConfiguration The configuration of the logic
 * @param playerNumPerRoom The player number per room
 * @param parent QObject parent.
 */
LogicRunner::LogicRunner(const QMdmmCore::LogicConfiguration &logicConfiguration, int playerNumPerRoom, QObject *parent)
    : QObject(parent)
    , d(new p::LogicRunnerP(logicConfiguration, playerNumPerRoom, this))
{
}

/**
 * @brief dtor.
 */
LogicRunner::~LogicRunner() = default;

/**
 * @brief Add a pre-wired agent to the game
 * @param agent the agent to add, already wired to its operation side (ServerConnection / GUI / Bot)
 * @return the added agent, or @c nullptr if the player name already exists or @p agent is @c nullptr
 *
 * This is the unified entry point for a player. The operation side creates the @c Agent and wires
 * it up before calling this -- for the network path, ServerP also creates a @c ServerConnection
 * (a child of the agent) and binds the socket on it; for a local player, GUI / Bot simply own the
 * agent. This function only registers the agent with the logic side: it inserts the agent into the
 * room, connects the agent's logic-port signals (replies / speech / operation) to the room, and
 * broadcasts the join to every player. The socket / wire lifecycle is not LogicRunner's concern.
 */
Agent *LogicRunner::addAgent(Agent *agent)
{
    if (agent == nullptr)
        return nullptr;

    const QString playerName = agent->objectName();
    if (d->agents.contains(playerName))
        return nullptr;

    d->agents.insert(playerName, agent);

    // Register the agent's wire plumbing, if the operation side created one (network path). The
    // ServerConnection is a child of the agent so it travels with it; it reports the socket drop
    // as an Agent event (agentDisconnected) that the room listens to.
    if (p::ServerConnection *conn = agent->findChild<p::ServerConnection *>(); conn != nullptr) {
        connect(conn, &p::ServerConnection::agentDisconnected, d, &p::LogicRunnerP::agentDisconnected);
        d->connections.insert(playerName, conn);
    }

    // Connect the agent's logic-port signals to the room (identity change / speech / operation /
    // replies). The operation port (ServerConnection / GUI / Bot) is wired by whoever created the
    // agent, not here.
    connect(agent, &Agent::stateChanged, d, &p::LogicRunnerP::agentStateChanged);
    connect(agent, &Agent::spoken, d, &p::LogicRunnerP::agentSpoken);
    connect(agent, &Agent::operated, d, &p::LogicRunnerP::agentOperated);
    connect(agent, &Agent::replyStoneScissorsCloth, d, &p::LogicRunnerP::agentStoneScissorsClothReplied);
    connect(agent, &Agent::replyActionOrder, d, &p::LogicRunnerP::agentActionOrderReplied);
    connect(agent, &Agent::replyAction, d, &p::LogicRunnerP::agentActionReplied);
    connect(agent, &Agent::replyUpgrade, d, &p::LogicRunnerP::agentUpgradeReplied);

    // When a new agent is added, first we'd notify the logic configuration to client
    // This is also a signal to client that it should switch state for room data

    agent->notifyLogicConfiguration();

    emit d->addPlayer(playerName);

    foreach (Agent *a, d->agents)
        a->notifyPlayerAdd(playerName, agent->screenName(), agent->state());

    // Tell the newly added agent about every player that joined before it.
    // NOTE: iterate over the *existing* agents and report their identities,
    // not the new player's name again (that would duplicate notifyPlayerAdd
    // for the new agent and make the client treat it as a fatal error).
    foreach (Agent *a, d->agents) {
        if (a != agent)
            agent->notifyPlayerAdd(a->objectName(), a->screenName(), a->state());
    }

    if (full()) {
        foreach (Agent *a, d->agents)
            a->notifyGameStart();
        foreach (Agent *a, d->agents)
            a->notifyRoundStart();
        emit d->roundStart();
    }

    return agent;
}

/**
 * @brief Reconnect a previously disconnected agent by restoring its state and room snapshot
 * @param agent the offline agent to reconnect
 * @return the reconnected agent, or @c nullptr if @p agent is @c nullptr, not in this room, or still online
 *
 * A reconnect only makes sense for a player who is already in the room (the room is full, so the
 * game has started) but whose socket was cleared by @c ServerConnection::onSocketDisconnected,
 * which also marked the agent offline. This is the logic-side half of a reconnect: it restores
 * the online flag (not Trust -- the "managed" flag, see @c StateMaskTrust) and resends the state
 * snapshot so the reconnecting client can rebuild its room view. The wire-side half (rebind the
 * socket + replay the missed round events) is
 * @c ServerConnection::reconnect, called by the operation side (ServerP) that owns the socket.
 * The room itself only ever deals with agents, never sockets (D-018).
 */
Agent *LogicRunner::reconnectAgent(Agent *agent)
{
    if (agent == nullptr)
        return nullptr;

    // Only an agent that is already in this room can be reconnected.
    if (d->agents.value(agent->objectName(), nullptr) != agent)
        return nullptr;

    // A still-online agent is not a reconnect candidate: only an agent that
    // onSocketDisconnected marked offline can be reconnected here.
    if (agent->state().testFlag(QMdmmCore::Data::StateMaskOnline))
        return nullptr;

    // Restore the online flag that onSocketDisconnected cleared. Trust (the "managed" flag) is
    // NOT restored by default: a reconnecting player must not be automatically trusted -- a
    // managed player still replies from its own client, and only a dropped player gets the
    // server-side default reply. setState emits stateChanged, which LogicRunnerP::agentStateChanged
    // turns into a notifyAgentStateChange broadcast to every agent -- this is what lets the other,
    // still-connected clients see that the player is back online.
    QMdmmCore::Data::AgentState state = agent->state();
    state.setFlag(QMdmmCore::Data::StateMaskOnline, true);
    agent->setState(state);

    // Resend the state snapshot to the reconnected client so it can rebuild its room: the logic
    // configuration, then every player (identity + current state). The client treats a duplicate
    // notifyPlayerAdd as benign (ClientP::notifyPlayerAdded), and the notifyAgentStateChange
    // broadcast above is dropped by the reconnected client until it has learned the players here.
    agent->notifyLogicConfiguration();

    foreach (Agent *a, d->agents)
        agent->notifyPlayerAdd(a->objectName(), a->screenName(), a->state());

    return agent;
}

/**
 * @brief get the agent of a specific internal name
 * @param playerName the internal name of the searched agent
 * @return the agent of the internal name, or @c nullptr if not found
 */
Agent *LogicRunner::agent(const QString &playerName)
{
    return d->agents.value(playerName, nullptr);
}

/**
 * @brief get the agent of a specific internal name (const version)
 * @param playerName the internal name of the searched agent
 * @return the agent of the internal name, or @c nullptr if not found
 */
const Agent *LogicRunner::agent(const QString &playerName) const
{
    return d->agents.value(playerName, nullptr);
}

/**
 * @brief if the room is full
 * @return @c true if the number of agents reaches @c ServerConfiguration::playerNumPerRoom
 */
bool LogicRunner::full() const
{
    return d->agents.count() >= d->playerNumPerRoom;
}

/**
 * @fn LogicRunner::gameOver(QPrivateSignal)
 * @brief emitted when the game is over
 */

#ifndef DOXYGEN
} // namespace v0
#endif
} // namespace QMdmmNetworking
