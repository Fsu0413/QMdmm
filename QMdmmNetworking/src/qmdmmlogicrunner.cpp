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

QHash<QMdmmCore::Protocol::NotifyId, void (ServerAgentP::*)(const QJsonValue &)> ServerAgentP::notifyCallback {
    std::make_pair(QMdmmCore::Protocol::NotifySpeak, &ServerAgentP::notifySpeak),
    std::make_pair(QMdmmCore::Protocol::NotifyOperate, &ServerAgentP::notifyOperate),
};

QHash<QMdmmCore::Protocol::RequestId, void (ServerAgentP::*)(const QJsonValue &)> ServerAgentP::replyCallback {
    std::make_pair(QMdmmCore::Protocol::RequestStoneScissorsCloth, &ServerAgentP::replyStoneScissorsCloth),
    std::make_pair(QMdmmCore::Protocol::RequestActionOrder, &ServerAgentP::replyActionOrder),
    std::make_pair(QMdmmCore::Protocol::RequestAction, &ServerAgentP::replyAction),
    std::make_pair(QMdmmCore::Protocol::RequestUpgrade, &ServerAgentP::replyUpgrade),
};

QHash<QMdmmCore::Protocol::RequestId, void (ServerAgentP::*)()> ServerAgentP::defaultReplyCallback {
    std::make_pair(QMdmmCore::Protocol::RequestStoneScissorsCloth, &ServerAgentP::defaultReplyStoneScissorsCloth),
    std::make_pair(QMdmmCore::Protocol::RequestActionOrder, &ServerAgentP::defaultReplyActionOrder),
    std::make_pair(QMdmmCore::Protocol::RequestAction, &ServerAgentP::defaultReplyAction),
    std::make_pair(QMdmmCore::Protocol::RequestUpgrade, &ServerAgentP::defaultReplyUpgrade),
};

int ServerAgentP::requestTimeoutGracePeriod = 60;

ServerAgentP::ServerAgentP(const QString &name, LogicRunnerP *parent)
    : Agent(name, parent)
    , p(parent)
    , currentRequest(QMdmmCore::Protocol::RequestInvalid)
    , requestTimer(new QTimer(this))
{
    requestTimer->setInterval(p->conf.requestTimeout() + requestTimeoutGracePeriod);
    requestTimer->setSingleShot(true);
    connect(requestTimer, &QTimer::timeout, this, &ServerAgentP::requestTimeout);
}

ServerAgentP::~ServerAgentP() = default;

void ServerAgentP::setSocket(Socket *_socket)
{
    if (socket != nullptr)
        socket->deleteLater();

    socket = _socket;
    if (socket != nullptr) {
        connect(socket, &Socket::packetReceived, this, &ServerAgentP::packetReceived);
        // Forward the socket drop directly to the room, so the room can find the
        // agent that owns the socket and mark it offline (a socket-disconnected
        // signal relayed through this agent would lose the identity of the agent).
        connect(socket, &Socket::socketDisconnected, p, &LogicRunnerP::socketDisconnected);
        connect(this, &ServerAgentP::sendPacket, socket, &Socket::sendPacket);
    }
}

void ServerAgentP::addRequest(QMdmmCore::Protocol::RequestId requestId, const QJsonValue &value)
{
    currentRequest = requestId;
    currentRequestValue = value;

    if (socket != nullptr) {
        emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::TypeRequest, requestId, value));
        requestTimer->start();
    } else {
        // We'd make this default reply in the event queue
        // reasons are:
        // 1. introducing time-consuming task in the request function is bad.
        // 2. reply should be called asynchronous since this is the designed way for it
        QTimer::singleShot(0, Qt::CoarseTimer, this, &ServerAgentP::executeDefaultReply);
    }
}

void ServerAgentP::replyStoneScissorsCloth(const QJsonValue &value)
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

    emit p->sscReply(objectName(), ssc);

#undef DEFAULTREPLY
}

void ServerAgentP::replyActionOrder(const QJsonValue &value)
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

    emit p->actionOrderReply(objectName(), ao);

#undef DEFAULTREPLY
}

void ServerAgentP::replyAction(const QJsonValue &value)
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
    QMdmmCore::Data::Action action = static_cast<QMdmmCore::Data::Action>(vaction.toInt());
    switch (action) {
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
    switch (action) {
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
    switch (action) {
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

    emit p->actionReply(objectName(), action, toPlayer, toPlace);

#undef DEFAULTREPLY
}

void ServerAgentP::replyUpgrade(const QJsonValue &value)
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

    emit p->upgradeReply(objectName(), ups);

#undef DEFAULTREPLY
}

void ServerAgentP::defaultReplyStoneScissorsCloth()
{
    emit p->sscReply(objectName(), static_cast<QMdmmCore::Data::StoneScissorsCloth>(QRandomGenerator::global()->generate() % 3));
}

void ServerAgentP::defaultReplyActionOrder()
{
    QJsonObject ob = currentRequestValue.toObject();
    QJsonArray arr = ob.value(QStringLiteral("remainedOrders")).toArray();
    int num = ob.value(QStringLiteral("selectionNum")).toInt();
    QList<int> ao;
    ao.reserve(num);
    while ((num--) != 0)
        ao.append(arr.takeAt(0).toInt());

    emit p->actionOrderReply(objectName(), ao);
}

void ServerAgentP::defaultReplyAction()
{
    emit p->actionReply(objectName(), QMdmmCore::Data::DoNothing, {}, 0);
}

void ServerAgentP::defaultReplyUpgrade()
{
    int times = currentRequestValue.toInt(1);
    QList<QMdmmCore::Data::UpgradeItem> ups;
    ups.reserve(times);
    while ((times--) != 0)
        ups << QMdmmCore::Data::UpgradeMaxHp;
    emit p->upgradeReply(objectName(), ups);
}

void ServerAgentP::packetReceived(const QMdmmCore::Packet &packet)
{
    if (socket == nullptr)
        return;

    if (packet.type() == QMdmmCore::Protocol::TypeNotify) {
        if ((packet.notifyId() & QMdmmCore::Protocol::NotifyToAgentMask) != 0) {
            void (ServerAgentP::*call)(const QJsonValue &) = notifyCallback.value(packet.notifyId(), nullptr);
            if (call != nullptr)
                (this->*call)(packet.value());
            else
                socket->setHasError(true);
        }
    } else if (packet.type() == QMdmmCore::Protocol::TypeReply) {
        if (currentRequest == packet.requestId()) {
            requestTimer->stop();
            currentRequest = QMdmmCore::Protocol::RequestInvalid;
            void (ServerAgentP::*call)(const QJsonValue &) = replyCallback.value(packet.requestId(), nullptr);
            if (call != nullptr)
                (this->*call)(packet.value());
            else
                socket->setHasError(true);
        }
    }
}

void ServerAgentP::requestStoneScissorsCloth(const QStringList &playerNames, int strivedOrder)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerNames"), QJsonArray::fromStringList(playerNames));
    ob.insert(QStringLiteral("strivedOrder"), strivedOrder);
    addRequest(QMdmmCore::Protocol::RequestStoneScissorsCloth, ob);
}

void ServerAgentP::requestActionOrder(const QList<int> &remainedOrders, int maximumOrder, int selectionNum)
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

void ServerAgentP::requestAction(int currentOrder)
{
    addRequest(QMdmmCore::Protocol::RequestAction, currentOrder);
}

void ServerAgentP::requestUpgrade(int remainingTimes)
{
    addRequest(QMdmmCore::Protocol::RequestUpgrade, remainingTimes);
}

void ServerAgentP::notifyLogicConfiguration()
{
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyLogicConfiguration, p->conf));
}

void ServerAgentP::notifyAgentStateChanged(const QString &playerName, const QMdmmCore::Data::AgentState &agentState)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerName"), playerName);
    ob.insert(QStringLiteral("agentState"), static_cast<int>(QMdmmCore::Data::AgentState::Int(agentState)));
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyAgentStateChanged, ob));
}

void ServerAgentP::notifyPlayerAdded(const QString &playerName, const QString &screenName, const QMdmmCore::Data::AgentState &agentState)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerName"), playerName);
    ob.insert(QStringLiteral("screenName"), screenName);
    ob.insert(QStringLiteral("agentState"), static_cast<int>(QMdmmCore::Data::AgentState::Int(agentState)));
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyPlayerAdded, ob));
}

void ServerAgentP::notifyPlayerRemoved(const QString &playerName)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerName"), playerName);
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyPlayerRemoved, ob));
}

void ServerAgentP::notifyGameStart()
{
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyGameStart, {}));
}

void ServerAgentP::notifyRoundStart()
{
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyRoundStart, {}));
}

QMdmmCore::Packet ServerAgentP::notifyStoneScissorsCloth(const QHash<QString, QMdmmCore::Data::StoneScissorsCloth> &replies, int seq)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("seq"), seq);
    for (QHash<QString, QMdmmCore::Data::StoneScissorsCloth>::const_iterator it = replies.constBegin(); it != replies.constEnd(); ++it)
        ob.insert(it.key(), static_cast<int>(it.value()));
    QMdmmCore::Packet packet(QMdmmCore::Protocol::NotifyStoneScissorsCloth, ob);
    emit sendPacket(packet);
    return packet;
}

QMdmmCore::Packet ServerAgentP::notifyActionOrder(const QHash<int, QString> &result, int seq)
{
    QJsonArray arr;
    for (int i = 1; i <= result.count(); ++i)
        arr.append(result.value(i));
    QJsonObject ob;
    ob.insert(QStringLiteral("seq"), seq);
    ob.insert(QStringLiteral("order"), arr);
    QMdmmCore::Packet packet(QMdmmCore::Protocol::NotifyActionOrder, ob);
    emit sendPacket(packet);
    return packet;
}

QMdmmCore::Packet ServerAgentP::notifyAction(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace, int seq)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("seq"), seq);
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
    emit sendPacket(packet);
    return packet;
}

void ServerAgentP::notifyRoundOver()
{
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyRoundOver, {}));
}

QMdmmCore::Packet ServerAgentP::notifyUpgrade(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades, int seq)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("seq"), seq);
    for (QHash<QString, QList<QMdmmCore::Data::UpgradeItem>>::const_iterator it = upgrades.constBegin(); it != upgrades.constEnd(); ++it) {
        QJsonArray arr;
        foreach (QMdmmCore::Data::UpgradeItem up, it.value())
            arr.append(static_cast<int>(up));
        ob.insert(it.key(), arr);
    }
    QMdmmCore::Packet packet(QMdmmCore::Protocol::NotifyUpgrade, ob);
    emit sendPacket(packet);
    return packet;
}

void ServerAgentP::notifyGameOver(const QStringList &playerNames)
{
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyGameOver, QJsonArray::fromStringList(playerNames)));
}

void ServerAgentP::notifySpoken(const QString &playerName, const QString &content)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerName"), playerName);
    ob.insert(QStringLiteral("content"), content);
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifySpoken, ob));
}

void ServerAgentP::notifyOperated(const QString &playerName, const QJsonValue &todo)
{
    Q_UNIMPLEMENTED();
    Q_UNUSED(playerName);
    Q_UNUSED(todo);
}

void ServerAgentP::requestTimeout()
{
    if (socket != nullptr)
        socket->setHasError(true);
    executeDefaultReply();
}

void ServerAgentP::executeDefaultReply()
{
    if (currentRequest != QMdmmCore::Protocol::RequestInvalid) {
        void (ServerAgentP::*call)() = defaultReplyCallback.value(currentRequest, nullptr);
        currentRequest = QMdmmCore::Protocol::RequestInvalid;
        if (call != nullptr)
            (this->*call)();
    }
}

LogicRunnerP::LogicRunnerP(QMdmmCore::LogicConfiguration logicConfiguration, LogicRunner *q)
    : QObject(q)
    , q(q)
    , conf(std::move(logicConfiguration))
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
    ServerAgentP *changedAgent = qobject_cast<ServerAgentP *>(sender());
    if (changedAgent == nullptr)
        return;

    foreach (ServerAgentP *agent, agents)
        agent->notifyAgentStateChanged(changedAgent->objectName(), state);
}

void LogicRunnerP::agentSpoken(const QJsonValue &value)
{
    ServerAgentP *speakAgent = qobject_cast<ServerAgentP *>(sender());
    if (speakAgent == nullptr)
        return;

    QString s = value.toString();
    if (!s.isEmpty()) {
        foreach (ServerAgentP *agent, agents)
            agent->notifySpoken(speakAgent->objectName(), s);
    }
}

void LogicRunnerP::agentOperated(const QJsonValue &value)
{
    ServerAgentP *operateAgent = qobject_cast<ServerAgentP *>(sender());
    if (operateAgent == nullptr)
        return;

    foreach (ServerAgentP *agent, agents)
        agent->notifyOperated(operateAgent->objectName(), value);
}

void LogicRunnerP::socketDisconnected()
{
    Socket *disconnectedSocket = qobject_cast<Socket *>(sender());
    if (disconnectedSocket == nullptr)
        return;

    // The socket is connected directly (see ServerAgentP::setSocket), so sender()
    // is the Socket, not the agent. Locate the agent that owns it.
    ServerAgentP *disconnectedAgent = nullptr;
    foreach (ServerAgentP *agent, agents) {
        if (agent->socket == disconnectedSocket) {
            disconnectedAgent = agent;
            break;
        }
    }
    if (disconnectedAgent == nullptr)
        return;

    disconnectedAgent->socket->deleteLater();
    disconnectedAgent->socket = nullptr;

    if (q->full()) {
        // case 1: room is full, so game has started
        // Agent should exit game if round over or logic runs pass round over, which makes game over and the logic quits
        // But if client is reconnected before round over, the game should continue
        // TODO: round over - implement it in LogicRunnerP::upgradeResult, iterate all agents and check if they are online
        // before notifying agent->notifyUpgrade to everyone. Run gameover if at least one agent is offline.
        // This matches the gameover behavior of QMdmmCore::Logic

        QMdmmCore::Data::AgentState state = disconnectedAgent->state();
        state.setFlag(QMdmmCore::Data::StateMaskOnline, false).setFlag(QMdmmCore::Data::StateMaskTrust, false);
        disconnectedAgent->setState(state);

        // if all agents are disconnected, terminate the game.
        bool allDisconnected = true;
        foreach (ServerAgentP *agent, agents) {
            if (agent->state().testFlag(QMdmmCore::Data::StateMaskOnline)) {
                allDisconnected = false;
                break;
            }
        }

        if (allDisconnected) {
            emit q->gameOver(LogicRunner::QPrivateSignal());
            return;
        }

        // If there is an active request, use default reply
        // QMdmmNetworking::p::ServerAgentP::executeDefaultReply handles it even if there is no active request
        disconnectedAgent->executeDefaultReply();
    } else {
        // case 2: room is not full, so game hasn't started
        // Agent should be deleted.
        ServerAgentP *taken = agents.take(disconnectedAgent->objectName());
        Q_UNUSED(taken);
        Q_ASSERT(taken == disconnectedAgent);

        foreach (ServerAgentP *agent, agents)
            agent->notifyPlayerRemoved(disconnectedAgent->objectName());
        emit removePlayer(disconnectedAgent->objectName());

        disconnectedAgent->deleteLater();
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
        ServerAgentP *agent = agents.value(playerName);
        agent->requestStoneScissorsCloth(playerNames, 0);
    }
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogicRunnerP::sscResult(const QHash<QString, QMdmmCore::Data::StoneScissorsCloth> &replies)
{
    const int seq = ++roundEventSeq;
    // The packet is identical for every agent, so capture it while broadcasting and cache it
    // keyed by seq for the reconnect catch-up (see roundEventCache in the header).
    QMdmmCore::Packet packet;
    foreach (ServerAgentP *agent, agents)
        packet = agent->notifyStoneScissorsCloth(replies, seq);
    roundEventCache.insert(seq, packet);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogicRunnerP::requestActionOrder(const QString &playerName, const QList<int> &availableOrders, int maximumOrderNum, int selections)
{
    ServerAgentP *agent = agents.value(playerName);
    agent->requestActionOrder(availableOrders, maximumOrderNum, selections);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogicRunnerP::actionOrderResult(const QHash<int, QString> &result)
{
    const int seq = ++roundEventSeq;
    QMdmmCore::Packet packet;
    foreach (ServerAgentP *agent, agents)
        packet = agent->notifyActionOrder(result, seq);
    roundEventCache.insert(seq, packet);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogicRunnerP::requestSscForActionOrder(const QStringList &playerNames, int strivedOrder)
{
    foreach (const QString &playerName, playerNames) {
        ServerAgentP *agent = agents.value(playerName);
        agent->requestStoneScissorsCloth(playerNames, strivedOrder);
    }
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogicRunnerP::requestAction(const QString &playerName, int actionOrder)
{
    ServerAgentP *agent = agents.value(playerName);
    agent->requestAction(actionOrder);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogicRunnerP::actionResult(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace)
{
    const int seq = ++roundEventSeq;
    QMdmmCore::Packet packet;
    foreach (ServerAgentP *agent, agents)
        packet = agent->notifyAction(playerName, action, toPlayer, toPlace, seq);
    roundEventCache.insert(seq, packet);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogicRunnerP::requestUpgrade(const QString &playerName, int upgradePoint)
{
    ServerAgentP *agent = agents.value(playerName);
    agent->requestUpgrade(upgradePoint);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogicRunnerP::upgradeResult(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades)
{
    const int seq = ++roundEventSeq;
    QMdmmCore::Packet packet;
    foreach (ServerAgentP *agent, agents)
        packet = agent->notifyUpgrade(upgrades, seq);
    roundEventCache.insert(seq, packet);

    // The upgrade phase finished without a game over. Advance to the next round.
    // This mirrors the initial kick-off in addSocket(): announce the new round to
    // every agent (so clients reset their local room via notifyRoundStart) and
    // then start it. Without this, the match stalls after the very first round.
    foreach (ServerAgentP *agent, agents)
        agent->notifyRoundStart();

    // A new round begins: reset the round-event sequence and drop the previous round's events
    // so the next round's events restart from 1 (the client tracks "last received seq" per round).
    roundEventSeq = 0;
    roundEventCache.clear();

    emit roundStart();
}

void LogicRunnerP::roundOver()
{
    // The round's action phase is over: drop the round-event cache. A reconnect from here on is
    // in the upgrade phase or the next round, where the old round's events are no longer needed.
    roundEventCache.clear();

    foreach (ServerAgentP *agent, agents)
        agent->notifyRoundOver();
}

void LogicRunnerP::gameOver(const QStringList &winners)
{
    foreach (ServerAgentP *agent, agents)
        agent->notifyGameOver(winners);
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
 * when the game is over, the LogicRunner should be destroyed and all agents disconnected.
 *
 * @note This class is designed for one game only. Lobby / multi-room support is not
 * implemented yet.
 */

/**
 * @brief ctor.
 * @param logicConfiguration The configuration of the logic
 * @param parent QObject parent.
 */
LogicRunner::LogicRunner(const QMdmmCore::LogicConfiguration &logicConfiguration, QObject *parent)
    : QObject(parent)
    , d(new p::LogicRunnerP(logicConfiguration, this))
{
}

/**
 * @brief dtor.
 */
LogicRunner::~LogicRunner() = default;

/**
 * @brief Add a socket (a connected agent) to the game
 * @param playerName the internal name of the player
 * @param screenName the screen name of the player
 * @param agentState the initial state of the agent
 * @param socket the socket of the connected client
 * @return the newly added agent, or @c nullptr if the player name already exists
 */
Agent *LogicRunner::addSocket(const QString &playerName, const QString &screenName, const QMdmmCore::Data::AgentState &agentState, Socket *socket)
{
    if (d->agents.contains(playerName))
        return nullptr;

    p::ServerAgentP *addedAgent = new p::ServerAgentP(playerName, d);
    addedAgent->setScreenName(screenName);
    addedAgent->setState(agentState);

    addedAgent->setSocket(socket);
    d->agents.insert(playerName, addedAgent);

    connect(addedAgent, &p::ServerAgentP::stateChanged, d, &p::LogicRunnerP::agentStateChanged);
    connect(addedAgent, &p::ServerAgentP::notifySpeak, d, &p::LogicRunnerP::agentSpoken);
    connect(addedAgent, &p::ServerAgentP::notifyOperate, d, &p::LogicRunnerP::agentOperated);

    // When a new agent is added, first we'd notify the logic configuration to client
    // This is also a signal to client that it should switch state for room data

    addedAgent->notifyLogicConfiguration();

    emit d->addPlayer(playerName);

    foreach (p::ServerAgentP *agent, d->agents)
        agent->notifyPlayerAdded(playerName, screenName, agentState);

    // Tell the newly added agent about every player that joined before it.
    // NOTE: iterate over the *existing* agents and report their identities,
    // not the new player's name again (that would duplicate notifyPlayerAdded
    // for the new agent and make the client treat it as a fatal error).
    foreach (p::ServerAgentP *agent, d->agents) {
        if (agent != addedAgent)
            addedAgent->notifyPlayerAdded(agent->objectName(), agent->screenName(), agent->state());
    }

    if (full()) {
        foreach (p::ServerAgentP *agent, d->agents)
            agent->notifyGameStart();
        foreach (p::ServerAgentP *agent, d->agents)
            agent->notifyRoundStart();
        emit d->roundStart();
    }

    return addedAgent;
}

/**
 * @brief Reconnect a previously disconnected agent by rebinding its socket
 * @param playerName the internal name of the player to reconnect
 * @param socket the new socket of the reconnecting client
 * @return the reconnected agent, or @c nullptr if the player is unknown or still connected
 *
 * A reconnect only makes sense for a player who is already in the room (the room is full, so the
 * game has started) but whose socket was cleared by @c LogicRunnerP::socketDisconnected. It
 * rebinds the socket, restores the online / trust flags, and resends the state snapshot so the
 * reconnecting client can rebuild its room view.
 */
Agent *LogicRunner::reconnect(const QString &playerName, Socket *socket)
{
    p::ServerAgentP *reconnectedAgent = d->agents.value(playerName, nullptr);
    if (reconnectedAgent == nullptr)
        return nullptr;

    // A still-connected agent is not a reconnect candidate: only an agent whose socket was
    // cleared by socketDisconnected can be rebound here.
    if (reconnectedAgent->socket != nullptr)
        return nullptr;

    // Rebind the socket. setSocket is safe because socketDisconnected already set the old socket
    // to nullptr before this point, so nothing gets double-deleted.
    reconnectedAgent->setSocket(socket);

    // Restore the online / trust flags that socketDisconnected cleared. setState emits
    // stateChanged, which LogicRunnerP::agentStateChanged turns into a notifyAgentStateChanged
    // broadcast to every agent -- this is what lets the other, still-connected clients see that
    // the player is back online.
    QMdmmCore::Data::AgentState state = reconnectedAgent->state();
    state.setFlag(QMdmmCore::Data::StateMaskOnline, true).setFlag(QMdmmCore::Data::StateMaskTrust, true);
    reconnectedAgent->setState(state);

    // Resend the state snapshot to the reconnected client so it can rebuild its room: the logic
    // configuration, then every player (identity + current state). The client treats a duplicate
    // notifyPlayerAdded as benign (ClientP::notifyPlayerAdded), and the notifyAgentStateChanged
    // broadcast above is dropped by the reconnected client until it has learned the players here.
    reconnectedAgent->notifyLogicConfiguration();

    foreach (p::ServerAgentP *agent, d->agents)
        reconnectedAgent->notifyPlayerAdded(agent->objectName(), agent->screenName(), agent->state());

    // TODO: resending notifyGameStart/notifyRoundStart here is wrong. The reconnecting client never
    // left the round (only its socket dropped), so a replayed RoundStart makes it clear its local
    // round state and desync from the server. Replace with precise catch-up: the client reports its
    // last received round-event sequence number on reconnect, and the server replays only the missed
    // events (ssc / action-order / action / upgrade) instead of re-announcing the round.
    reconnectedAgent->notifyGameStart();
    reconnectedAgent->notifyRoundStart();

    return reconnectedAgent;
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
 * @return @c true if the number of agents reaches @c LogicConfiguration::playerNumPerRoom
 */
bool LogicRunner::full() const
{
    return d->agents.count() >= d->conf.playerNumPerRoom();
}

/**
 * @fn LogicRunner::gameOver(QPrivateSignal)
 * @brief emitted when the game is over
 */

#ifndef DOXYGEN
} // namespace v0
#endif
} // namespace QMdmmNetworking
