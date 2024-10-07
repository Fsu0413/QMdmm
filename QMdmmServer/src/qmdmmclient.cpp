// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmclient.h"
#include "qmdmmclient_p.h"

#include <QMdmmRoom>

#include <QDateTime>

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
{
    heartbeatTimer->setSingleShot(false);
    heartbeatTimer->setInterval(30000);
    connect(heartbeatTimer, &QTimer::timeout, this, &QMdmmClientPrivate::heartbeatTimeout);
}

void QMdmmClientPrivate::requestStoneScissorsCloth(const QJsonValue &value)
{
}

void QMdmmClientPrivate::requestActionOrder(const QJsonValue &value)
{
}

void QMdmmClientPrivate::requestAction(const QJsonValue &value)
{
}

void QMdmmClientPrivate::requestUpgrade(const QJsonValue &value)
{
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

    QMdmmAgent *agent = agents.take(playerName);
    delete agent;
}

void QMdmmClientPrivate::notifyGameStart(const QJsonValue &value)
{
    Q_UNUSED(value);
}

void QMdmmClientPrivate::notifyRoundStart(const QJsonValue &value)
{
    Q_UNUSED(value);
}

void QMdmmClientPrivate::notifyStoneScissorsCloth(const QJsonValue &value)
{
    Q_UNUSED(value);
}

void QMdmmClientPrivate::notifyActionOrder(const QJsonValue &value)
{
    Q_UNUSED(value);
}

void QMdmmClientPrivate::notifyAction(const QJsonValue &value)
{
    Q_UNUSED(value);
}

void QMdmmClientPrivate::notifyRoundOver(const QJsonValue &value)
{
    Q_UNUSED(value);
}

void QMdmmClientPrivate::notifyUpgrade(const QJsonValue &value)
{
    Q_UNUSED(value);
}

void QMdmmClientPrivate::notifyGameOver(const QJsonValue &value)
{
    if (socket != nullptr) {
        heartbeatTimer->stop();
        disconnect(socket);
        socket->disconnect(this);

        socket->deleteLater();
    }

    Q_UNUSED(value);
}

void QMdmmClientPrivate::notifySpoken(const QJsonValue &value)
{
    Q_UNUSED(value);
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
