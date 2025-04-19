// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmlogicrunner.h"
#include "qmdmmlogicrunner_p.h"

#include <QJsonArray>
#include <QMetaType>
#include <QRandomGenerator>
#include <utility>

namespace QMdmmNetworking {
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
        connect(socket, &Socket::socketDisconnected, this, &ServerAgentP::socketDisconnected);
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

void ServerAgentP::notifyStoneScissorsCloth(const QHash<QString, QMdmmCore::Data::StoneScissorsCloth> &replies)
{
    QJsonObject ob;
    for (QHash<QString, QMdmmCore::Data::StoneScissorsCloth>::const_iterator it = replies.constBegin(); it != replies.constEnd(); ++it)
        ob.insert(it.key(), static_cast<int>(it.value()));
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyStoneScissorsCloth, ob));
}

void ServerAgentP::notifyActionOrder(const QHash<int, QString> &result)
{
    QJsonArray arr;
    for (int i = 1; i < result.count(); ++i)
        arr.append(result.value(i));
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyActionOrder, arr));
}

void ServerAgentP::notifyAction(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace)
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

    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyAction, ob));
}

void ServerAgentP::notifyRoundOver()
{
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyRoundOver, {}));
}

void ServerAgentP::notifyUpgrade(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades)
{
    QJsonObject ob;
    for (QHash<QString, QList<QMdmmCore::Data::UpgradeItem>>::const_iterator it = upgrades.constBegin(); it != upgrades.constEnd(); ++it) {
        QJsonArray arr;
        foreach (QMdmmCore::Data::UpgradeItem up, it.value())
            arr.append(static_cast<int>(up));
        ob.insert(it.key(), arr);
    }
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyUpgrade, ob));
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
    QMdmmCore::Packet p(QMdmmCore::Protocol::NotifySpoken, ob);
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
    ServerAgentP *disconnectedAgent = qobject_cast<ServerAgentP *>(sender());
    if (disconnectedAgent == nullptr)
        return;

    disconnectedAgent->socket->deleteLater();
    disconnectedAgent->socket = nullptr;

    if (q->full()) {
        // case 1: room is full, so game has started
        // Agent should exit game if round over or logic runs pass round over, which makes game over and the logic quits
        // But if client is reconnected before round over, the game should continue
        // TODO: round over

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
        // QMdmmServerAgentPrivate::executeDefaultReply handles it even if there is not active request
        disconnectedAgent->executeDefaultReply();
    } else {
        // case 2: room is not full, so game hasn't started
        // Agent should be deleted.
        ServerAgentP *taken = agents.take(objectName());
        Q_UNUSED(taken);
        Q_ASSERT(taken == disconnectedAgent);

        foreach (ServerAgentP *agent, agents)
            agent->notifyPlayerRemoved(disconnectedAgent->objectName());
        emit removePlayer(disconnectedAgent->objectName());

        disconnectedAgent->deleteLater();
    }
}

LogicRunnerP::~LogicRunnerP() = default;

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
    foreach (ServerAgentP *agent, agents)
        agent->notifyStoneScissorsCloth(replies);
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
    foreach (ServerAgentP *agent, agents)
        agent->notifyActionOrder(result);
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
    foreach (ServerAgentP *agent, agents)
        agent->notifyAction(playerName, action, toPlayer, toPlace);
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
    foreach (ServerAgentP *agent, agents)
        agent->notifyUpgrade(upgrades);
}
} // namespace p

namespace v0 {
LogicRunner::LogicRunner(const QMdmmCore::LogicConfiguration &logicConfiguration, QObject *parent)
    : QObject(parent)
    , d(new p::LogicRunnerP(logicConfiguration, this))
{
}

LogicRunner::~LogicRunner() = default;

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
    foreach (p::ServerAgentP *agent, d->agents) {
        if (agent != addedAgent)
            addedAgent->notifyPlayerAdded(playerName, screenName, agentState);
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

Agent *LogicRunner::agent(const QString &playerName)
{
    return d->agents.value(playerName, nullptr);
}

const Agent *LogicRunner::agent(const QString &playerName) const
{
    return d->agents.value(playerName, nullptr);
}

bool LogicRunner::full() const
{
    return d->agents.count() >= d->conf.playerNumPerRoom();
}
} // namespace v0
} // namespace QMdmmNetworking
