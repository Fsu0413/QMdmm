// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmclient_p.h"
#include "qmdmmclient.h"

#include <QMdmmLogicConfiguration>
#include <QMdmmPlayer>

#include <QJsonArray>
#include <QJsonDocument>
#include <QScopeGuard>

namespace QMdmmNetworking {
namespace p {

QHash<QMdmmCore::Protocol::RequestId, void (ClientP::*)(const QJsonValue &)> ClientP::requestCallback {
    std::make_pair(QMdmmCore::Protocol::RequestStoneScissorsCloth, &ClientP::requestStoneScissorsCloth),
    std::make_pair(QMdmmCore::Protocol::RequestActionOrder, &ClientP::requestActionOrder),
    std::make_pair(QMdmmCore::Protocol::RequestAction, &ClientP::requestAction),
    std::make_pair(QMdmmCore::Protocol::RequestUpgrade, &ClientP::requestUpgrade),
};

QHash<QMdmmCore::Protocol::NotifyId, void (ClientP::*)(const QJsonValue &)> ClientP::notifyCallback {
    // from Server
    std::make_pair(QMdmmCore::Protocol::NotifyPongServer, &ClientP::notifyPongServer),
    std::make_pair(QMdmmCore::Protocol::NotifyVersion, &ClientP::notifyVersion),

    // from Agent
    std::make_pair(QMdmmCore::Protocol::NotifyLogicConfiguration, &ClientP::notifyLogicConfiguration),
    std::make_pair(QMdmmCore::Protocol::NotifyAgentStateChanged, &ClientP::notifyAgentStateChanged),
    std::make_pair(QMdmmCore::Protocol::NotifyPlayerAdded, &ClientP::notifyPlayerAdded),
    std::make_pair(QMdmmCore::Protocol::NotifyPlayerRemoved, &ClientP::notifyPlayerRemoved),
    std::make_pair(QMdmmCore::Protocol::NotifyGameStart, &ClientP::notifyGameStart),
    std::make_pair(QMdmmCore::Protocol::NotifyRoundStart, &ClientP::notifyRoundStart),
    std::make_pair(QMdmmCore::Protocol::NotifyStoneScissorsCloth, &ClientP::notifyStoneScissorsCloth),
    std::make_pair(QMdmmCore::Protocol::NotifyActionOrder, &ClientP::notifyActionOrder),
    std::make_pair(QMdmmCore::Protocol::NotifyAction, &ClientP::notifyAction),
    std::make_pair(QMdmmCore::Protocol::NotifyRoundOver, &ClientP::notifyRoundOver),
    std::make_pair(QMdmmCore::Protocol::NotifyUpgrade, &ClientP::notifyUpgrade),
    std::make_pair(QMdmmCore::Protocol::NotifyGameOver, &ClientP::notifyGameOver),
    std::make_pair(QMdmmCore::Protocol::NotifySpoken, &ClientP::notifySpoken),
    std::make_pair(QMdmmCore::Protocol::NotifyOperated, &ClientP::notifyOperated),
};

ClientP::ClientP(ClientConfiguration clientConfiguration, Client *q)
    : QObject(q)
    , q(q)
    , clientConfiguration(std::move(clientConfiguration))
    , socket(nullptr)
    , room(new QMdmmCore::Room(QMdmmCore::LogicConfiguration(), this))
    , heartbeatTimer(new QTimer(this))
    , currentRequest(QMdmmCore::Protocol::RequestInvalid)
    , initialState(QMdmmCore::Data::StateOffline)
{
    heartbeatTimer->setInterval(30000);
    heartbeatTimer->setSingleShot(false);
    connect(heartbeatTimer, &QTimer::timeout, this, &ClientP::heartbeatTimeout);
}

// TODO: replace auto with the real type
// Qt documentation only mentioned "auto" here
#define ONERRPRINTJSON(value)                                                     \
    auto onRet_ [[maybe_unused]] = qScopeGuard([this, value, func = __func__]() { \
        if (socket != nullptr)                                                    \
            socket->setHasError(true);                                            \
        QByteArray json(QJsonDocument({value}).toJson(QJsonDocument::Indented));  \
        qDebug("%s fails with Json value: %s", func, json.constData());           \
    });

// NOLINTNEXTLINE(readability-make-member-function-const)
void ClientP::requestStoneScissorsCloth(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

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
    for (const QJsonValueRef &vplayerName : vaplayerNames) {
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

    emit q->requestStoneScissorsCloth(playerNames, strivedOrder, Client::QPrivateSignal());
    onRet_.dismiss();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void ClientP::requestActionOrder(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

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
    for (const QJsonValueRef &vremainedOrder : varemainedOrders) {
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

    emit q->requestActionOrder(remainedOrders, maximumOrder, selectionNum, Client::QPrivateSignal());
    onRet_.dismiss();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void ClientP::requestAction(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

    if (!value.isDouble())
        return;
    int currentOrder = value.toInt();

    emit q->requestAction(currentOrder, Client::QPrivateSignal());
    onRet_.dismiss();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void ClientP::requestUpgrade(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

    if (!value.isDouble())
        return;
    int remainedTimes = value.toInt();

    emit q->requestUpgrade(remainedTimes, Client::QPrivateSignal());
    onRet_.dismiss();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void ClientP::notifyPongServer(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

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

    onRet_.dismiss();
}

void ClientP::notifyVersion(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

    if (!value.isObject())
        return;
    QJsonObject ob = value.toObject();

    if (!ob.contains(QStringLiteral("versionNumber")))
        return;
    QJsonValue vversionNumber = ob.value(QStringLiteral("versionNumber"));
    if (!vversionNumber.isString())
        return;
    QString versionNumber = vversionNumber.toString();

    if (!ob.contains(QStringLiteral("protocolVersion")))
        return;
    QJsonValue vprotocolVersion = ob.value(QStringLiteral("protocolVersion"));
    if (!vprotocolVersion.isDouble())
        return;
    int protocolVersion = vprotocolVersion.toInt();

    if (protocolVersion != QMdmmCore::Protocol::version())
        return;

    if (QVersionNumber::fromString(versionNumber) != QMdmmCore::Global::version()) {
        // how to deal with this?
        // Theoratically it should be compatible with each other.
        // noop for now....
    }

    // sign in process
    QJsonObject signInOb;
    signInOb.insert(QStringLiteral("playerName"), q->objectName());
    signInOb.insert(QStringLiteral("screenName"), clientConfiguration.screenName());
    signInOb.insert(QStringLiteral("agentState"), static_cast<int>(initialState));
    emit socket->sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifySignIn, signInOb));

    onRet_.dismiss();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void ClientP::notifyLogicConfiguration(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

    QMdmmCore::LogicConfiguration conf;
    if (!conf.deserialize(value))
        return;

    room->setLogicConfiguration(conf);
    onRet_.dismiss();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void ClientP::notifyAgentStateChanged(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

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
    Agent *agent = agents.value(playerName);

    QJsonValue vagentState = ob.value(QStringLiteral("agentState"));
    if (!vagentState.isDouble())
        return;
    QMdmmCore::Data::AgentState agentState = QMdmmCore::Data::AgentState(static_cast<QMdmmCore::Data::AgentState::Int>(vagentState.toInt()));

    agent->setState(agentState);
    onRet_.dismiss();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void ClientP::notifyPlayerAdded(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

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
    QMdmmCore::Data::AgentState agentState = QMdmmCore::Data::AgentState(static_cast<QMdmmCore::Data::AgentState::Int>(vagentState.toInt()));

    if (room->addPlayer(playerName) == nullptr)
        return;

    Agent *agent = new Agent(playerName, this);
    agent->setScreenName(screenName);
    agent->setState(agentState);

    emit q->notifyPlayerAdded(playerName, screenName, agentState, Client::QPrivateSignal());
    onRet_.dismiss();
}

void ClientP::notifyPlayerRemoved(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

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

    emit q->notifyPlayerRemoved(playerName, Client::QPrivateSignal());

    Agent *agent = agents.take(playerName);
    delete agent;

    onRet_.dismiss();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void ClientP::notifyGameStart(const QJsonValue &value [[maybe_unused]])
{
    emit q->notifyGameStart(Client::QPrivateSignal());
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void ClientP::notifyRoundStart(const QJsonValue &value [[maybe_unused]])
{
    emit q->notifyRoundStart(Client::QPrivateSignal());
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void ClientP::notifyStoneScissorsCloth(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

    if (!value.isObject())
        return;
    QJsonObject ob = value.toObject();

    QHash<QString, QMdmmCore::Data::StoneScissorsCloth> replies;
    for (QJsonObject::const_iterator it = ob.constBegin(); it != ob.constEnd(); ++it) {
        QString playerName = it.key();
        if (!agents.contains(playerName))
            return;
        QJsonValue vssc = it.value();
        if (!vssc.isDouble())
            return;
        QMdmmCore::Data::StoneScissorsCloth ssc = static_cast<QMdmmCore::Data::StoneScissorsCloth>(vssc.toInt());
        switch (ssc) {
        case QMdmmCore::Data::Stone:
        case QMdmmCore::Data::Scissors:
        case QMdmmCore::Data::Cloth:
            break;
        default:
            return;
        }
        replies.insert(playerName, ssc);
    }

    emit q->notifyStoneScissorsCloth(replies, Client::QPrivateSignal());
    onRet_.dismiss();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void ClientP::notifyActionOrder(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

    if (!value.isArray())
        return;
    QJsonArray arr = value.toArray();

    QHash<int, QString> result;
    int i = 0;
    for (const QJsonValueRef &val : arr) {
        if (!value.isString())
            return;
        QString playerName = val.toString();
        if (!agents.contains(playerName))
            return;
        result.insert(++i, playerName);
    }

    emit q->notifyActionOrder(result, Client::QPrivateSignal());
    onRet_.dismiss();
}

// NOLINTNEXTLINE(readability-make-member-function-const,readability-function-cognitive-complexity)
void ClientP::notifyAction(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

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
        return;
    }

    QString toPlayer;
    int toPlace = -1;

    switch (action) {
    case QMdmmCore::Data::Slash:
    case QMdmmCore::Data::Kick:
    case QMdmmCore::Data::LetMove: {
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
    case QMdmmCore::Data::Move:
    case QMdmmCore::Data::LetMove: {
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

    emit q->notifyAction(playerName, action, toPlayer, toPlace, Client::QPrivateSignal());

    // This replyed action should always be success, since it is judged in Server
    // So if it fails, forcefully set the client as error, so it can disconnect the socket
    bool success = applyAction(playerName, action, toPlayer, toPlace);
    if (success)
        onRet_.dismiss();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void ClientP::notifyRoundOver(const QJsonValue &value [[maybe_unused]])
{
    emit q->notifyRoundOver(Client::QPrivateSignal());
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void ClientP::notifyUpgrade(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

    if (!value.isObject())
        return;
    QJsonObject ob = value.toObject();

    QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> replies;
    for (QJsonObject::const_iterator it = ob.constBegin(); it != ob.constEnd(); ++it) {
        QString playerName = it.key();
        if (!agents.contains(playerName))
            return;
        QJsonValue vupgrades = it.value();
        if (!vupgrades.isArray())
            return;
        QJsonArray vaupgrades = vupgrades.toArray();
        QList<QMdmmCore::Data::UpgradeItem> upgrades;
        for (const QJsonValueRef &vupgrade : vaupgrades) {
            if (!vupgrade.isDouble())
                return;
            QMdmmCore::Data::UpgradeItem upgrade = static_cast<QMdmmCore::Data::UpgradeItem>(vupgrade.toInt());
            switch (upgrade) {
            case QMdmmCore::Data::UpgradeKnife:
            case QMdmmCore::Data::UpgradeHorse:
            case QMdmmCore::Data::UpgradeMaxHp:
                break;
            default:
                return;
            }
            upgrades << upgrade;
        }
        replies.insert(playerName, upgrades);
    }

    emit q->notifyUpgrade(replies, Client::QPrivateSignal());

    // This replyed upgrade should always be success, since it is judged in Server
    // So if it fails, forcefully set the client as error, so it can disconnect the socket
    bool success = applyUpgrade(replies);
    if (success)
        onRet_.dismiss();
}

void ClientP::notifyGameOver(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

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
    for (const QJsonValueRef &vwinner : arr) {
        if (!vwinner.isString())
            return;
        QString winner = vwinner.toString();
        if (agents.contains(winner))
            return;
        winners << winner;
    }

    emit q->notifyGameOver(winners, Client::QPrivateSignal());
    onRet_.dismiss();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void ClientP::notifySpoken(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

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
    QString content = QString::fromUtf8(QByteArray::fromBase64(vContent.toString().toLatin1()));

    emit q->notifySpoken(playerName, content, Client::QPrivateSignal());
    onRet_.dismiss();
}

void ClientP::notifyOperated(const QJsonValue &value)
{
    ONERRPRINTJSON(value);
    onRet_.dismiss();
    Q_UNIMPLEMENTED();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
bool ClientP::applyAction(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace)
{
    QMdmmCore::Player *from = room->player(playerName);
    switch (action) {
    case QMdmmCore::Data::DoNothing: {
        return from->doNothing();
    }
    case QMdmmCore::Data::BuyKnife: {
        return from->buyKnife();
    }
    case QMdmmCore::Data::BuyHorse: {
        return from->buyHorse();
    }
    case QMdmmCore::Data::Slash: {
        QMdmmCore::Player *to = room->player(toPlayer);
        return from->slash(to);
    }
    case QMdmmCore::Data::Kick: {
        QMdmmCore::Player *to = room->player(toPlayer);
        return from->kick(to);
    }
    case QMdmmCore::Data::Move: {
        return from->move(toPlace);
    }
    case QMdmmCore::Data::LetMove: {
        QMdmmCore::Player *to = room->player(toPlayer);
        return from->letMove(to, toPlace);
    }
    default:
        break;
    }

    return false;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
bool ClientP::applyUpgrade(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades)
{
    bool ret = true;

    for (QHash<QString, QList<QMdmmCore::Data::UpgradeItem>>::const_iterator it = upgrades.constBegin(); it != upgrades.constEnd(); ++it) {
        QMdmmCore::Player *up = room->player(it.key());
        const QList<QMdmmCore::Data::UpgradeItem> &items = it.value();
        foreach (QMdmmCore::Data::UpgradeItem item, items) {
            bool success = false;
            switch (item) {
            case QMdmmCore::Data::UpgradeKnife:
                success = up->upgradeKnife();
                break;
            case QMdmmCore::Data::UpgradeHorse:
                success = up->upgradeHorse();
                break;
            case QMdmmCore::Data::UpgradeMaxHp:
                success = up->upgradeMaxHp();
                break;
            default:
                break;
            }
            ret = ret && success;
        }
    }

    return ret;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void ClientP::socketPacketReceived(const QMdmmCore::Packet &packet)
{
    if (socket == nullptr)
        return;

    if (packet.type() == QMdmmCore::Protocol::TypeRequest) {
        currentRequest = packet.requestId();
        void (ClientP::*call)(const QJsonValue &) = requestCallback.value(packet.requestId(), nullptr);
        if (call != nullptr)
            (this->*call)(packet.value());
        else
            socket->setHasError(true);
    } else if (packet.type() == QMdmmCore::Protocol::TypeNotify) {
        if (((packet.notifyId() & QMdmmCore::Protocol::NotifyFromServerMask) != 0) || ((packet.notifyId() & QMdmmCore::Protocol::NotifyFromAgentMask) != 0)) {
            void (ClientP::*call)(const QJsonValue &) = notifyCallback.value(packet.notifyId(), nullptr);
            if (call != nullptr)
                (this->*call)(packet.value());
            else
                socket->setHasError(true);
        }
    }
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void ClientP::socketErrorOccurred(const QString &errorString)
{
    // TODO: redirect this error string out of client
    socket->disconnect(this);
    emit q->socketErrorDisconnected(errorString, Client::QPrivateSignal());
    socket->deleteLater();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void ClientP::socketDisconnected()
{
    socket->disconnect(this);
    emit q->socketErrorDisconnected(QStringLiteral("Disconnected"), Client::QPrivateSignal());
    socket->deleteLater();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void ClientP::heartbeatTimeout()
{
    if (socket != nullptr)
        emit socket->sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyPingServer, QDateTime::currentMSecsSinceEpoch()));
}

} // namespace p
} // namespace QMdmmNetworking
