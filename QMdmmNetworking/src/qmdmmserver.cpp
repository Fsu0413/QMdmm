// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmserver.h"
#include "qmdmmserver_p.h"

#include "qmdmmagent.h"

#include <QLocalSocket>
#include <QTcpSocket>

QHash<QMdmmProtocol::NotifyId, void (QMdmmServerPrivate::*)(QMdmmSocket *, const QJsonValue &)> QMdmmServerPrivate::notifyCallback {
    std::make_pair(QMdmmProtocol::NotifyPingServer, &QMdmmServerPrivate::pingServer),
    std::make_pair(QMdmmProtocol::NotifySignIn, &QMdmmServerPrivate::signIn),
    std::make_pair(QMdmmProtocol::NotifyObserve, &QMdmmServerPrivate::observe),
};

QMdmmServerPrivate::QMdmmServerPrivate(const QMdmmServerConfiguration &serverConfiguration, QMdmmServer *p)
    : QObject(p)
    , serverConfiguration(serverConfiguration)
    , p(p)
    , t(nullptr)
    , l(nullptr)
    , w(nullptr)
    , current(nullptr)
{
    // Tcp
    if (serverConfiguration.tcpEnabled) {
        t = new QTcpServer(this);
        connect(t,
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
                // Qt pre-6.3: old behavior
                &QTcpServer::newConnection,
#else
                // Qt post-6.4: new pendingConnectionAvailable signal, emitted after connection is added to pending connection queue instead of the connection is estabilished
                &QTcpServer::pendingConnectionAvailable,
#endif
                this, &QMdmmServerPrivate::tcpServerNewConnection);
    }

    // Local
    if (serverConfiguration.localEnabled) {
        l = new QLocalServer(this);
        l->setSocketOptions(QLocalServer::WorldAccessOption);
        connect(l, &QLocalServer::newConnection, this, &QMdmmServerPrivate::localServerNewConnection);
    }

    // WebSocket
    if (serverConfiguration.websocketEnabled) {
        w = new QWebSocketServer(serverConfiguration.websocketName, QWebSocketServer::NonSecureMode, this);
        connect(w, &QWebSocketServer::newConnection, this, &QMdmmServerPrivate::websocketServerNewConnection);
    }
}

void QMdmmServerPrivate::pingServer(QMdmmSocket *socket, const QJsonValue &packetValue)
{
    emit socket->sendPacket(QMdmmPacket(QMdmmProtocol::NotifyPongServer, packetValue));
}

void QMdmmServerPrivate::signIn(QMdmmSocket *socket, const QJsonValue &packetValue)
{
    do {
        if (!packetValue.isObject())
            break;

        QJsonObject ob = packetValue.toObject();

#define CONVERTAGENTSTATE() QMdmmData::AgentState(v.toInt())

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

        QString playerName;
        CONF(playerName, isString, v.toString);

        QString screenName;
        CONF(screenName, isString, v.toString);

        QMdmmData::AgentState agentState;
        CONF(agentState, isDouble, CONVERTAGENTSTATE);

#undef CONF
#undef CONVERTAGENTSTATE

        // TODO: check if reconnect

        if (current == nullptr || current->full()) {
            current = new QMdmmLogicRunner(serverConfiguration.logicConfiguration, this);
            connect(current, &QMdmmLogicRunner::gameOver, this, &QMdmmServerPrivate::logicRunnerGameOver);
        }

        if (current->addSocket(playerName, screenName, agentState, socket) == nullptr)
            break;

        return;
    } while (false);

    socket->setHasError(true);
}

void QMdmmServerPrivate::observe(QMdmmSocket *socket, const QJsonValue &packetValue)
{
    // TODO
    socket->setHasError(true);
}

void QMdmmServerPrivate::introduceSocket(QMdmmSocket *socket) // NOLINT(readability-make-member-function-const)
{
    connect(socket, &QMdmmSocket::packetReceived, this, &QMdmmServerPrivate::socketPacketReceived);

    QJsonObject ob;
    ob.insert(QStringLiteral("versionNumber"), QMdmmGlobal::version().toString());
    ob.insert(QStringLiteral("protocolVersion"), QMdmmProtocol::protocolVersion());
    QMdmmPacket packet(QMdmmProtocol::NotifyVersion, ob);
    emit socket->sendPacket(packet);
}

void QMdmmServerPrivate::tcpServerNewConnection()
{
    while (t->hasPendingConnections()) {
        QTcpSocket *socket = t->nextPendingConnection();
        QMdmmSocket *mdmmSocket = new QMdmmSocket(socket, this);
        introduceSocket(mdmmSocket);
    }
}

void QMdmmServerPrivate::localServerNewConnection()
{
    while (l->hasPendingConnections()) {
        QLocalSocket *socket = l->nextPendingConnection();
        QMdmmSocket *mdmmSocket = new QMdmmSocket(socket, this);
        introduceSocket(mdmmSocket);
    }
}

void QMdmmServerPrivate::websocketServerNewConnection()
{
    while (w->hasPendingConnections()) {
        QWebSocket *socket = w->nextPendingConnection();
        QMdmmSocket *mdmmSocket = new QMdmmSocket(socket, this);
        introduceSocket(mdmmSocket);
    }
}

void QMdmmServerPrivate::socketPacketReceived(QMdmmPacket packet)
{
    QMdmmSocket *socket = qobject_cast<QMdmmSocket *>(sender());

    if (socket == nullptr)
        return;

    if (packet.type() == QMdmmProtocol::TypeNotify) {
        if ((packet.notifyId() | QMdmmProtocol::NotifyToServerMask) != 0) {
            // These packages should be processed in Server
            void (QMdmmServerPrivate::*call)(QMdmmSocket *, const QJsonValue &) = notifyCallback.value(packet.notifyId(), nullptr);
            if (call != nullptr)
                (this->*call)(socket, packet.value());
            else
                socket->setHasError(true);
        }
    }
}

void QMdmmServerPrivate::logicRunnerGameOver()
{
    if (QMdmmLogicRunner *runner = qobject_cast<QMdmmLogicRunner *>(sender()); runner != nullptr) {
        if (current == runner)
            current = nullptr;

        runner->deleteLater();
    }
}

QMdmmServer::QMdmmServer(const QMdmmServerConfiguration &serverConfiguration, QObject *parent)
    : QObject(parent)
    , d(new QMdmmServerPrivate(serverConfiguration, this))
{
}

bool QMdmmServer::listenTcpServer()
{
    if (d->serverConfiguration.tcpEnabled)
        return d->t->listen(QHostAddress::Any, d->serverConfiguration.tcpPort);

    return false;
}

bool QMdmmServer::listenLocalServer()
{
    if (d->serverConfiguration.localEnabled)
        return d->l->listen(d->serverConfiguration.localSocketName);

    return false;
}

bool QMdmmServer::listenWebsocketServer()
{
    if (d->serverConfiguration.websocketEnabled)
        return d->w->listen(QHostAddress::Any, d->serverConfiguration.websocketPort);

    return false;
}

// No need to delete d.
// It will always be deleted by QObject dtor
QMdmmServer::~QMdmmServer() = default;
