// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmserver.h"
#include "qmdmmserver_p.h"

#include "qmdmmagent.h"

#include <QLocalSocket>
#include <QTcpSocket>
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
    };
    // clang-format on

    return defaultInstance;
}

#define CONVERTTOTYPEBOOL(v) ((v).toBool())
#define CONVERTTOTYPEUINT16T(v) ((uint16_t)((v).toInt()))
#define CONVERTTOTYPEQSTRING(v) ((v).toString())
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

#undef IMPLEMENTATION_CONFIGURATION_SETTER_CONST_REFERENCE
#undef IMPLEMENTATION_CONFIGURATION
#undef CONVERTTOTYPEQSTRING
#undef CONVERTTOTYPEUINT16T
#undef CONVERTTOTYPEBOOL

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
        // where "socket->hasError(true)" should be done
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

        // The client reports the last round-event sequence number it received before a drop, so the
        // server can replay only the events it missed (precise catch-up). A fresh sign-in omits the
        // field and defaults to 0 -- harmless, since a fresh room has an empty round-event cache.
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
        // name falls through to addSocket below, which rejects it as a spurious "new player" error.
        const auto runners = findChildren<LogicRunner *>();
        for (LogicRunner *runner : runners) {
            Agent *existing = runner->agent(playerName);
            if (existing == nullptr || existing->state().testFlag(QMdmmCore::Data::StateMaskOnline))
                continue;
            if (runner->reconnect(playerName, socket, lastRoundEventSeq) != nullptr)
                return;
            socket->setHasError(true);
            return;
        }

        if (current == nullptr || current->full()) {
            current = new LogicRunner(logicConfiguration, this);
            connect(current, &LogicRunner::gameOver, this, &ServerP::logicRunnerGameOver);
        }

        if (current->addSocket(playerName, screenName, agentState, socket) == nullptr)
            break;

        return;
    } while (false);

    socket->setHasError(true);
}

void ServerP::observe(Socket *socket, const QJsonValue &packetValue)
{
    // TODO
    Q_UNUSED(packetValue);
    socket->setHasError(true);
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
                socket->setHasError(true);
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
 */
bool Server::listen()
{
    bool ret = true;

    if (d->serverConfiguration.tcpEnabled())
        ret = d->t->listen(QHostAddress::Any, d->serverConfiguration.tcpPort()) && ret;
    if (d->serverConfiguration.localEnabled())
        ret = d->l->listen(d->serverConfiguration.localSocketName()) && ret;
    if (d->serverConfiguration.websocketEnabled())
        ret = d->w->listen(QHostAddress::Any, d->serverConfiguration.websocketPort()) && ret;

    return ret;
}

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
