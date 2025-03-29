// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmlogicrunner.h"
#include "qmdmmlogicrunner_p.h"

#include <QJsonArray>
#include <QMetaType>
#include <QRandomGenerator>
#include <utility>

QHash<QMdmmCore::Protocol::NotifyId, void (QMdmmServerAgentPrivate::*)(const QJsonValue &)> QMdmmServerAgentPrivate::notifyCallback {
    std::make_pair(QMdmmCore::Protocol::NotifySpeak, &QMdmmServerAgentPrivate::notifySpeak),
    std::make_pair(QMdmmCore::Protocol::NotifyOperate, &QMdmmServerAgentPrivate::notifyOperate),
};

QHash<QMdmmCore::Protocol::RequestId, void (QMdmmServerAgentPrivate::*)(const QJsonValue &)> QMdmmServerAgentPrivate::replyCallback {
    std::make_pair(QMdmmCore::Protocol::RequestStoneScissorsCloth, &QMdmmServerAgentPrivate::replyStoneScissorsCloth),
    std::make_pair(QMdmmCore::Protocol::RequestActionOrder, &QMdmmServerAgentPrivate::replyActionOrder),
    std::make_pair(QMdmmCore::Protocol::RequestAction, &QMdmmServerAgentPrivate::replyAction),
    std::make_pair(QMdmmCore::Protocol::RequestUpgrade, &QMdmmServerAgentPrivate::replyUpgrade),
};

QHash<QMdmmCore::Protocol::RequestId, void (QMdmmServerAgentPrivate::*)()> QMdmmServerAgentPrivate::defaultReplyCallback {
    std::make_pair(QMdmmCore::Protocol::RequestStoneScissorsCloth, &QMdmmServerAgentPrivate::defaultReplyStoneScissorsCloth),
    std::make_pair(QMdmmCore::Protocol::RequestActionOrder, &QMdmmServerAgentPrivate::defaultReplyActionOrder),
    std::make_pair(QMdmmCore::Protocol::RequestAction, &QMdmmServerAgentPrivate::defaultReplyAction),
    std::make_pair(QMdmmCore::Protocol::RequestUpgrade, &QMdmmServerAgentPrivate::defaultReplyUpgrade),
};

int QMdmmServerAgentPrivate::requestTimeoutGracePeriod = 60;

QMdmmServerAgentPrivate::QMdmmServerAgentPrivate(const QString &name, QMdmmLogicRunnerPrivate *parent)
    : QMdmmAgent(name, parent)
    , p(parent)
    , currentRequest(QMdmmCore::Protocol::RequestInvalid)
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

void QMdmmServerAgentPrivate::addRequest(QMdmmCore::Protocol::RequestId requestId, const QJsonValue &value)
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

void QMdmmServerAgentPrivate::defaultReplyStoneScissorsCloth()
{
    emit p->sscReply(objectName(), static_cast<QMdmmCore::Data::StoneScissorsCloth>(QRandomGenerator::global()->generate() % 3));
}

void QMdmmServerAgentPrivate::defaultReplyActionOrder()
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

void QMdmmServerAgentPrivate::defaultReplyAction()
{
    emit p->actionReply(objectName(), QMdmmCore::Data::DoNothing, {}, 0);
}

void QMdmmServerAgentPrivate::defaultReplyUpgrade()
{
    int times = currentRequestValue.toInt(1);
    QList<QMdmmCore::Data::UpgradeItem> ups;
    ups.reserve(times);
    while ((times--) != 0)
        ups << QMdmmCore::Data::UpgradeMaxHp;
    emit p->upgradeReply(objectName(), ups);
}

void QMdmmServerAgentPrivate::packetReceived(const QMdmmCore::Packet &packet)
{
    if (socket == nullptr)
        return;

    if (packet.type() == QMdmmCore::Protocol::TypeNotify) {
        if ((packet.notifyId() & QMdmmCore::Protocol::NotifyToAgentMask) != 0) {
            void (QMdmmServerAgentPrivate::*call)(const QJsonValue &) = notifyCallback.value(packet.notifyId(), nullptr);
            if (call != nullptr)
                (this->*call)(packet.value());
            else
                socket->setHasError(true);
        }
    } else if (packet.type() == QMdmmCore::Protocol::TypeReply) {
        if (currentRequest == packet.requestId()) {
            requestTimer->stop();
            currentRequest = QMdmmCore::Protocol::RequestInvalid;
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
    addRequest(QMdmmCore::Protocol::RequestStoneScissorsCloth, ob);
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
    addRequest(QMdmmCore::Protocol::RequestActionOrder, ob);
}

void QMdmmServerAgentPrivate::requestAction(int currentOrder)
{
    addRequest(QMdmmCore::Protocol::RequestAction, currentOrder);
}

void QMdmmServerAgentPrivate::requestUpgrade(int remainingTimes)
{
    addRequest(QMdmmCore::Protocol::RequestUpgrade, remainingTimes);
}

void QMdmmServerAgentPrivate::notifyLogicConfiguration()
{
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyLogicConfiguration, p->conf));
}

void QMdmmServerAgentPrivate::notifyAgentStateChanged(const QString &playerName, const QMdmmCore::Data::AgentState &agentState)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerName"), playerName);
    ob.insert(QStringLiteral("agentState"), static_cast<int>(QMdmmCore::Data::AgentState::Int(agentState)));
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyAgentStateChanged, ob));
}

void QMdmmServerAgentPrivate::notifyPlayerAdded(const QString &playerName, const QString &screenName, const QMdmmCore::Data::AgentState &agentState)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerName"), playerName);
    ob.insert(QStringLiteral("screenName"), screenName);
    ob.insert(QStringLiteral("agentState"), static_cast<int>(QMdmmCore::Data::AgentState::Int(agentState)));
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyPlayerAdded, ob));
}

void QMdmmServerAgentPrivate::notifyPlayerRemoved(const QString &playerName)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerName"), playerName);
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyPlayerRemoved, ob));
}

void QMdmmServerAgentPrivate::notifyGameStart()
{
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyGameStart, {}));
}

void QMdmmServerAgentPrivate::notifyRoundStart()
{
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyRoundStart, {}));
}

void QMdmmServerAgentPrivate::notifyStoneScissorsCloth(const QHash<QString, QMdmmCore::Data::StoneScissorsCloth> &replies)
{
    QJsonObject ob;
    for (QHash<QString, QMdmmCore::Data::StoneScissorsCloth>::const_iterator it = replies.constBegin(); it != replies.constEnd(); ++it)
        ob.insert(it.key(), static_cast<int>(it.value()));
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyStoneScissorsCloth, ob));
}

void QMdmmServerAgentPrivate::notifyActionOrder(const QHash<int, QString> &result)
{
    QJsonArray arr;
    for (int i = 1; i < result.count(); ++i)
        arr.append(result.value(i));
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyActionOrder, arr));
}

void QMdmmServerAgentPrivate::notifyAction(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace)
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

void QMdmmServerAgentPrivate::notifyRoundOver()
{
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyRoundOver, {}));
}

void QMdmmServerAgentPrivate::notifyUpgrade(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades)
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

void QMdmmServerAgentPrivate::notifyGameOver(const QStringList &playerNames)
{
    emit sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyGameOver, QJsonArray::fromStringList(playerNames)));
}

void QMdmmServerAgentPrivate::notifySpoken(const QString &playerName, const QString &content)
{
    QJsonObject ob;
    ob.insert(QStringLiteral("playerName"), playerName);
    ob.insert(QStringLiteral("content"), content);
    QMdmmCore::Packet p(QMdmmCore::Protocol::NotifySpoken, ob);
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
    if (currentRequest != QMdmmCore::Protocol::RequestInvalid) {
        void (QMdmmServerAgentPrivate::*call)() = defaultReplyCallback.value(currentRequest, nullptr);
        currentRequest = QMdmmCore::Protocol::RequestInvalid;
        if (call != nullptr)
            (this->*call)();
    }
}

QMdmmLogicRunnerPrivate::QMdmmLogicRunnerPrivate(QMdmmCore::LogicConfiguration logicConfiguration, QMdmmLogicRunner *q)
    : QObject(q)
    , q(q)
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

void QMdmmLogicRunnerPrivate::agentStateChanged(const QMdmmCore::Data::AgentState &state)
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
        foreach (QMdmmServerAgentPrivate *agent, agents) {
            if (agent->state().testFlag(QMdmmCore::Data::StateMaskOnline)) {
                allDisconnected = false;
                break;
            }
        }

        if (allDisconnected) {
            emit q->gameOver(QMdmmLogicRunner::QPrivateSignal());
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
void QMdmmLogicRunnerPrivate::sscResult(const QHash<QString, QMdmmCore::Data::StoneScissorsCloth> &replies)
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
void QMdmmLogicRunnerPrivate::actionResult(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace)
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
void QMdmmLogicRunnerPrivate::upgradeResult(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades)
{
    foreach (QMdmmServerAgentPrivate *agent, agents)
        agent->notifyUpgrade(upgrades);
}

QMdmmLogicRunner::QMdmmLogicRunner(const QMdmmCore::LogicConfiguration &logicConfiguration, QObject *parent)
    : QObject(parent)
    , d(new QMdmmLogicRunnerPrivate(logicConfiguration, this))
{
}

QMdmmLogicRunner::~QMdmmLogicRunner() = default;

QMdmmAgent *QMdmmLogicRunner::addSocket(const QString &playerName, const QString &screenName, const QMdmmCore::Data::AgentState &agentState, QMdmmSocket *socket)
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
