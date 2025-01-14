// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmclient.h"
#include "qmdmmclient_p.h"

#include <QMdmmPlayer>
#include <QMdmmRoom>

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QScopeGuard>

#include <random>

const QMdmmClientConfiguration &QMdmmClientConfiguration::defaults()
{
    // clang-format off
    static const QMdmmClientConfiguration defaultInstance {
        std::make_pair(QStringLiteral("screenName"), QStringLiteral("QMdmm-Fans")),
    };
    // clang-format on

    return defaultInstance;
}

#define CONVERTTOTYPEQSTRING(v) v.toString()
#define IMPLEMENTATION_CONFIGURATION(type, valueName, ValueName, convertToType, convertToQVariant) \
    type QMdmmClientConfiguration::valueName() const                                               \
    {                                                                                              \
        if (contains(QStringLiteral(#valueName)))                                                  \
            return convertToType(value(QStringLiteral(#valueName)));                               \
        return convertToType(defaults().value(QStringLiteral(#valueName)));                        \
    }                                                                                              \
    void QMdmmClientConfiguration::set##ValueName(type value)                                      \
    {                                                                                              \
        insert(QStringLiteral(#valueName), convertToQVariant(value));                              \
    }

#define IMPLEMENTATION_CONFIGURATION2(type, valueName, ValueName, convertToType, convertToQVariant) \
    type QMdmmClientConfiguration::valueName() const                                                \
    {                                                                                               \
        if (contains(QStringLiteral(#valueName)))                                                   \
            return convertToType(value(QStringLiteral(#valueName)));                                \
        return convertToType(defaults().value(QStringLiteral(#valueName)));                         \
    }                                                                                               \
    void QMdmmClientConfiguration::set##ValueName(const type &value)                                \
    {                                                                                               \
        insert(QStringLiteral(#valueName), convertToQVariant(value));                               \
    }

IMPLEMENTATION_CONFIGURATION2(QString, screenName, ScreenName, CONVERTTOTYPEQSTRING, )

#undef IMPLEMENTATION_CONFIGURATION2
#undef IMPLEMENTATION_CONFIGURATION
#undef CONVERTTOTYPEQSTRING

QHash<QMdmmProtocol::RequestId, void (QMdmmClientPrivate::*)(const QJsonValue &)> QMdmmClientPrivate::requestCallback {
    std::make_pair(QMdmmProtocol::RequestStoneScissorsCloth, &QMdmmClientPrivate::requestStoneScissorsCloth),
    std::make_pair(QMdmmProtocol::RequestActionOrder, &QMdmmClientPrivate::requestActionOrder),
    std::make_pair(QMdmmProtocol::RequestAction, &QMdmmClientPrivate::requestAction),
    std::make_pair(QMdmmProtocol::RequestUpgrade, &QMdmmClientPrivate::requestUpgrade),
};

QHash<QMdmmProtocol::NotifyId, void (QMdmmClientPrivate::*)(const QJsonValue &)> QMdmmClientPrivate::notifyCallback {
    // from Server
    std::make_pair(QMdmmProtocol::NotifyPongServer, &QMdmmClientPrivate::notifyPongServer),
    std::make_pair(QMdmmProtocol::NotifyVersion, &QMdmmClientPrivate::notifyVersion),

    // from Agent
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

QMdmmClientPrivate::QMdmmClientPrivate(QMdmmClientConfiguration clientConfiguration, QMdmmClient *q)
    : QObject(q)
    , q(q)
    , clientConfiguration(std::move(clientConfiguration))
    , socket(nullptr)
    , room(new QMdmmRoom(QMdmmLogicConfiguration(), this))
    , heartbeatTimer(new QTimer(this))
    , currentRequest(QMdmmProtocol::RequestInvalid)
    , initialState(QMdmmData::StateOffline)
{
    heartbeatTimer->setInterval(30000);
    heartbeatTimer->setSingleShot(false);
    connect(heartbeatTimer, &QTimer::timeout, this, &QMdmmClientPrivate::heartbeatTimeout);
}

// TODO: replace auto with the real type
// Qt documentation only mentioned "auto" here
#define ONERRPRINTJSON(value)                                                    \
    auto onRet_ = qScopeGuard([this, value, func = __func__]() {                 \
        if (socket != nullptr)                                                   \
            socket->setHasError(true);                                           \
        QByteArray json(QJsonDocument({value}).toJson(QJsonDocument::Indented)); \
        qDebug("%s fails with Json value: %s", func, json.constData());          \
    });

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::requestStoneScissorsCloth(const QJsonValue &value)
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

    onRet_.dismiss();
    emit q->requestStoneScissorsCloth(playerNames, strivedOrder, QMdmmClient::QPrivateSignal());
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::requestActionOrder(const QJsonValue &value)
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

    onRet_.dismiss();
    emit q->requestActionOrder(remainedOrders, maximumOrder, selectionNum, QMdmmClient::QPrivateSignal());
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::requestAction(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

    if (!value.isDouble())
        return;
    int currentOrder = value.toInt();

    onRet_.dismiss();
    emit q->requestAction(currentOrder, QMdmmClient::QPrivateSignal());
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::requestUpgrade(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

    if (!value.isDouble())
        return;
    int remainedTimes = value.toInt();

    onRet_.dismiss();
    emit q->requestUpgrade(remainedTimes, QMdmmClient::QPrivateSignal());
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyPongServer(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

    bool ok = false;
    int64_t pongTime = value.toVariant().toLongLong(&ok);
    if (!ok) {
        socket->setHasError(true);
        return;
    }

    onRet_.dismiss();

    int64_t currentTime = QDateTime::currentMSecsSinceEpoch();
    int64_t elapsed = currentTime - pongTime;

    // broadcast? or recorded internally?
    Q_UNUSED(elapsed);
}

void QMdmmClientPrivate::notifyVersion(const QJsonValue &value)
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

    if (protocolVersion != QMdmmProtocol::protocolVersion())
        return;

    if (QVersionNumber::fromString(versionNumber) != QMdmmGlobal::version()) {
        // how to deal with this?
        // Theoratically it should be compatible with each other.
        // noop for now....
    }

    onRet_.dismiss();

    // sign in process
    QJsonObject signInOb;
    signInOb.insert(QStringLiteral("playerName"), q->objectName());
    signInOb.insert(QStringLiteral("screenName"), clientConfiguration.screenName());
    signInOb.insert(QStringLiteral("agentState"), static_cast<int>(initialState));
    emit socket->sendPacket(QMdmmPacket(QMdmmProtocol::NotifySignIn, signInOb));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyLogicConfiguration(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

    QMdmmLogicConfiguration conf;
    if (!conf.deserialize(value))
        return;

    onRet_.dismiss();
    room->setLogicConfiguration(conf);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyAgentStateChanged(const QJsonValue &value)
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
    QMdmmAgent *agent = agents.value(playerName);

    QJsonValue vagentState = ob.value(QStringLiteral("agentState"));
    if (!vagentState.isDouble())
        return;
    QMdmmData::AgentState agentState = QMdmmData::AgentState(static_cast<QMdmmData::AgentState::Int>(vagentState.toInt()));

    onRet_.dismiss();
    agent->setState(agentState);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyPlayerAdded(const QJsonValue &value)
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
    QMdmmData::AgentState agentState = QMdmmData::AgentState(static_cast<QMdmmData::AgentState::Int>(vagentState.toInt()));

    if (room->addPlayer(playerName) == nullptr)
        return;

    onRet_.dismiss();

    QMdmmAgent *agent = new QMdmmAgent(playerName, this);
    agent->setScreenName(screenName);
    agent->setState(agentState);

    emit q->notifyPlayerAdded(playerName, screenName, agentState, QMdmmClient::QPrivateSignal());
}

void QMdmmClientPrivate::notifyPlayerRemoved(const QJsonValue &value)
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

    onRet_.dismiss();

    emit q->notifyPlayerRemoved(playerName, QMdmmClient::QPrivateSignal());

    QMdmmAgent *agent = agents.take(playerName);
    delete agent;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyGameStart(const QJsonValue &value)
{
    Q_UNUSED(value);
    emit q->notifyGameStart(QMdmmClient::QPrivateSignal());
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyRoundStart(const QJsonValue &value)
{
    Q_UNUSED(value);
    emit q->notifyRoundStart(QMdmmClient::QPrivateSignal());
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyStoneScissorsCloth(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

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

    onRet_.dismiss();
    emit q->notifyStoneScissorsCloth(replies, QMdmmClient::QPrivateSignal());
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyActionOrder(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

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

    onRet_.dismiss();
    emit q->notifyActionOrder(result, QMdmmClient::QPrivateSignal());
}

// NOLINTNEXTLINE(readability-make-member-function-const,readability-function-cognitive-complexity)
void QMdmmClientPrivate::notifyAction(const QJsonValue &value)
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

    onRet_.dismiss();

    emit q->notifyAction(playerName, action, toPlayer, toPlace, QMdmmClient::QPrivateSignal());
    applyAction(playerName, action, toPlayer, toPlace);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyRoundOver(const QJsonValue &value)
{
    Q_UNUSED(value);
    emit q->notifyRoundOver(QMdmmClient::QPrivateSignal());
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifyUpgrade(const QJsonValue &value)
{
    ONERRPRINTJSON(value);

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

    onRet_.dismiss();

    emit q->notifyUpgrade(replies, QMdmmClient::QPrivateSignal());
    applyUpgrade(replies);
}

void QMdmmClientPrivate::notifyGameOver(const QJsonValue &value)
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
    for (const QJsonValue &vwinner : arr) {
        if (!vwinner.isString())
            return;
        QString winner = vwinner.toString();
        if (agents.contains(winner))
            return;
        winners << winner;
    }

    onRet_.dismiss();

    emit q->notifyGameOver(winners, QMdmmClient::QPrivateSignal());
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::notifySpoken(const QJsonValue &value)
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

    onRet_.dismiss();

    emit q->notifySpoken(playerName, content, QMdmmClient::QPrivateSignal());
}

void QMdmmClientPrivate::notifyOperated(const QJsonValue &value)
{
    ONERRPRINTJSON(value);
    Q_UNIMPLEMENTED();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
bool QMdmmClientPrivate::applyAction(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPlace)
{
    QMdmmPlayer *from = room->player(playerName);
    switch (action) {
    case QMdmmData::DoNothing: {
        return from->doNothing();
    }
    case QMdmmData::BuyKnife: {
        return from->buyKnife();
    }
    case QMdmmData::BuyHorse: {
        return from->buyHorse();
    }
    case QMdmmData::Slash: {
        QMdmmPlayer *to = room->player(toPlayer);
        return from->slash(to);
    }
    case QMdmmData::Kick: {
        QMdmmPlayer *to = room->player(toPlayer);
        return from->kick(to);
    }
    case QMdmmData::Move: {
        return from->move(toPlace);
    }
    case QMdmmData::LetMove: {
        QMdmmPlayer *to = room->player(toPlayer);
        return from->letMove(to, toPlace);
    }
    default:
        break;
    }

    return false;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
bool QMdmmClientPrivate::applyUpgrade(const QHash<QString, QList<QMdmmData::UpgradeItem>> &upgrades)
{
    bool ret = true;

    for (QHash<QString, QList<QMdmmData::UpgradeItem>>::const_iterator it = upgrades.constBegin(); it != upgrades.constEnd(); ++it) {
        QMdmmPlayer *up = room->player(it.key());
        const QList<QMdmmData::UpgradeItem> &items = it.value();
        foreach (QMdmmData::UpgradeItem item, items) {
            bool success = false;
            switch (item) {
            case QMdmmData::UpgradeKnife:
                success = up->upgradeKnife();
                break;
            case QMdmmData::UpgradeHorse:
                success = up->upgradeHorse();
                break;
            case QMdmmData::UpgradeMaxHp:
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
void QMdmmClientPrivate::socketPacketReceived(QMdmmPacket packet)
{
    if (socket == nullptr)
        return;

    if (packet.type() == QMdmmProtocol::TypeRequest) {
        currentRequest = packet.requestId();
        void (QMdmmClientPrivate::*call)(const QJsonValue &) = requestCallback.value(packet.requestId(), nullptr);
        if (call != nullptr)
            (this->*call)(packet.value());
        else
            socket->setHasError(true);
    } else if (packet.type() == QMdmmProtocol::TypeNotify) {
        if (((packet.notifyId() & QMdmmProtocol::NotifyFromServerMask) != 0) || ((packet.notifyId() & QMdmmProtocol::NotifyFromAgentMask) != 0)) {
            void (QMdmmClientPrivate::*call)(const QJsonValue &) = notifyCallback.value(packet.notifyId(), nullptr);
            if (call != nullptr)
                (this->*call)(packet.value());
            else
                socket->setHasError(true);
        }
    }
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::socketErrorOccurred(const QString &errorString)
{
    // TODO: redirect this error string out of client
    socket->disconnect(this);
    emit q->socketErrorDisconnected(errorString, QMdmmClient::QPrivateSignal());
    socket->deleteLater();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::socketDisconnected()
{
    socket->disconnect(this);
    emit q->socketErrorDisconnected(QStringLiteral("Disconnected"), QMdmmClient::QPrivateSignal());
    socket->deleteLater();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void QMdmmClientPrivate::heartbeatTimeout()
{
    if (socket != nullptr)
        emit socket->sendPacket(QMdmmPacket(QMdmmProtocol::NotifyPingServer, QDateTime::currentMSecsSinceEpoch()));
}

namespace {
inline QString generateRandomString()
{
    std::random_device random1;
    std::mt19937 random2(random1());

    QByteArray arr;
    for (int i = 0; i < 30; ++i)
        arr.append(static_cast<char>(random2() % 255));

    return QString::fromLatin1(arr.toBase64(QByteArray::OmitTrailingEquals));
}
} // namespace

QMdmmClient::QMdmmClient(QMdmmClientConfiguration clientConfiguration, QObject *parent)
    : QObject(parent)
    , d(new QMdmmClientPrivate(std::move(clientConfiguration), this))
{
    setObjectName(generateRandomString());
}

QMdmmClient::~QMdmmClient() = default;

bool QMdmmClient::connectToHost(const QString &host, QMdmmData::AgentState initialState)
{
    if (d->socket != nullptr) {
        d->socket->disconnect(d);
        d->socket->deleteLater();
    }

    d->initialState = initialState;
    d->socket = new QMdmmSocket(d);
    connect(d->socket, &QMdmmSocket::socketDisconnected, d, &QMdmmClientPrivate::socketDisconnected);
    connect(d->socket, &QMdmmSocket::socketErrorOccurred, d, &QMdmmClientPrivate::socketErrorOccurred);
    connect(d->socket, &QMdmmSocket::packetReceived, d, &QMdmmClientPrivate::socketPacketReceived);
    return d->socket->connectToHost(host);
}

QMdmmRoom *QMdmmClient::room()
{
    return d->room;
}

const QMdmmRoom *QMdmmClient::room() const
{
    return d->room;
}

void QMdmmClient::notifySpeak(const QString &content)
{
    // Although JSON is native UTF-8 we decided to use Base64 anyway.
    // This can make our request / response all in one line.
    if (d->socket != nullptr)
        emit d->socket->sendPacket(QMdmmPacket(QMdmmProtocol::NotifySpeak, QString::fromLatin1(content.toUtf8().toBase64())));
}

void QMdmmClient::notifyOperate(const void *todo)
{
    Q_UNIMPLEMENTED();
    Q_UNUSED(todo);

    if (d->socket != nullptr)
        emit d->socket->sendPacket(QMdmmPacket(QMdmmProtocol::NotifyOperate, {}));
}

void QMdmmClient::requestTimeout()
{
    // This should be a definitely invalid reply, to trigger default reply logic implemented in server.
    if (d->socket != nullptr && d->currentRequest != QMdmmProtocol::RequestInvalid) {
        d->currentRequest = QMdmmProtocol::RequestInvalid;
        emit d->socket->sendPacket(QMdmmPacket(QMdmmProtocol::TypeReply, d->currentRequest, {}));
    }
}

void QMdmmClient::replyStoneScissorsCloth(QMdmmData::StoneScissorsCloth stoneScissorsCloth)
{
    if (d->socket != nullptr && d->currentRequest == QMdmmProtocol::RequestStoneScissorsCloth) {
        d->currentRequest = QMdmmProtocol::RequestInvalid;
        emit d->socket->sendPacket(QMdmmPacket(QMdmmProtocol::TypeReply, QMdmmProtocol::RequestStoneScissorsCloth, static_cast<int>(stoneScissorsCloth)));
    }
}

void QMdmmClient::replyActionOrder(QList<int> actionOrder)
{
    if (d->socket != nullptr && d->currentRequest == QMdmmProtocol::RequestActionOrder) {
        d->currentRequest = QMdmmProtocol::RequestInvalid;
        QJsonArray arr;
        foreach (int a, actionOrder)
            arr.append(a);

        emit d->socket->sendPacket(QMdmmPacket(QMdmmProtocol::TypeReply, QMdmmProtocol::RequestActionOrder, arr));
    }
}

void QMdmmClient::replyAction(QMdmmData::Action action, const QString &toPlayer, int toPlace)
{
    if (d->socket != nullptr && d->currentRequest == QMdmmProtocol::RequestAction) {
        d->currentRequest = QMdmmProtocol::RequestInvalid;
        QJsonObject ob;
        ob.insert(QStringLiteral("action"), static_cast<int>(action));
        ob.insert(QStringLiteral("toPlayer"), toPlayer);
        ob.insert(QStringLiteral("toPlace"), toPlace);

        emit d->socket->sendPacket(QMdmmPacket(QMdmmProtocol::TypeReply, QMdmmProtocol::RequestAction, ob));
    }
}

void QMdmmClient::replyUpgrade(QList<QMdmmData::UpgradeItem> upgrades)
{
    if (d->socket != nullptr && d->currentRequest == QMdmmProtocol::RequestUpgrade) {
        d->currentRequest = QMdmmProtocol::RequestInvalid;
        QJsonArray arr;
        foreach (QMdmmData::UpgradeItem it, upgrades)
            arr.append(static_cast<int>(it));

        emit d->socket->sendPacket(QMdmmPacket(QMdmmProtocol::TypeReply, QMdmmProtocol::RequestUpgrade, arr));
    }
}
