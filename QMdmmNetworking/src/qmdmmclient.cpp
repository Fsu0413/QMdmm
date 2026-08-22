// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmclient.h"
#include "qmdmmclient_p.h"

#include <QMdmmRoom>

#include <QDateTime>
#include <QJsonArray>

#include <random>

/**
 * @file qmdmmclient.h
 * @brief This is the file where the networking Client is defined.
 */

namespace QMdmmNetworking {

#ifndef DOXYGEN
namespace v0 {
#endif

/**
 * @class ClientConfiguration
 * @brief Contains configurations of client
 */

/**
 * @property ClientConfiguration::screenName
 * @brief The screen name of the client, default "QMdmm-Fans"
 */

/**
 * @fn ClientConfiguration::screenName() const
 * @brief getter of @c ClientConfiguration::screenName
 * @return @c ClientConfiguration::screenName
 */

/**
 * @fn ClientConfiguration::setScreenName(const QString &screenName)
 * @brief setter of @c ClientConfiguration::screenName
 * @param screenName @c ClientConfiguration::screenName
 */

/**
 * @brief Get default values of configuration
 * @return default configuration
 */
const ClientConfiguration &ClientConfiguration::defaults()
{
    // clang-format off
    static const ClientConfiguration defaultInstance {
        std::make_pair(QStringLiteral("screenName"), QStringLiteral("QMdmm-Fans")),
    };
    // clang-format on

    return defaultInstance;
}

#define CONVERTTOTYPEQSTRING(v) v.toString()
#define IMPLEMENTATION_CONFIGURATION(type, valueName, ValueName, convertToType, convertToQVariant) \
    type ClientConfiguration::valueName() const                                                    \
    {                                                                                              \
        if (contains(QStringLiteral(#valueName)))                                                  \
            return convertToType(value(QStringLiteral(#valueName)));                               \
        return convertToType(defaults().value(QStringLiteral(#valueName)));                        \
    }                                                                                              \
    void ClientConfiguration::set##ValueName(type value)                                           \
    {                                                                                              \
        insert(QStringLiteral(#valueName), convertToQVariant(value));                              \
    }

#define IMPLEMENTATION_CONFIGURATION2(type, valueName, ValueName, convertToType, convertToQVariant) \
    type ClientConfiguration::valueName() const                                                     \
    {                                                                                               \
        if (contains(QStringLiteral(#valueName)))                                                   \
            return convertToType(value(QStringLiteral(#valueName)));                                \
        return convertToType(defaults().value(QStringLiteral(#valueName)));                         \
    }                                                                                               \
    void ClientConfiguration::set##ValueName(const type &value)                                     \
    {                                                                                               \
        insert(QStringLiteral(#valueName), convertToQVariant(value));                               \
    }

IMPLEMENTATION_CONFIGURATION2(QString, screenName, ScreenName, CONVERTTOTYPEQSTRING, )

#undef IMPLEMENTATION_CONFIGURATION2
#undef IMPLEMENTATION_CONFIGURATION
#undef CONVERTTOTYPEQSTRING

namespace {
inline QString generateRandomString()
{
    static thread_local std::random_device random1;
    static thread_local std::mt19937 random2(random1());

    QByteArray arr;
    for (int i = 0; i < 30; ++i)
        arr.append(static_cast<char>(random2() % 255));

    return QString::fromLatin1(arr.toBase64(QByteArray::OmitTrailingEquals));
}
} // namespace

/**
 * @class Client
 * @brief The client that connects to a server and plays the game.
 *
 * A Client maintains the connection to a server (through a @c Socket), the local
 * @c Room where the game state is mirrored, and exposes requests / notifications to
 * drive a UI or an automated player.
 */

/**
 * @brief ctor.
 * @param clientConfiguration The configuration of the client
 * @param parent QObject parent.
 */
Client::Client(ClientConfiguration clientConfiguration, QObject *parent)
    : QObject(parent)
    , d(new p::ClientP(std::move(clientConfiguration), this))
{
    setObjectName(generateRandomString());
    d->initSelfAgent();
}

/**
 * @brief dtor.
 */
Client::~Client() = default;

/**
 * @brief Connect to a server
 * @param host the host address to connect to (the scheme decides the transport, see @c Socket::connectToHost())
 * @param initialState the initial agent state used when signing in
 * @return @c true if the connection is initiated successfully, @c false otherwise
 */
bool Client::connectToHost(const QString &host, QMdmmCore::Data::AgentState initialState)
{
    if (d->socket != nullptr) {
        d->socket->disconnect(d);
        d->socket->deleteLater();
    }

    // Remember the host so the client can reconnect by itself after a drop. An
    // explicit connectToHost always starts a fresh session: clear any in-flight
    // reconnect state so a manual reconnect and the automatic retry never fight.
    d->host = host;
    d->initialState = initialState;
    d->reconnectAttempts = 0;
    d->reconnectInProgress = false;
    d->reconnectTimer->stop();

    return d->connectSocket();
}

/**
 * @brief get the local room where the game state is mirrored
 * @return the local room, or @c nullptr if not yet connected
 */
QMdmmCore::Room *Client::room()
{
    return d->room;
}

/**
 * @brief get the local room where the game state is mirrored (const version)
 * @return the local room, or @c nullptr if not yet connected
 */
const QMdmmCore::Room *Client::room() const
{
    return d->room;
}

/**
 * @brief get this client's own agent
 * @return this client's own agent
 */
Agent *Client::agent()
{
    return d->agents.value(objectName(), nullptr);
}

/**
 * @brief get this client's own agent (const version)
 * @return this client's own agent
 */
const Agent *Client::agent() const
{
    return d->agents.value(objectName(), nullptr);
}

/**
 * @brief Notify the server that this client speaks a message
 * @param content the content of the message
 */
void Client::notifySpeak(const QString &content)
{
    // Although JSON is native UTF-8 we decided to use Base64 anyway.
    // This can make our request / response all in one line.
    if (d->socket != nullptr)
        emit d->socket->sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifySpeak, QString::fromLatin1(content.toUtf8().toBase64())));
}

/**
 * @brief Notify the server that this client operates
 * @param todo the operation data
 *
 * @note This is not implemented yet (the observe / operate semantics are undefined).
 */
void Client::notifyOperate(const void *todo)
{
    Q_UNIMPLEMENTED();
    Q_UNUSED(todo);

    if (d->socket != nullptr)
        emit d->socket->sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyOperate, {}));
}

/**
 * @brief Reply to the current request with a timeout (triggers the default reply on the server)
 */
void Client::requestTimeout()
{
    // This should be a definitely invalid reply, to trigger default reply logic implemented in server.
    if (d->socket != nullptr && d->currentRequest != QMdmmCore::Protocol::RequestInvalid) {
        d->currentRequest = QMdmmCore::Protocol::RequestInvalid;
        emit d->socket->sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::TypeReply, d->currentRequest, {}));
    }
}

/**
 * @brief Reply to a Stone-Scissors-Cloth request
 * @param stoneScissorsCloth the chosen Stone-Scissors-Cloth
 */
void Client::replyStoneScissorsCloth(QMdmmCore::Data::StoneScissorsCloth stoneScissorsCloth)
{
    if (d->socket != nullptr && d->currentRequest == QMdmmCore::Protocol::RequestStoneScissorsCloth) {
        d->currentRequest = QMdmmCore::Protocol::RequestInvalid;
        emit d->socket->sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::TypeReply, QMdmmCore::Protocol::RequestStoneScissorsCloth, static_cast<int>(stoneScissorsCloth)));
    }
}

/**
 * @brief Reply to an action order request
 * @param actionOrder the desired action order
 */
void Client::replyActionOrder(const QList<int> &actionOrder)
{
    if (d->socket != nullptr && d->currentRequest == QMdmmCore::Protocol::RequestActionOrder) {
        d->currentRequest = QMdmmCore::Protocol::RequestInvalid;
        QJsonArray arr;
        foreach (int a, actionOrder)
            arr.append(a);

        emit d->socket->sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::TypeReply, QMdmmCore::Protocol::RequestActionOrder, arr));
    }
}

/**
 * @brief Reply to an action request
 * @param action the action to make
 * @param toPlayer the target player name
 * @param toPlace the target place
 */
void Client::replyAction(QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace)
{
    if (d->socket != nullptr && d->currentRequest == QMdmmCore::Protocol::RequestAction) {
        d->currentRequest = QMdmmCore::Protocol::RequestInvalid;
        QJsonObject ob;
        ob.insert(QStringLiteral("action"), static_cast<int>(action));
        ob.insert(QStringLiteral("toPlayer"), toPlayer);
        ob.insert(QStringLiteral("toPlace"), toPlace);

        emit d->socket->sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::TypeReply, QMdmmCore::Protocol::RequestAction, ob));
    }
}

/**
 * @brief Reply to an upgrade request
 * @param upgrades the list of upgrade items
 */
void Client::replyUpgrade(const QList<QMdmmCore::Data::UpgradeItem> &upgrades)
{
    if (d->socket != nullptr && d->currentRequest == QMdmmCore::Protocol::RequestUpgrade) {
        d->currentRequest = QMdmmCore::Protocol::RequestInvalid;
        QJsonArray arr;
        foreach (QMdmmCore::Data::UpgradeItem it, upgrades)
            arr.append(static_cast<int>(it));

        emit d->socket->sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::TypeReply, QMdmmCore::Protocol::RequestUpgrade, arr));
    }
}

/**
 * @fn Client::socketConnectionLost(const QString &errorString, QPrivateSignal)
 * @brief emitted once when the connection drops and the client starts retrying internally
 * @param errorString the reason the connection dropped
 */

/**
 * @fn Client::socketReconnectSucceeded(QPrivateSignal)
 * @brief emitted when the client re-establishes the connection and re-signed in after a disconnect
 */

/**
 * @fn Client::socketErrorDisconnected(const QString &errorString, QPrivateSignal)
 * @brief emitted when the socket encounters an error and gets disconnected
 * @param errorString the error description
 */

/**
 * @fn Client::requestStoneScissorsCloth(const QStringList &playerNames, int strivedOrder, QPrivateSignal)
 * @brief emitted when the server requests a Stone-Scissors-Cloth choice
 * @param playerNames the player names involved in the Stone-Scissors-Cloth
 * @param strivedOrder the action order being strived for (0 if not applicable)
 */

/**
 * @fn Client::requestActionOrder(const QList<int> &remainedOrders, int maximumOrder, int selectionNum, QPrivateSignal)
 * @brief emitted when the server requests the desired action order
 * @param remainedOrders the available (remained) orders
 * @param maximumOrder total number of action orders
 * @param selectionNum the count of selections to make
 */

/**
 * @fn Client::requestAction(int currentOrder, QPrivateSignal)
 * @brief emitted when the server requests an action
 * @param currentOrder the current action order
 */

/**
 * @fn Client::requestUpgrade(int remainingTimes, QPrivateSignal)
 * @brief emitted when the server requests an upgrade
 * @param remainingTimes the remaining upgrade points
 */

/**
 * @fn Client::notifyPlayerAdded(const QString &playerName, const QString &screenName, const QMdmmCore::Data::AgentState &agentState, QPrivateSignal)
 * @brief emitted when a player is added
 * @param playerName the internal name of the added player
 * @param screenName the screen name of the added player
 * @param agentState the state of the added player
 */

/**
 * @fn Client::notifyPlayerRemoved(const QString &playerName, QPrivateSignal)
 * @brief emitted when a player is removed
 * @param playerName the internal name of the removed player
 */

/**
 * @fn Client::notifyGameStart(QPrivateSignal)
 * @brief emitted when the game starts
 */

/**
 * @fn Client::notifyRoundStart(QPrivateSignal)
 * @brief emitted when a round starts
 */

/**
 * @fn Client::notifyStoneScissorsCloth(const QHash<QString, QMdmmCore::Data::StoneScissorsCloth> &ssc, QPrivateSignal)
 * @brief emitted when the Stone-Scissors-Cloth result is reported
 * @param ssc the Stone-Scissors-Cloth choices (key = internal name of player, value = Stone-Scissors-Cloth)
 */

/**
 * @fn Client::notifyActionOrder(const QHash<int, QString> &actionOrderResult, QPrivateSignal)
 * @brief emitted when the action order is confirmed
 * @param actionOrderResult the result of action orders (key = action order, value = internal name of player)
 */

/**
 * @fn Client::notifyAction(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace, QPrivateSignal)
 * @brief emitted when an action is reported
 * @param playerName the internal name of the acting player
 * @param action the action made
 * @param toPlayer the target player name
 * @param toPlace the target place
 */

/**
 * @fn Client::notifyRoundOver(QPrivateSignal)
 * @brief emitted when a round is over
 */

/**
 * @fn Client::notifyUpgrade(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades, QPrivateSignal)
 * @brief emitted when the upgrades are reported
 * @param upgrades the upgrades performed by each player (key = internal name of player, value = list of upgrade items)
 */

/**
 * @fn Client::notifyGameOver(const QStringList &winners, QPrivateSignal)
 * @brief emitted when the game is over
 * @param winners the internal names of the winners
 */

/**
 * @fn Client::notifySpoken(const QString &playerName, const QString &content, QPrivateSignal)
 * @brief emitted when a player speaks
 * @param playerName the internal name of the speaking player
 * @param content the content of the message
 */

/**
 * @fn Client::notifyOperated(QPrivateSignal)
 * @brief emitted when a player operates
 */

#ifndef DOXYGEN
} // namespace v0
#endif
} // namespace QMdmmNetworking
