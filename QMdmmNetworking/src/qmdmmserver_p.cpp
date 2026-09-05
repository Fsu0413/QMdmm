// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmserver_p.h"

#include "qmdmmagent.h"
#include "qmdmmserverconnection_p.h"
#include "qmdmmsocket.h"

#include <QMdmmProtocol>

namespace QMdmmNetworking {
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
        const QList<LogicRunner *> runners = findChildren<LogicRunner *>();
        for (LogicRunner *runner : runners) {
            Agent *existing = runner->agent(playerName);
            if (existing == nullptr || existing->state().testFlag(QMdmmCore::Data::StateMaskOnline))
                continue;

            // D-018: the socket is digested at the wire layer and the room only deals with agents,
            // so a reconnect is split across the two. Find the agent's wire plumbing, rebind the
            // socket + replay the missed round events on it, then restore the agent's state +
            // snapshot on the logic side.
            p::ServerConnectionP *conn = existing->findChild<p::ServerConnectionP *>();
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
        p::ServerConnectionP *conn = new p::ServerConnectionP(agent, logicConfiguration, serverConfiguration.requestTimeout(), agent);
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
} // namespace QMdmmNetworking
