// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmclient.h"
#include "qmdmmclient_p.h"

#include <QMdmmRoom>

#include <QDateTime>
#include <QJsonArray>

QHash<QMdmmProtocol::RequestId, void (QMdmmClientPrivate::*)(const QJsonValue &)> QMdmmClientPrivate::requestCallback {
    std::make_pair(QMdmmProtocol::RequestStoneScissorsCloth, &QMdmmClientPrivate::requestStoneScissorsCloth),
    std::make_pair(QMdmmProtocol::RequestActionOrder, &QMdmmClientPrivate::requestActionOrder),
    std::make_pair(QMdmmProtocol::RequestAction, &QMdmmClientPrivate::requestAction),
    std::make_pair(QMdmmProtocol::RequestUpgrade, &QMdmmClientPrivate::requestUpgrade),
};

QHash<QMdmmProtocol::NotifyId, void (QMdmmClientPrivate::*)(const QJsonValue &)> QMdmmClientPrivate::notifyCallback {
    std::make_pair(QMdmmProtocol::NotifyPongServer, &QMdmmClientPrivate::notifyPongServer),
    std::make_pair(QMdmmProtocol::NotifyVersion, &QMdmmClientPrivate::notifyVersion),
    std::make_pair(QMdmmProtocol::NotifyLogicConfiguration, &QMdmmClientPrivate::notifyLogicConfiguration),
    std::make_pair(QMdmmProtocol::NotifyAgentStateChanged, &QMdmmClientPrivate::notifyAgentStateChanged),
    std::make_pair(QMdmmProtocol::NotifyPlayerAdded, &QMdmmClientPrivate::notifyPlayerAdded),
    std::make_pair(QMdmmProtocol::NotifyPlayerRemoved, &QMdmmClientPrivate::notifyPlayerRemoved),
    std::make_pair(QMdmmProtocol::NotifyGameStart, &QMdmmClientPrivate::notifyGameStart),
    std::make_pair(QMdmmProtocol::NotifyRoundStart, &QMdmmClientPrivate::notifyRoundStart),
    std::make_pair(QMdmmProtocol::NotifyStoneScissorsCloth, &QMdmmClientPrivate::notifyStoneScissorsCloth),
    std::make_pair(QMdmmProtocol::NotifyActionOrder, &QMdmmClientPrivate::notifyActionOrder),
    std::make_pair(QMdmmProtocol::NotifyAction, &QMdmmClientPrivate::notifyAction),
    std::make_pair(QMdmmProtocol::NotifyRoundOver, &QMdmmClientPrivate::notifyRoundOver),
    std::make_pair(QMdmmProtocol::NotifyUpgrade, &QMdmmClientPrivate::notifyUpgrade),
    std::make_pair(QMdmmProtocol::NotifyGameOver, &QMdmmClientPrivate::notifyGameOver),
    std::make_pair(QMdmmProtocol::NotifySpoken, &QMdmmClientPrivate::notifySpoken),
    std::make_pair(QMdmmProtocol::NotifyOperated, &QMdmmClientPrivate::notifyOperated),
};

QMdmmClientPrivate::QMdmmClientPrivate(QMdmmClient *p)
    : QObject(p)
    , p(p)
    , socket(nullptr)
    , room(new QMdmmRoom(QMdmmLogicConfiguration(), this))
    , heartbeatTimer(new QTimer(this))
    , currentRequest(QMdmmProtocol::RequestInvalid)
{
    heartbeatTimer->setInterval(30000);
    heartbeatTimer->setSingleShot(false);
    connect(heartbeatTimer, &QTimer::timeout, this, &QMdmmClientPrivate::heartbeatTimeout);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::requestStoneScissorsCloth(const QJsonValue &value)
{
    if (!value.isObject())
        return;
    QJsonObject ob = value.toObject();

    if (!ob.contains(QStringLiteral("playerNames")))
        return;
    QJsonValue vplayerNames = ob.value(QStringLiteral("playerNames"));
    if (!vplayerNames.isArray())
        return;
    QJsonArray vaplayerNames = vplayerNames.toArray();
    QStringList playerNames;
    for (const QJsonValue &vplayerName : vaplayerNames) {
        if (!vplayerName.isString())
            return;
        playerNames << vplayerName.toString();
    }

    if (!ob.contains(QStringLiteral("strivedOrder")))
        return;
    QJsonValue vstrivedOrder = ob.value(QStringLiteral("strivedOrder"));
    if (!vstrivedOrder.isDouble())
        return;
    int strivedOrder = vstrivedOrder.toInt();

    emit p->requestStoneScissorsCloth(playerNames, strivedOrder);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::requestActionOrder(const QJsonValue &value)
{
    if (!value.isObject())
        return;
    QJsonObject ob = value.toObject();

    if (!ob.contains(QStringLiteral("remainedOrders")))
        return;
    QJsonValue vremainedOrders = ob.value(QStringLiteral("remainedOrders"));
    if (!vremainedOrders.isArray())
        return;
    QJsonArray varemainedOrders = vremainedOrders.toArray();
    QList<int> remainedOrders;
    for (const QJsonValue &vremainedOrder : varemainedOrders) {
        if (!vremainedOrder.isDouble())
            return;
        remainedOrders << vremainedOrder.toInt();
    }

    if (!ob.contains(QStringLiteral("maximumOrder")))
        return;
    QJsonValue vmaximumOrder = ob.value(QStringLiteral("maximumOrder"));
    if (!vmaximumOrder.isDouble())
        return;
    int maximumOrder = vmaximumOrder.toInt();

    if (!ob.contains(QStringLiteral("selectionNum")))
        return;
    QJsonValue vselectionNum = ob.value(QStringLiteral("selectionNum"));
    if (!vselectionNum.isDouble())
        return;
    int selectionNum = vselectionNum.toInt();

    emit p->requestActionOrder(remainedOrders, maximumOrder, selectionNum);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::requestAction(const QJsonValue &value)
{
    if (!value.isDouble())
        return;
    int currentOrder = value.toInt();

    emit p->requestAction(currentOrder);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::requestUpgrade(const QJsonValue &value)
{
    if (!value.isDouble())
        return;
    int remainedTimes = value.toInt();

    emit p->requestUpgrade(remainedTimes);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyPongServer(const QJsonValue &value)
{
    bool ok = false;
    int64_t pongTime = value.toVariant().toLongLong(&ok);
    if (!ok) {
        socket->setHasError(true);
        return;
    }

    int64_t currentTime = QDateTime::currentMSecsSinceEpoch();
    int64_t elapsed = currentTime - pongTime;

    // broadcast? or recorded internally?
    Q_UNUSED(elapsed);
}

void QMdmmClientPrivate::notifyVersion(const QJsonValue &value)
{
    // sign in process
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyLogicConfiguration(const QJsonValue &value)
{
    QMdmmLogicConfiguration conf;
    if (conf.deserialize(value))
        room->setLogicConfiguration(conf);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyAgentStateChanged(const QJsonValue &value)
{
    if (!value.isObject())
        return;

    QJsonObject ob = value.toObject();
    if (!ob.contains(QStringLiteral("playerName")))
        return;
    if (!ob.contains(QStringLiteral("agentState")))
        return;

    QJsonValue vplayerName = ob.value(QStringLiteral("playerName"));
    if (!vplayerName.isString())
        return;
    QString playerName = vplayerName.toString();

    if (!agents.contains(playerName))
        return;
    QMdmmAgent *agent = agents.value(playerName);

    QJsonValue vagentState = ob.value(QStringLiteral("agentState"));
    if (!vagentState.isDouble())
        return;
    QMdmmData::AgentState agentState = QMdmmData::AgentState(static_cast<QMdmmData::AgentState::Int>(vagentState.toInt()));

    agent->setState(agentState);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyPlayerAdded(const QJsonValue &value)
{
    if (!value.isObject())
        return;

    QJsonObject ob = value.toObject();
    if (!ob.contains(QStringLiteral("playerName")))
        return;
    if (!ob.contains(QStringLiteral("screenName")))
        return;
    if (!ob.contains(QStringLiteral("agentState")))
        return;

    QJsonValue vplayerName = ob.value(QStringLiteral("playerName"));
    if (!vplayerName.isString())
        return;
    QString playerName = vplayerName.toString();
    if (agents.contains(playerName))
        return;

    QJsonValue vscreenName = ob.value(QStringLiteral("screenName"));
    if (!vscreenName.isString())
        return;
    QString screenName = vscreenName.toString();

    QJsonValue vagentState = ob.value(QStringLiteral("agentState"));
    if (!vagentState.isDouble())
        return;
    QMdmmData::AgentState agentState = QMdmmData::AgentState(static_cast<QMdmmData::AgentState::Int>(vagentState.toInt()));

    if (room->addPlayer(playerName) == nullptr)
        return;

    QMdmmAgent *agent = new QMdmmAgent(playerName, this);
    agent->setScreenName(screenName);
    agent->setState(agentState);

    emit p->notifyPlayerAdded(playerName, screenName, agentState);
}

void QMdmmClientPrivate::notifyPlayerRemoved(const QJsonValue &value)
{
    if (!value.isObject())
        return;
    QJsonObject ob = value.toObject();

    if (!ob.contains(QStringLiteral("playerName")))
        return;
    QJsonValue vplayerName = ob.value(QStringLiteral("playerName"));
    if (!vplayerName.isString())
        return;
    QString playerName = vplayerName.toString();
    if (!agents.contains(playerName))
        return;

    if (!room->removePlayer(playerName))
        return;

    emit p->notifyPlayerRemoved(playerName);

    QMdmmAgent *agent = agents.take(playerName);
    delete agent;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyGameStart(const QJsonValue &value)
{
    Q_UNUSED(value);
    emit p->notifyGameStart();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyRoundStart(const QJsonValue &value)
{
    Q_UNUSED(value);
    emit p->notifyRoundStart();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyStoneScissorsCloth(const QJsonValue &value)
{
    if (!value.isObject())
        return;
    QJsonObject ob = value.toObject();

    QHash<QString, QMdmmData::StoneScissorsCloth> replies;
    for (QJsonObject::const_iterator it = ob.constBegin(); it != ob.constEnd(); ++it) {
        QString playerName = it.key();
        if (!agents.contains(playerName))
            return;
        QJsonValue vssc = it.value();
        if (!vssc.isDouble())
            return;
        QMdmmData::StoneScissorsCloth ssc = static_cast<QMdmmData::StoneScissorsCloth>(vssc.toInt());
        switch (ssc) {
        case QMdmmData::Stone:
        case QMdmmData::Scissors:
        case QMdmmData::Cloth:
            break;
        default:
            return;
        }
        replies.insert(playerName, ssc);
    }

    emit p->notifyStoneScissorsCloth(replies);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyActionOrder(const QJsonValue &value)
{
    if (!value.isArray())
        return;
    QJsonArray arr = value.toArray();

    QHash<int, QString> result;
    int i = 0;
    for (const QJsonValue &val : arr) {
        if (!value.isString())
            return;
        QString playerName = val.toString();
        if (!agents.contains(playerName))
            return;
        result.insert(++i, playerName);
    }

    emit p->notifyActionOrder(result);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyAction(const QJsonValue &value)
{
    if (!value.isObject())
        return;
    QJsonObject ob = value.toObject();

    if (!ob.contains(QStringLiteral("playerName")))
        return;
    QJsonValue vplayerName = ob.value(QStringLiteral("playerName"));
    if (!vplayerName.isString())
        return;
    QString playerName = vplayerName.toString();
    if (!agents.contains(playerName))
        return;

    if (!ob.contains(QStringLiteral("action")))
        return;
    QJsonValue vaction = ob.value(QStringLiteral("action"));
    if (!vaction.isDouble())
        return;
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
        return;
    }

    QString toPlayer;
    int toPlace = -1;

    switch (action) {
    case QMdmmData::Slash:
    case QMdmmData::Kick:
    case QMdmmData::LetMove: {
        if (!ob.contains(QStringLiteral("toPlayer")))
            return;
        QJsonValue vtoPlayer = ob.value(QStringLiteral("toPlayer"));
        if (!vtoPlayer.isString())
            return;
        toPlayer = vtoPlayer.toString();
        if (!agents.contains(toPlayer))
            return;

        break;
    }
    default:
        break;
    }

    switch (action) {
    case QMdmmData::Move:
    case QMdmmData::LetMove: {
        if (!ob.contains(QStringLiteral("toPlace")))
            return;
        QJsonValue vtoPlace = ob.value(QStringLiteral("toPlace"));
        if (!vtoPlace.isDouble())
            return;
        toPlace = vtoPlace.toInt();
        if ((toPlace < 0) || (toPlace > agents.count()))
            return;

        break;
    }
    default:
        break;
    }

    emit p->notifyAction(playerName, action, toPlayer, toPlace);

    // applyAction();
    Q_UNIMPLEMENTED();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyRoundOver(const QJsonValue &value)
{
    Q_UNUSED(value);
    emit p->notifyRoundOver();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyUpgrade(const QJsonValue &value)
{
    if (!value.isObject())
        return;
    QJsonObject ob = value.toObject();

    QHash<QString, QList<QMdmmData::UpgradeItem>> replies;
    for (QJsonObject::const_iterator it = ob.constBegin(); it != ob.constEnd(); ++it) {
        QString playerName = it.key();
        if (!agents.contains(playerName))
            return;
        QJsonValue vupgrades = it.value();
        if (!vupgrades.isArray())
            return;
        QJsonArray vaupgrades = vupgrades.toArray();
        QList<QMdmmData::UpgradeItem> upgrades;
        for (const QJsonValue &vupgrade : vaupgrades) {
            if (!vupgrade.isDouble())
                return;
            QMdmmData::UpgradeItem upgrade = static_cast<QMdmmData::UpgradeItem>(vupgrade.toInt());
            switch (upgrade) {
            case QMdmmData::UpgradeKnife:
            case QMdmmData::UpgradeHorse:
            case QMdmmData::UpgradeMaxHp:
                break;
            default:
                return;
            }
            upgrades << upgrade;
        }
        replies.insert(playerName, upgrades);
    }

    emit p->notifyUpgrade(replies);

    // applyUpgrade();
    Q_UNIMPLEMENTED();
}

void QMdmmClientPrivate::notifyGameOver(const QJsonValue &value)
{
    // Game is over, all other requests / replies are not processed anymore.
    // No longer receives anything from server and disconnects socket here seems reasonable.
    // All other things (agents, room, players inside room) can be cleaned up during Client instance destruction

    if (socket != nullptr) {
        heartbeatTimer->stop();
        disconnect(socket);
        socket->disconnect(this);

        socket->deleteLater();
    }

    if (!value.isArray())
        return;
    QJsonArray arr = value.toArray();

    QStringList winners;
    for (const QJsonValue &vwinner : arr) {
        if (!vwinner.isString())
            return;
        QString winner = vwinner.toString();
        if (agents.contains(winner))
            return;
        winners << winner;
    }

    emit p->notifyGameOver(winners);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifySpoken(const QJsonValue &value)
{
    if (!value.isObject())
        return;
    QJsonObject ob = value.toObject();

    if (!ob.contains(QStringLiteral("playerName")))
        return;
    QJsonValue vplayerName = ob.value(QStringLiteral("playerName"));
    if (!vplayerName.isString())
        return;
    QString playerName = vplayerName.toString();
    if (!agents.contains(playerName))
        return;

    if (!ob.contains(QStringLiteral("content")))
        return;
    QJsonValue vContent = ob.value(QStringLiteral("content"));
    if (!vContent.isString())
        return;
    QString content = vContent.toString();

    emit p->notifySpoken(playerName, content);
}

void QMdmmClientPrivate::notifyOperated(const QJsonValue &value)
{
    Q_UNIMPLEMENTED();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::heartbeatTimeout()
{
    if (socket != nullptr)
        emit socket->sendPacket(QMdmmPacket(QMdmmProtocol::NotifyPingServer, QDateTime::currentMSecsSinceEpoch()));
}

QMdmmClient::QMdmmClient(QObject *parent)
    : QObject(parent)
    , d(new QMdmmClientPrivate(this))
{
}

QMdmmClient::~QMdmmClient() = default;

void QMdmmClient::requestTimeout()
{
    // process default reply stuff
}
