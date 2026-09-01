// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmserver.h"
#include "qmdmmserver_p.h"

#include "qmdmmagent.h"
#include "qmdmmlogicrunner_p.h"

#include <QLocalSocket>
#include <QTcpSocket>

#include <cmath>
#include <limits>
#include <utility>

/**
 * @file qmdmmserver.h
 * @brief This is the file where the networking Server is defined.
 */

namespace QMdmmNetworking {
#ifndef DOXYGEN
namespace v0 {
#endif

/**
 * @class ServerConfiguration
 * @brief Contains configurations of server
 *
 * The configuration is a @c QJsonObject with well-known keys. Because the class inherits
 * @c QJsonObject, arbitrary keys can be inserted and values are not validated on access; unknown
 * keys are ignored. Call @c deserialize() to validate a raw JSON value (rejecting wrong types and
 * out-of-range values) before using it.
 */

/**
 * @property ServerConfiguration::tcpEnabled
 * @brief Whether the TCP server is enabled, default true
 */

/**
 * @property ServerConfiguration::tcpPort
 * @brief The TCP port, default 6366
 */

/**
 * @property ServerConfiguration::localEnabled
 * @brief Whether the local socket server is enabled, default true
 */

/**
 * @property ServerConfiguration::localSocketName
 * @brief The local socket name, default "QMdmm"
 */

/**
 * @property ServerConfiguration::websocketEnabled
 * @brief Whether the WebSocket server is enabled, default true
 */

/**
 * @property ServerConfiguration::websocketName
 * @brief The WebSocket server name, default "QMdmm"
 */

/**
 * @property ServerConfiguration::websocketPort
 * @brief The WebSocket port, default 6367
 */

/**
 * @property ServerConfiguration::playerNumPerRoom
 * @brief The player number per room, default 3
 */

/**
 * @property ServerConfiguration::requestTimeout
 * @brief The request timeout in seconds, default 20
 *
 * The server's request timer only backstops abnormal cases now (D-020): a healthy
 * client replies or explicitly gives up on its own, so the timer fires only when the
 * client is gone or stuck. The value is in seconds; it is converted to milliseconds
 * when the request timer is armed.
 */

/**
 * @fn ServerConfiguration::tcpEnabled() const
 * @brief getter of @c ServerConfiguration::tcpEnabled
 * @return @c ServerConfiguration::tcpEnabled
 */

/**
 * @fn ServerConfiguration::setTcpEnabled(bool tcpEnabled)
 * @brief setter of @c ServerConfiguration::tcpEnabled
 * @param tcpEnabled @c ServerConfiguration::tcpEnabled
 */

/**
 * @fn ServerConfiguration::tcpPort() const
 * @brief getter of @c ServerConfiguration::tcpPort
 * @return @c ServerConfiguration::tcpPort
 */

/**
 * @fn ServerConfiguration::setTcpPort(uint16_t tcpPort)
 * @brief setter of @c ServerConfiguration::tcpPort
 * @param tcpPort @c ServerConfiguration::tcpPort
 */

/**
 * @fn ServerConfiguration::localEnabled() const
 * @brief getter of @c ServerConfiguration::localEnabled
 * @return @c ServerConfiguration::localEnabled
 */

/**
 * @fn ServerConfiguration::setLocalEnabled(bool localEnabled)
 * @brief setter of @c ServerConfiguration::localEnabled
 * @param localEnabled @c ServerConfiguration::localEnabled
 */

/**
 * @fn ServerConfiguration::localSocketName() const
 * @brief getter of @c ServerConfiguration::localSocketName
 * @return @c ServerConfiguration::localSocketName
 */

/**
 * @fn ServerConfiguration::setLocalSocketName(const QString &localSocketName)
 * @brief setter of @c ServerConfiguration::localSocketName
 * @param localSocketName @c ServerConfiguration::localSocketName
 */

/**
 * @fn ServerConfiguration::websocketEnabled() const
 * @brief getter of @c ServerConfiguration::websocketEnabled
 * @return @c ServerConfiguration::websocketEnabled
 */

/**
 * @fn ServerConfiguration::setWebsocketEnabled(bool websocketEnabled)
 * @brief setter of @c ServerConfiguration::websocketEnabled
 * @param websocketEnabled @c ServerConfiguration::websocketEnabled
 */

/**
 * @fn ServerConfiguration::websocketName() const
 * @brief getter of @c ServerConfiguration::websocketName
 * @return @c ServerConfiguration::websocketName
 */

/**
 * @fn ServerConfiguration::setWebsocketName(const QString &websocketName)
 * @brief setter of @c ServerConfiguration::websocketName
 * @param websocketName @c ServerConfiguration::websocketName
 */

/**
 * @fn ServerConfiguration::websocketPort() const
 * @brief getter of @c ServerConfiguration::websocketPort
 * @return @c ServerConfiguration::websocketPort
 */

/**
 * @fn ServerConfiguration::setWebsocketPort(uint16_t websocketPort)
 * @brief setter of @c ServerConfiguration::websocketPort
 * @param websocketPort @c ServerConfiguration::websocketPort
 */

/**
 * @fn ServerConfiguration::playerNumPerRoom() const
 * @brief getter of @c ServerConfiguration::playerNumPerRoom
 * @return @c ServerConfiguration::playerNumPerRoom
 */

/**
 * @fn ServerConfiguration::setPlayerNumPerRoom(int playerNumPerRoom)
 * @brief setter of @c ServerConfiguration::playerNumPerRoom
 * @param playerNumPerRoom @c ServerConfiguration::playerNumPerRoom
 */

/**
 * @fn ServerConfiguration::requestTimeout() const
 * @brief getter of @c ServerConfiguration::requestTimeout
 * @return @c ServerConfiguration::requestTimeout
 */

/**
 * @fn ServerConfiguration::setRequestTimeout(int requestTimeout)
 * @brief setter of @c ServerConfiguration::requestTimeout
 * @param requestTimeout @c ServerConfiguration::requestTimeout
 */

/**
 * @brief Get default values of configuration
 * @return default configuration
 */
const ServerConfiguration &ServerConfiguration::defaults()
{
    // clang-format off
    static const ServerConfiguration defaultInstance {
        qMakePair(QStringLiteral("tcpEnabled"), true),
        qMakePair(QStringLiteral("tcpPort"), (int)(6366U)),
        qMakePair(QStringLiteral("localEnabled"), true),
        qMakePair(QStringLiteral("localSocketName"), QStringLiteral("QMdmm")),
        qMakePair(QStringLiteral("websocketEnabled"), true),
        qMakePair(QStringLiteral("websocketName"), QStringLiteral("QMdmm")),
        qMakePair(QStringLiteral("websocketPort"), (int)(6367U)),
        qMakePair(QStringLiteral("playerNumPerRoom"), 3),
        qMakePair(QStringLiteral("requestTimeout"), 20),
    };
    // clang-format on

    return defaultInstance;
}

#define CONVERTTOTYPEBOOL(v) ((v).toBool())
#define CONVERTTOTYPEUINT16T(v) ((uint16_t)((v).toInt()))
#define CONVERTTOTYPEQSTRING(v) ((v).toString())
#define CONVERTTOTYPEINT(v) ((v).toInt())
#define IMPLEMENTATION_CONFIGURATION(type, valueName, ValueName, convertToType, convertToJsonValue) \
    type ServerConfiguration::valueName() const                                                     \
    {                                                                                               \
        if (contains(QStringLiteral(#valueName)))                                                   \
            return convertToType(value(QStringLiteral(#valueName)));                                \
        return convertToType(defaults().value(QStringLiteral(#valueName)));                         \
    }                                                                                               \
    void ServerConfiguration::set##ValueName(type value)                                            \
    {                                                                                               \
        insert(QStringLiteral(#valueName), convertToJsonValue(value));                              \
    }

#define IMPLEMENTATION_CONFIGURATION_SETTER_CONST_REFERENCE(type, valueName, ValueName, convertToType, convertToJsonValue) \
    type ServerConfiguration::valueName() const                                                                            \
    {                                                                                                                      \
        if (contains(QStringLiteral(#valueName)))                                                                          \
            return convertToType(value(QStringLiteral(#valueName)));                                                       \
        return convertToType(defaults().value(QStringLiteral(#valueName)));                                                \
    }                                                                                                                      \
    void ServerConfiguration::set##ValueName(const type &value)                                                            \
    {                                                                                                                      \
        insert(QStringLiteral(#valueName), convertToJsonValue(value));                                                     \
    }

IMPLEMENTATION_CONFIGURATION(bool, tcpEnabled, TcpEnabled, CONVERTTOTYPEBOOL, )
IMPLEMENTATION_CONFIGURATION(uint16_t, tcpPort, TcpPort, CONVERTTOTYPEUINT16T, )
IMPLEMENTATION_CONFIGURATION(bool, localEnabled, LocalEnabled, CONVERTTOTYPEBOOL, )
IMPLEMENTATION_CONFIGURATION_SETTER_CONST_REFERENCE(QString, localSocketName, LocalSocketName, CONVERTTOTYPEQSTRING, )
IMPLEMENTATION_CONFIGURATION(bool, websocketEnabled, WebsocketEnabled, CONVERTTOTYPEBOOL, )
IMPLEMENTATION_CONFIGURATION_SETTER_CONST_REFERENCE(QString, websocketName, WebsocketName, CONVERTTOTYPEQSTRING, )
IMPLEMENTATION_CONFIGURATION(uint16_t, websocketPort, WebsocketPort, CONVERTTOTYPEUINT16T, )
IMPLEMENTATION_CONFIGURATION(int, playerNumPerRoom, PlayerNumPerRoom, CONVERTTOTYPEINT, )
IMPLEMENTATION_CONFIGURATION(int, requestTimeout, RequestTimeout, CONVERTTOTYPEINT, )

#undef IMPLEMENTATION_CONFIGURATION_SETTER_CONST_REFERENCE
#undef IMPLEMENTATION_CONFIGURATION
#undef CONVERTTOTYPEQSTRING
#undef CONVERTTOTYPEUINT16T
#undef CONVERTTOTYPEBOOL
#undef CONVERTTOTYPEINT

/**
 * @brief deserialize @c QJsonValue to @c ServerConfiguration
 * @param value the value to be deserialized
 * @return if the deserialize succeeded
 * @note It is possible to convert the value to @c QJsonObject and directly assign the value, since this class inherits @c QJsonObject, but the value check in this function will be nonexistent then.
 *
 * The value must be an object containing every configuration key. Boolean fields must be booleans and
 * string fields must be strings; every numeric field must be a whole number (fractions, NaN and negatives
 * are rejected). Ports must be in [1, 65535] (port 0 is reserved); @c playerNumPerRoom must be at least 2
 * (a game needs an opponent for rock-paper-scissors action-order resolution); @c requestTimeout is either
 * 0 (no explicit timeout, grace only) or at least 15 seconds. Unknown keys are ignored.
 */
bool ServerConfiguration::deserialize(const QJsonValue &value) // NOLINT(readability-function-cognitive-complexity)
{
    if (!value.isObject())
        return false;

    const QJsonObject ob = value.toObject();
    QJsonObject result;

    // A numeric field must be a whole number: JSON numbers are doubles, so reject fractions
    // (e.g. 1.5), NaN, negatives, and values that do not fit in an int, which toInt() would
    // otherwise silently truncate or wrap.
    const auto parseNonNegativeInt = [](const QJsonValue &v, int *out) {
        if (!v.isDouble())
            return false;

        const double d = v.toDouble();
        if (d != std::floor(d) || d < 0.0 || d > static_cast<double>(std::numeric_limits<int>::max()))
            return false;

        *out = static_cast<int>(d);
        return true;
    };

#define CONF_BOOL(member)                                                          \
    {                                                                              \
        if (!ob.contains(QStringLiteral(#member)))                                 \
            return false;                                                          \
        if (!ob.value(QStringLiteral(#member)).isBool())                           \
            return false;                                                          \
        result.insert(QStringLiteral(#member), ob.value(QStringLiteral(#member))); \
    }

#define CONF_STRING(member)                                                        \
    {                                                                              \
        if (!ob.contains(QStringLiteral(#member)))                                 \
            return false;                                                          \
        if (!ob.value(QStringLiteral(#member)).isString())                         \
            return false;                                                          \
        result.insert(QStringLiteral(#member), ob.value(QStringLiteral(#member))); \
    }

#define CONF_PORT(member)                                                     \
    {                                                                         \
        int parsed = 0;                                                       \
        if (!ob.contains(QStringLiteral(#member)))                            \
            return false;                                                     \
        if (!parseNonNegativeInt(ob.value(QStringLiteral(#member)), &parsed)) \
            return false;                                                     \
        if (parsed == 0 || parsed > 65535)                                    \
            return false;                                                     \
        result.insert(QStringLiteral(#member), parsed);                       \
    }

    CONF_BOOL(tcpEnabled);
    CONF_PORT(tcpPort);
    CONF_BOOL(localEnabled);
    CONF_STRING(localSocketName);
    CONF_BOOL(websocketEnabled);
    CONF_STRING(websocketName);
    CONF_PORT(websocketPort);

#undef CONF_BOOL
#undef CONF_STRING
#undef CONF_PORT

    // playerNumPerRoom: whole number >= 2 (a game needs at least two players).
    {
        int parsed = 0;
        if (!ob.contains(QStringLiteral("playerNumPerRoom")))
            return false;
        if (!parseNonNegativeInt(ob.value(QStringLiteral("playerNumPerRoom")), &parsed))
            return false;
        if (parsed < 2)
            return false;
        result.insert(QStringLiteral("playerNumPerRoom"), parsed);
    }

    // requestTimeout: 0 (no explicit timeout, grace only) or >= 15 seconds.
    {
        int parsed = 0;
        if (!ob.contains(QStringLiteral("requestTimeout")))
            return false;
        if (!parseNonNegativeInt(ob.value(QStringLiteral("requestTimeout")), &parsed))
            return false;
        if (parsed != 0 && parsed < 15)
            return false;
        result.insert(QStringLiteral("requestTimeout"), parsed);
    }

    *this = result;
    return true;
}

#ifndef DOXYGEN
} // namespace v0
#endif

#ifndef DOXYGEN
namespace p {

QHash<QMdmmCore::Protocol::NotifyId, void (ServerP::*)(Socket *, const QJsonValue &)> ServerP::notifyCallback {
    std::make_pair(QMdmmCore::Protocol::NotifyPingServer, &ServerP::pingServer),
    std::make_pair(QMdmmCore::Protocol::NotifySignIn, &ServerP::signIn),
    std::make_pair(QMdmmCore::Protocol::NotifyObserve, &ServerP::observe),
};

ServerP::ServerP(ServerConfiguration serverConfiguration_, QMdmmCore::LogicConfiguration logicConfiguration, Server *q)
    : QObject(q)
    , serverConfiguration(std::move(serverConfiguration_))
    , logicConfiguration(std::move(logicConfiguration))
    , q(q)
    , t(nullptr)
    , l(nullptr)
    , w(nullptr)
    , current(nullptr)
{
    // Tcp
    if (serverConfiguration.tcpEnabled()) {
        t = new QTcpServer(this);

        // Qt post-6.4: new pendingConnectionAvailable signal, emitted after connection is added to pending connection queue
        // instead of the connection is established (Pre 6.3 behavior)
        connect(t, &QTcpServer::pendingConnectionAvailable, this, &ServerP::tcpServerNewConnection);
    }

    // Local
    if (serverConfiguration.localEnabled()) {
        l = new QLocalServer(this);
        l->setSocketOptions(QLocalServer::WorldAccessOption);
        connect(l, &QLocalServer::newConnection, this, &ServerP::localServerNewConnection);
    }

    // WebSocket
    if (serverConfiguration.websocketEnabled()) {
        w = new QWebSocketServer(serverConfiguration.websocketName(), QWebSocketServer::NonSecureMode, this);
        connect(w, &QWebSocketServer::newConnection, this, &ServerP::websocketServerNewConnection);
    }
}

void ServerP::pingServer(Socket *socket, const QJsonValue &packetValue)
{
    emit socket->sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyPongServer, packetValue));
}

void ServerP::signIn(Socket *socket, const QJsonValue &packetValue)
{
    do {
        if (!packetValue.isObject())
            break;

        QJsonObject ob = packetValue.toObject();

#define CONVERTAGENTSTATE() QMdmmCore::Data::AgentState(v.toInt())

        // NOLINTBEGIN(bugprone-macro-parentheses)

        // no do .. while (0) here since I'd like 'break' to exit outside this block
        // where "socket->setError(...)" should be done
#define CONF(member, check, convert)                      \
    {                                                     \
        if (!ob.contains(QStringLiteral(#member)))        \
            break;                                        \
        QJsonValue v = ob.value(QStringLiteral(#member)); \
        if (!v.check())                                   \
            break;                                        \
        member = convert();                               \
    }

        // NOLINTEND(bugprone-macro-parentheses)

        QString playerName;
        CONF(playerName, isString, v.toString);

        QString screenName;
        CONF(screenName, isString, v.toString);

        QMdmmCore::Data::AgentState agentState;
        CONF(agentState, isDouble, CONVERTAGENTSTATE);

#undef CONF
#undef CONVERTAGENTSTATE

        // The client reports how many round events it received before a drop, so the server can
        // replay only the events it missed (precise catch-up). A fresh sign-in omits the field and
        // defaults to 0 -- harmless, since a fresh room has an empty round-event log.
        int lastRoundEventSeq = 0;
        if (ob.contains(QStringLiteral("lastRoundEventSeq"))) {
            QJsonValue vlastRoundEventSeq = ob.value(QStringLiteral("lastRoundEventSeq"));
            if (vlastRoundEventSeq.isDouble())
                lastRoundEventSeq = vlastRoundEventSeq.toInt();
        }

        // Reconnect path: a reconnecting player may live in ANY room, not just `current`. `current`
        // only tracks the room currently recruiting players; full rooms keep running in the
        // background and are deleted later on gameOver. So scan every LogicRunner child for the
        // offline agent and reconnect it in whichever room it is found. A still-online duplicate
        // name falls through to addAgent below, which rejects it as a spurious "new player" error.
        const auto runners = findChildren<LogicRunner *>();
        for (LogicRunner *runner : runners) {
            Agent *existing = runner->agent(playerName);
            if (existing == nullptr || existing->state().testFlag(QMdmmCore::Data::StateMaskOnline))
                continue;

            // D-018: the socket is digested at the wire layer and the room only deals with agents,
            // so a reconnect is split across the two. Find the agent's wire plumbing, rebind the
            // socket + replay the missed round events on it, then restore the agent's state +
            // snapshot on the logic side.
            p::ServerConnection *conn = existing->findChild<p::ServerConnection *>();
            if (conn == nullptr) {
                // A local agent has no wire; a sign-in over the wire cannot reconnect it.
                socket->setError({Socket::ProtocolError, {}});
                return;
            }

            conn->reconnect(socket, lastRoundEventSeq);
            if (runner->reconnectAgent(existing) != nullptr)
                return;

            socket->setError({Socket::ProtocolError, {}});
            return;
        }

        if (current == nullptr || current->full()) {
            current = new LogicRunner(logicConfiguration, serverConfiguration.playerNumPerRoom(), this);
            connect(current, &LogicRunner::gameOver, this, &ServerP::logicRunnerGameOver);
        }

        // Assemble the agent on the operation side (network path): create the agent (identity +
        // controller) and its wire plumbing (ServerConnection), bind the socket, then register the
        // whole thing with the room via addAgent. The ServerConnection is a child of the agent so
        // it travels with it; it reports socket drops as an Agent event the room listens to.
        Agent *agent = new Agent(playerName, current);
        agent->setScreenName(screenName);
        agent->setState(agentState);
        p::ServerConnection *conn = new p::ServerConnection(agent, logicConfiguration, serverConfiguration.requestTimeout(), agent);
        conn->setSocket(socket);

        if (current->addAgent(agent) == nullptr)
            break;

        return;
    } while (false);

    socket->setError({Socket::ProtocolError, {}});
}

void ServerP::observe(Socket *socket, const QJsonValue &packetValue)
{
    // TODO
    Q_UNUSED(packetValue);
    socket->setError({Socket::ProtocolError, {}});
}

void ServerP::introduceSocket(Socket *socket) // NOLINT(readability-make-member-function-const)
{
    connect(socket, &Socket::packetReceived, this, &ServerP::socketPacketReceived);

    QJsonObject ob;
    ob.insert(QStringLiteral("versionNumber"), QMdmmCore::Global::version().toString());
    ob.insert(QStringLiteral("protocolVersion"), QMdmmCore::Protocol::version());
    QMdmmCore::Packet packet(QMdmmCore::Protocol::NotifyVersion, ob);
    emit socket->sendPacket(packet);
}

void ServerP::tcpServerNewConnection()
{
    while (t->hasPendingConnections()) {
        QTcpSocket *socket = t->nextPendingConnection();
        Socket *mdmmSocket = new Socket(socket, this);
        introduceSocket(mdmmSocket);
    }
}

void ServerP::localServerNewConnection()
{
    while (l->hasPendingConnections()) {
        QLocalSocket *socket = l->nextPendingConnection();
        Socket *mdmmSocket = new Socket(socket, this);
        introduceSocket(mdmmSocket);
    }
}

void ServerP::websocketServerNewConnection()
{
    while (w->hasPendingConnections()) {
        QWebSocket *socket = w->nextPendingConnection();
        Socket *mdmmSocket = new Socket(socket, this);
        introduceSocket(mdmmSocket);
    }
}

void ServerP::socketPacketReceived(const QMdmmCore::Packet &packet)
{
    Socket *socket = qobject_cast<Socket *>(sender());

    if (socket == nullptr)
        return;

    if (packet.type() == QMdmmCore::Protocol::TypeNotify) {
        if ((packet.notifyId() & QMdmmCore::Protocol::NotifyToServerMask) != 0) {
            // These packages should be processed in Server
            void (ServerP::*call)(Socket *, const QJsonValue &) = notifyCallback.value(packet.notifyId(), nullptr);
            if (call != nullptr)
                (this->*call)(socket, packet.value());
            else
                socket->setError({Socket::ProtocolError, {}});
        }
    }
}

void ServerP::logicRunnerGameOver()
{
    if (LogicRunner *runner = qobject_cast<LogicRunner *>(sender()); runner != nullptr) {
        if (current == runner)
            current = nullptr;

        runner->deleteLater();
    }
}

} // namespace p
#endif

#ifndef DOXYGEN
namespace v0 {
#endif

/**
 * @class Server
 * @brief The server that accepts connections and runs games.
 *
 * The server listens on the configured transports (TCP / local socket / WebSocket) and,
 * once enough players sign in, starts a @c LogicRunner for a complete game.
 */

/**
 * @brief ctor.
 * @param serverConfiguration The configuration of the server
 * @param logicConfiguration The configuration of the logic used by the games
 * @param parent QObject parent.
 */
Server::Server(ServerConfiguration serverConfiguration, QMdmmCore::LogicConfiguration logicConfiguration, QObject *parent)
    : QObject(parent)
    , d(new p::ServerP(std::move(serverConfiguration), std::move(logicConfiguration), this))
{
}

/**
 * @brief Start listening on all enabled transports
 * @return @c true if all enabled transports are listening successfully
 *
 * Each enabled transport is started independently; a transport that fails to start emits
 * @c listenError() with its name and the underlying error string, so the caller can tell which
 * transport failed and why (the aggregate return value alone cannot). The return value is
 * @c false if any enabled transport failed.
 */
bool Server::listen()
{
    bool ret = true;

    if (d->serverConfiguration.tcpEnabled() && !d->t->listen(QHostAddress::Any, d->serverConfiguration.tcpPort())) {
        emit listenError(QStringLiteral("tcp"), d->t->errorString(), QPrivateSignal());
        ret = false;
    }

    if (d->serverConfiguration.localEnabled() && !d->l->listen(d->serverConfiguration.localSocketName())) {
        emit listenError(QStringLiteral("local"), d->l->errorString(), QPrivateSignal());
        ret = false;
    }

    if (d->serverConfiguration.websocketEnabled() && !d->w->listen(QHostAddress::Any, d->serverConfiguration.websocketPort())) {
        emit listenError(QStringLiteral("websocket"), d->w->errorString(), QPrivateSignal());
        ret = false;
    }

    return ret;
}

/**
 * @brief Stop listening on all enabled transports.
 *
 * Closes every listening socket that @c listen() started, releasing the ports / local-socket
 * names so they can be reused. Already-accepted connections are left untouched; only the
 * listening sockets are shut down. Safe to call even if the server is not currently listening.
 */
void Server::close()
{
    if (d->t != nullptr)
        d->t->close();
    if (d->l != nullptr)
        d->l->close();
    if (d->w != nullptr)
        d->w->close();
}

/**
 * @fn Server::listenError(const QString &transportName, const QString &errorString, QPrivateSignal)
 * @brief emitted when a transport fails to start listening in @c listen()
 * @param transportName which transport failed: @c "tcp", @c "local" or @c "websocket"
 * @param errorString the transport's own description of the failure
 */

// No need to delete d.
// It will always be deleted by QObject dtor
/**
 * @brief dtor.
 */
Server::~Server() = default;

#ifndef DOXYGEN
} // namespace v0
#endif
} // namespace QMdmmNetworking
