// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmlogicrunner.h"
#include "qmdmmlogicrunner_p.h"

#include <QJsonArray>
#include <QMetaType>
#include <QRandomGenerator>
#include <utility>

QHash<QMdmmProtocol::NotifyId, void (QMdmmServerAgentPrivate::*)(const QJsonValue &)> QMdmmServerAgentPrivate::notifyCallback {
    std::make_pair(QMdmmProtocol::NotifySpeak, &QMdmmServerAgentPrivate::notifySpeak),
    std::make_pair(QMdmmProtocol::NotifyOperate, &QMdmmServerAgentPrivate::notifyOperate),
};

QHash<QMdmmProtocol::RequestId, void (QMdmmServerAgentPrivate::*)(const QJsonValue &)> QMdmmServerAgentPrivate::replyCallback {
    std::make_pair(QMdmmProtocol::RequestStoneScissorsCloth, &QMdmmServerAgentPrivate::replyStoneScissorsCloth),
    std::make_pair(QMdmmProtocol::RequestActionOrder, &QMdmmServerAgentPrivate::replyActionOrder),
    std::make_pair(QMdmmProtocol::RequestAction, &QMdmmServerAgentPrivate::replyAction),
    std::make_pair(QMdmmProtocol::RequestUpgrade, &QMdmmServerAgentPrivate::replyUpgrade),
};

QHash<QMdmmProtocol::RequestId, void (QMdmmServerAgentPrivate::*)()> QMdmmServerAgentPrivate::defaultReplyCallback {
    std::make_pair(QMdmmProtocol::RequestStoneScissorsCloth, &QMdmmServerAgentPrivate::defaultReplyStoneScissorsCloth),
    std::make_pair(QMdmmProtocol::RequestActionOrder, &QMdmmServerAgentPrivate::defaultReplyActionOrder),
    std::make_pair(QMdmmProtocol::RequestAction, &QMdmmServerAgentPrivate::defaultReplyAction),
    std::make_pair(QMdmmProtocol::RequestUpgrade, &QMdmmServerAgentPrivate::defaultReplyUpgrade),
};

int QMdmmServerAgentPrivate::requestTimeoutGracePeriod = 60;

QMdmmServerAgentPrivate::QMdmmServerAgentPrivate(const QString &name, QMdmmLogicRunnerPrivate *parent)
    : QMdmmAgent(name, parent)
    , p(parent)
    , currentRequest(QMdmmProtocol::RequestInvalid)
    , requestTimer(new QTimer(this))
{
    requestTimer->setInterval(p->conf.requestTimeout() + requestTimeoutGracePeriod);
    requestTimer->setSingleShot(true);
    connect(requestTimer, &QTimer::timeout, this, &QMdmmServerAgentPrivate::requestTimeout);
}

QMdmmServerAgentPrivate::~QMdmmServerAgentPrivate() = default;

void QMdmmServerAgentPrivate::setSocket(QMdmmSocket *_socket)
{
    if (socket != nullptr)
        socket->deleteLater();

    socket = _socket;
    if (socket != nullptr) {
        connect(socket, &QMdmmSocket::packetReceived, this, &QMdmmServerAgentPrivate::packetReceived);
        connect(socket, &QMdmmSocket::socketDisconnected, this, &QMdmmServerAgentPrivate::socketDisconnected);
        connect(this, &QMdmmServerAgentPrivate::sendPacket, socket, &QMdmmSocket::sendPacket);
    }
}

void QMdmmServerAgentPrivate::addRequest(QMdmmProtocol::RequestId requestId, const QJsonValue &value)
{
    currentRequest = requestId;
    currentRequestValue = value;

    if (socket != nullptr) {
        emit sendPacket(QMdmmPacket(QMdmmProtocol::TypeRequest, requestId, value));
        requestTimer->start();
    } else {
        // We'd make this default reply in the event queue
        // reasons are:
        // 1. introducing time-consuming task in the request function is bad.
        // 2. reply should be called asynchoronous since this is the designed way for it
        QTimer::singleShot(0, Qt::CoarseTimer, this, &QMdmmServerAgentPrivate::executeDefaultReply);
    }
}

void QMdmmServerAgentPrivate::replyStoneScissorsCloth(const QJsonValue &value)
{
#define DEFAULTREPLY                      \
    do {                                  \
        defaultReplyStoneScissorsCloth(); \
        return;                           \
    } while (0)

    if (!value.isDouble())
        DEFAULTREPLY;

    QMdmmData::StoneScissorsCloth ssc = static_cast<QMdmmData::StoneScissorsCloth>(value.toInt());
    switch (ssc) {
    case QMdmmData::Stone:
    case QMdmmData::Scissors:
    case QMdmmData::Cloth:
        break;
    default:
        DEFAULTREPLY;
    }

    emit p->sscReply(objectName(), ssc);

#undef DEFAULTREPLY
}

void QMdmmServerAgentPrivate::replyActionOrder(const QJsonValue &value)
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

void QMdmmServerAgentPrivate::replyAction(const QJsonValue &value)
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
    QMdmmData::Action action = static_cast<QMdmmData::Action>(vaction.toInt());
    switch (action) {
    case QMdmmData::DoNothing:
    case QMdmmData::BuyKnife:
    case QMdmmData::BuyHorse:
    case QMdmmData::Slash:
    case QMdmmData::Kick:
    case QMdmmData::Move:
    case QMdmmData::LetMove:
        break;
    default:
        DEFAULTREPLY;
    }

    QString toPlayer;
    switch (action) {
    case QMdmmData::Slash:
    case QMdmmData::Kick:
    case QMdmmData::LetMove: {
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
    case QMdmmData::Move:
    case QMdmmData::LetMove: {
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

void QMdmmServerAgentPrivate::replyUpgrade(const QJsonValue &value)
{
#define DEFAULTREPLY           \
    do {                       \
        defaultReplyUpgrade(); \
        return;                \
    } while (0)

    if (!value.isArray())
        DEFAULTREPLY;

    QJsonArray arr = value.toArray();
    QList<QMdmmData::UpgradeItem> ups;
    for (QJsonArray::const_iterator it = arr.constBegin(); it != arr.constEnd(); ++it) {
        if (!it->isDouble())
            DEFAULTREPLY;
        QMdmmData::UpgradeItem up = static_cast<QMdmmData::UpgradeItem>(it->toInt());
        switch (up) {
        case QMdmmData::UpgradeKnife:
        case QMdmmData::UpgradeHorse:
        case QMdmmData::UpgradeMaxHp:
            break;
        default:
            DEFAULTREPLY;
        }
        ups << up;
    }

    emit p->upgradeReply(objectName(), ups);

#undef DEFAULTREPLY
}

void QMdmmServerAgentPrivate::defaultReplyStoneScissorsCloth()
{
    replyStoneScissorsCloth(static_cast<int>(QRandomGenerator::global()->generate() % 3));
}

void QMdmmServerAgentPrivate::defaultReplyActionOrder()
{
    QJsonObject ob = currentRequestValue.toObject();
    QJsonArray arr = ob.value(QStringLiteral("remainedOrders")).toArray();
    int num = ob.value(QStringLiteral("selectionNum")).toInt();
    QJsonArray rep;
    while ((num--) != 0)
        rep.append(arr.takeAt(0));
    replyActionOrder(rep);
}

void QMdmmServerAgentPrivate::defaultReplyAction()
{
    QJsonObject ob;
    ob.insert(QStringLiteral("action"), static_cast<int>(QMdmmData::DoNothing));
    replyAction(ob);
}

void QMdmmServerAgentPrivate::defaultReplyUpgrade()
{
    int times = currentRequestValue.toInt(1);
    QJsonArray rep;
    while ((times--) != 0)
        rep.append(static_cast<int>(QMdmmData::UpgradeMaxHp));
    replyUpgrade(rep);
}

void QMdmmServerAgentPrivate::packetReceived(QMdmmPacket packet)
{
    if (socket == nullptr)
        return;

    if (packet.type() == QMdmmProtocol::TypeNotify) {
        if ((packet.notifyId() | QMdmmProtocol::NotifyToAgentMask) != 0) {
            void (QMdmmServerAgentPrivate::*call)(const QJsonValue &) = notifyCallback.value(packet.notifyId(), nullptr);
            if (call != nullptr)
                (this->*call)(packet.value());
            else
                socket->setHasError(true);
        }
    } else if (packet.type() == QMdmmProtocol::TypeReply) {
        if (currentRequest == packet.requestId()) {
            requestTimer->stop();
            currentRequest = QMdmmProtocol::RequestInvalid;
            void (QMdmmServerAgentPrivate::*call)(const QJsonValue &) = replyCallback.value(packet.requestId(), nullptr);
            if (call != nullptr)
                (this->*call)(packet.value());
            else
                socket->setHasError(true);
        }
    }
}

void QMdmmServerAgentPrivate::requestStoneScissorsCloth(const QStringList &playerNames, int strivedOrder)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerNames"), QJsonArray::fromStringList(playerNames));
    ob.insert(QStringLiteral("strivedOrder"), strivedOrder);
    addRequest(QMdmmProtocol::RequestStoneScissorsCloth, ob);
}

void QMdmmServerAgentPrivate::requestActionOrder(const QList<int> &remainedOrders, int maximumOrder, int selectionNum)
{
    QJsonObject ob;
    QJsonArray arr;
    foreach (int remainedOrder, remainedOrders)
        arr.append(remainedOrder);
    ob.insert(QStringLiteral("remainedOrders"), arr);
    ob.insert(QStringLiteral("maximumOrder"), maximumOrder);
    ob.insert(QStringLiteral("selectionNum"), selectionNum);
    addRequest(QMdmmProtocol::RequestActionOrder, ob);
}

void QMdmmServerAgentPrivate::requestAction(int currentOrder)
{
    addRequest(QMdmmProtocol::RequestAction, currentOrder);
}

void QMdmmServerAgentPrivate::requestUpgrade(int remainingTimes)
{
    addRequest(QMdmmProtocol::RequestUpgrade, remainingTimes);
}

void QMdmmServerAgentPrivate::notifyLogicConfiguration()
{
    emit sendPacket(QMdmmPacket(QMdmmProtocol::NotifyLogicConfiguration, p->conf));
}

void QMdmmServerAgentPrivate::notifyAgentStateChanged(const QString &playerName, const QMdmmData::AgentState &agentState)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerName"), playerName);
    ob.insert(QStringLiteral("agentState"), static_cast<int>(QMdmmData::AgentState::Int(agentState)));
    emit sendPacket(QMdmmPacket(QMdmmProtocol::NotifyAgentStateChanged, ob));
}

void QMdmmServerAgentPrivate::notifyPlayerAdded(const QString &playerName, const QString &screenName, const QMdmmData::AgentState &agentState)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerName"), playerName);
    ob.insert(QStringLiteral("screenName"), screenName);
    ob.insert(QStringLiteral("agentState"), static_cast<int>(QMdmmData::AgentState::Int(agentState)));
    emit sendPacket(QMdmmPacket(QMdmmProtocol::NotifyPlayerAdded, ob));
}

void QMdmmServerAgentPrivate::notifyPlayerRemoved(const QString &playerName)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerName"), playerName);
    emit sendPacket(QMdmmPacket(QMdmmProtocol::NotifyPlayerRemoved, ob));
}

void QMdmmServerAgentPrivate::notifyGameStart()
{
    emit sendPacket(QMdmmPacket(QMdmmProtocol::NotifyGameStart, {}));
}

void QMdmmServerAgentPrivate::notifyRoundStart()
{
    emit sendPacket(QMdmmPacket(QMdmmProtocol::NotifyRoundStart, {}));
}

void QMdmmServerAgentPrivate::notifyStoneScissorsCloth(const QHash<QString, QMdmmData::StoneScissorsCloth> &replies)
{
    QJsonObject ob;
    for (QHash<QString, QMdmmData::StoneScissorsCloth>::const_iterator it = replies.constBegin(); it != replies.constEnd(); ++it)
        ob.insert(it.key(), static_cast<int>(it.value()));
    emit sendPacket(QMdmmPacket(QMdmmProtocol::NotifyStoneScissorsCloth, ob));
}

void QMdmmServerAgentPrivate::notifyActionOrder(const QHash<int, QString> &result)
{
    QJsonArray arr;
    for (int i = 1; i < result.count(); ++i)
        arr.append(result.value(i));
    emit sendPacket(QMdmmPacket(QMdmmProtocol::NotifyActionOrder, arr));
}

void QMdmmServerAgentPrivate::notifyAction(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPlace)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerName"), playerName);
    ob.insert(QStringLiteral("action"), static_cast<int>(action));

    switch (action) {
    case QMdmmData::Slash:
    case QMdmmData::Kick:
    case QMdmmData::LetMove:
        ob.insert(QStringLiteral("toPlayer"), toPlayer);
        break;
    default:
        break;
    }

    bool hasToPlace = false;
    switch (action) {
    case QMdmmData::Move:
    case QMdmmData::LetMove:
        ob.insert(QStringLiteral("toPlace"), toPlace);
        break;
    default:
        break;
    }

    emit sendPacket(QMdmmPacket(QMdmmProtocol::NotifyAction, ob));
}

void QMdmmServerAgentPrivate::notifyRoundOver()
{
    emit sendPacket(QMdmmPacket(QMdmmProtocol::NotifyRoundOver, {}));
}

void QMdmmServerAgentPrivate::notifyUpgrade(const QHash<QString, QList<QMdmmData::UpgradeItem>> &upgrades)
{
    QJsonObject ob;
    for (QHash<QString, QList<QMdmmData::UpgradeItem>>::const_iterator it = upgrades.constBegin(); it != upgrades.constEnd(); ++it) {
        QJsonArray arr;
        foreach (QMdmmData::UpgradeItem up, it.value())
            arr.append(static_cast<int>(up));
        ob.insert(it.key(), arr);
    }
    emit sendPacket(QMdmmPacket(QMdmmProtocol::NotifyUpgrade, ob));
}

void QMdmmServerAgentPrivate::notifyGameOver(const QStringList &playerNames)
{
    emit sendPacket(QMdmmPacket(QMdmmProtocol::NotifyGameOver, QJsonArray::fromStringList(playerNames)));
}

void QMdmmServerAgentPrivate::notifySpoken(const QString &playerName, const QString &content)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerName"), playerName);
    ob.insert(QStringLiteral("content"), content);
    QMdmmPacket p(QMdmmProtocol::NotifySpoken, ob);
}

void QMdmmServerAgentPrivate::notifyOperated(const QString &playerName, const QJsonValue &todo)
{
    Q_UNIMPLEMENTED();
    Q_UNUSED(playerName);
    Q_UNUSED(todo);
}

void QMdmmServerAgentPrivate::requestTimeout()
{
    if (socket != nullptr)
        socket->setHasError(true);
    executeDefaultReply();
}

void QMdmmServerAgentPrivate::executeDefaultReply()
{
    if (currentRequest != QMdmmProtocol::RequestInvalid) {
        void (QMdmmServerAgentPrivate::*call)() = defaultReplyCallback.value(currentRequest, nullptr);
        currentRequest = QMdmmProtocol::RequestInvalid;
        if (call != nullptr)
            (this->*call)();
    }
}

QMdmmLogicRunnerPrivate::QMdmmLogicRunnerPrivate(QMdmmLogicConfiguration logicConfiguration, QMdmmLogicRunner *parent)
    : QObject(parent)
    , p(parent)
    , conf(std::move(logicConfiguration))
{
    logicThread = new QThread(this);
    logic = new QMdmmLogic(conf);
    logic->moveToThread(logicThread);
    connect(logicThread, &QThread::finished, logic, &QMdmmLogic::deleteLater);
    logicThread->start();

#define CONNECTRUNNERTOLOGIC(signalName) connect(this, &QMdmmLogicRunnerPrivate::signalName, logic, &QMdmmLogic::signalName, Qt::QueuedConnection)

    CONNECTRUNNERTOLOGIC(addPlayer);
    CONNECTRUNNERTOLOGIC(removePlayer);
    CONNECTRUNNERTOLOGIC(roundStart);
    CONNECTRUNNERTOLOGIC(sscReply);
    CONNECTRUNNERTOLOGIC(actionOrderReply);
    CONNECTRUNNERTOLOGIC(actionReply);
    CONNECTRUNNERTOLOGIC(upgradeReply);

#undef CONNECTRUNNERTOLOGIC

#define CONNECTLOGICTORUNNER(signalName) connect(logic, &QMdmmLogic::signalName, this, &QMdmmLogicRunnerPrivate::signalName, Qt::QueuedConnection)

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

void QMdmmLogicRunnerPrivate::agentStateChanged(const QMdmmData::AgentState &state)
{
    QMdmmServerAgentPrivate *changedAgent = qobject_cast<QMdmmServerAgentPrivate *>(sender());
    if (changedAgent == nullptr)
        return;

    foreach (QMdmmServerAgentPrivate *agent, agents)
        agent->notifyAgentStateChanged(changedAgent->objectName(), state);
}

void QMdmmLogicRunnerPrivate::agentSpoken(const QJsonValue &value)
{
    QMdmmServerAgentPrivate *speakAgent = qobject_cast<QMdmmServerAgentPrivate *>(sender());
    if (speakAgent == nullptr)
        return;

    QString s = value.toString();
    if (!s.isEmpty()) {
        foreach (QMdmmServerAgentPrivate *agent, agents)
            agent->notifySpoken(speakAgent->objectName(), s);
    }
}

void QMdmmLogicRunnerPrivate::agentOperated(const QJsonValue &value)
{
    QMdmmServerAgentPrivate *operateAgent = qobject_cast<QMdmmServerAgentPrivate *>(sender());
    if (operateAgent == nullptr)
        return;

    foreach (QMdmmServerAgentPrivate *agent, agents)
        agent->notifyOperated(operateAgent->objectName(), value);
}

void QMdmmLogicRunnerPrivate::socketDisconnected()
{
    QMdmmServerAgentPrivate *disconnectedAgent = qobject_cast<QMdmmServerAgentPrivate *>(sender());
    if (disconnectedAgent == nullptr)
        return;

    disconnectedAgent->socket->deleteLater();
    disconnectedAgent->socket = nullptr;

    if (p->full()) {
        // case 1: room is full, so game has started
        // Agent should exit game if round over or logic runs pass round over, which makes game over and the logic quits
        // But if client is reconnected before round over, the game should continue
        // TODO: round over

        QMdmmData::AgentState state = disconnectedAgent->state();
        state.setFlag(QMdmmData::StateMaskOnline, false).setFlag(QMdmmData::StateMaskTrust, false);
        disconnectedAgent->setState(state);

        // if all agents are disconnected, terminate the game.
        bool allDisconnected = true;
        foreach (QMdmmServerAgentPrivate *agent, agents) {
            if (agent->state().testFlag(QMdmmData::StateMaskOnline)) {
                allDisconnected = false;
                break;
            }
        }

        if (allDisconnected) {
            emit p->gameOver(QMdmmLogicRunner::QPrivateSignal());
            return;
        }

        // If there is an active request, use default reply
        // QMdmmServerAgentPrivate::executeDefaultReply handles it even if there is not active request
        disconnectedAgent->executeDefaultReply();
    } else {
        // case 2: room is not full, so game hasn't started
        // Agent should be deleted.
        QMdmmServerAgentPrivate *taken = agents.take(objectName());
        Q_UNUSED(taken);
        Q_ASSERT(taken == disconnectedAgent);

        foreach (QMdmmServerAgentPrivate *agent, agents)
            agent->notifyPlayerRemoved(disconnectedAgent->objectName());
        emit removePlayer(disconnectedAgent->objectName());

        disconnectedAgent->deleteLater();
    }
}

QMdmmLogicRunnerPrivate::~QMdmmLogicRunnerPrivate() = default;

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmLogicRunnerPrivate::requestSscForAction(const QStringList &playerNames)
{
    foreach (const QString &playerName, playerNames) {
        QMdmmServerAgentPrivate *agent = agents.value(playerName);
        agent->requestStoneScissorsCloth(playerNames, 0);
    }
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmLogicRunnerPrivate::sscResult(const QHash<QString, QMdmmData::StoneScissorsCloth> &replies)
{
    foreach (QMdmmServerAgentPrivate *agent, agents)
        agent->notifyStoneScissorsCloth(replies);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmLogicRunnerPrivate::requestActionOrder(const QString &playerName, const QList<int> &availableOrders, int maximumOrderNum, int selections)
{
    QMdmmServerAgentPrivate *agent = agents.value(playerName);
    agent->requestActionOrder(availableOrders, maximumOrderNum, selections);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmLogicRunnerPrivate::actionOrderResult(const QHash<int, QString> &result)
{
    foreach (QMdmmServerAgentPrivate *agent, agents)
        agent->notifyActionOrder(result);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmLogicRunnerPrivate::requestSscForActionOrder(const QStringList &playerNames, int strivedOrder)
{
    foreach (const QString &playerName, playerNames) {
        QMdmmServerAgentPrivate *agent = agents.value(playerName);
        agent->requestStoneScissorsCloth(playerNames, strivedOrder);
    }
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmLogicRunnerPrivate::requestAction(const QString &playerName, int actionOrder)
{
    QMdmmServerAgentPrivate *agent = agents.value(playerName);
    agent->requestAction(actionOrder);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmLogicRunnerPrivate::actionResult(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPlace)
{
    foreach (QMdmmServerAgentPrivate *agent, agents)
        agent->notifyAction(playerName, action, toPlayer, toPlace);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmLogicRunnerPrivate::requestUpgrade(const QString &playerName, int upgradePoint)
{
    QMdmmServerAgentPrivate *agent = agents.value(playerName);
    agent->requestUpgrade(upgradePoint);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmLogicRunnerPrivate::upgradeResult(const QHash<QString, QList<QMdmmData::UpgradeItem>> &upgrades)
{
    foreach (QMdmmServerAgentPrivate *agent, agents)
        agent->notifyUpgrade(upgrades);
}

QMdmmLogicRunner::QMdmmLogicRunner(const QMdmmLogicConfiguration &logicConfiguration, QObject *parent)
    : QObject(parent)
    , d(new QMdmmLogicRunnerPrivate(logicConfiguration, this))
{
}

QMdmmLogicRunner::~QMdmmLogicRunner() = default;

QMdmmAgent *QMdmmLogicRunner::addSocket(const QString &playerName, const QString &screenName, const QMdmmData::AgentState &agentState, QMdmmSocket *socket)
{
    if (d->agents.contains(playerName))
        return nullptr;

    QMdmmServerAgentPrivate *addedAgent = new QMdmmServerAgentPrivate(playerName, d);
    addedAgent->setScreenName(screenName);
    addedAgent->setState(agentState);

    addedAgent->setSocket(socket);
    d->agents.insert(playerName, addedAgent);

    connect(addedAgent, &QMdmmServerAgentPrivate::stateChanged, d, &QMdmmLogicRunnerPrivate::agentStateChanged);
    connect(addedAgent, &QMdmmServerAgentPrivate::notifySpeak, d, &QMdmmLogicRunnerPrivate::agentSpoken);
    connect(addedAgent, &QMdmmServerAgentPrivate::notifyOperate, d, &QMdmmLogicRunnerPrivate::agentOperated);

    // When a new agent is added, first we'd notify the logic configuration to client
    // This is also a signal to client that it should switch state for room data

    addedAgent->notifyLogicConfiguration();

    emit d->addPlayer(playerName);

    foreach (QMdmmServerAgentPrivate *agent, d->agents)
        agent->notifyPlayerAdded(playerName, screenName, agentState);
    foreach (QMdmmServerAgentPrivate *agent, d->agents) {
        if (agent != addedAgent)
            addedAgent->notifyPlayerAdded(playerName, screenName, agentState);
    }

    if (full()) {
        foreach (QMdmmServerAgentPrivate *agent, d->agents)
            agent->notifyGameStart();
        foreach (QMdmmServerAgentPrivate *agent, d->agents)
            agent->notifyRoundStart();
        emit d->roundStart();
    }

    return addedAgent;
}

QMdmmAgent *QMdmmLogicRunner::agent(const QString &playerName)
{
    return d->agents.value(playerName, nullptr);
}

const QMdmmAgent *QMdmmLogicRunner::agent(const QString &playerName) const
{
    return d->agents.value(playerName, nullptr);
}

bool QMdmmLogicRunner::full() const
{
    return d->agents.count() >= d->conf.playerNumPerRoom();
}
