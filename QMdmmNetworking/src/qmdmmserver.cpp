// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmserver.h"
#include "qmdmmserver_p.h"

#include "qmdmmagent.h"

#include <QLocalSocket>
#include <QTcpSocket>
#include <utility>

const QMdmmServerConfiguration &QMdmmServerConfiguration::defaults()
{
    // clang-format off
    static const QMdmmServerConfiguration defaultInstance {
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
    type QMdmmServerConfiguration::valueName() const                                                \
    {                                                                                               \
        if (contains(QStringLiteral(#valueName)))                                                   \
            return convertToType(value(QStringLiteral(#valueName)));                                \
        return convertToType(defaults().value(QStringLiteral(#valueName)));                         \
    }                                                                                               \
    void QMdmmServerConfiguration::set##ValueName(type value)                                       \
    {                                                                                               \
        insert(QStringLiteral(#valueName), convertToJsonValue(value));                              \
    }

#define IMPLEMENTATION_CONFIGURATION_SETTER_CONST_REFERENCE(type, valueName, ValueName, convertToType, convertToJsonValue) \
    type QMdmmServerConfiguration::valueName() const                                                                       \
    {                                                                                                                      \
        if (contains(QStringLiteral(#valueName)))                                                                          \
            return convertToType(value(QStringLiteral(#valueName)));                                                       \
        return convertToType(defaults().value(QStringLiteral(#valueName)));                                                \
    }                                                                                                                      \
    void QMdmmServerConfiguration::set##ValueName(const type &value)                                                       \
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

QHash<QMdmmCore::Protocol::NotifyId, void (QMdmmServerPrivate::*)(QMdmmSocket *, const QJsonValue &)> QMdmmServerPrivate::notifyCallback {
    std::make_pair(QMdmmCore::Protocol::NotifyPingServer, &QMdmmServerPrivate::pingServer),
    std::make_pair(QMdmmCore::Protocol::NotifySignIn, &QMdmmServerPrivate::signIn),
    std::make_pair(QMdmmCore::Protocol::NotifyObserve, &QMdmmServerPrivate::observe),
};

QMdmmServerPrivate::QMdmmServerPrivate(QMdmmServerConfiguration serverConfiguration_, QMdmmCore::LogicConfiguration logicConfiguration, QMdmmServer *q)
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
        connect(t, &QTcpServer::pendingConnectionAvailable, this, &QMdmmServerPrivate::tcpServerNewConnection);
    }

    // Local
    if (serverConfiguration.localEnabled()) {
        l = new QLocalServer(this);
        l->setSocketOptions(QLocalServer::WorldAccessOption);
        connect(l, &QLocalServer::newConnection, this, &QMdmmServerPrivate::localServerNewConnection);
    }

    // WebSocket
    if (serverConfiguration.websocketEnabled()) {
        w = new QWebSocketServer(serverConfiguration.websocketName(), QWebSocketServer::NonSecureMode, this);
        connect(w, &QWebSocketServer::newConnection, this, &QMdmmServerPrivate::websocketServerNewConnection);
    }
}

void QMdmmServerPrivate::pingServer(QMdmmSocket *socket, const QJsonValue &packetValue)
{
    emit socket->sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyPongServer, packetValue));
}

void QMdmmServerPrivate::signIn(QMdmmSocket *socket, const QJsonValue &packetValue)
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

        // TODO: check if reconnect

        if (current == nullptr || current->full()) {
            current = new QMdmmLogicRunner(logicConfiguration, this);
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
    Q_UNUSED(packetValue);
    socket->setHasError(true);
}

void QMdmmServerPrivate::introduceSocket(QMdmmSocket *socket) // NOLINT(readability-make-member-function-const)
{
    connect(socket, &QMdmmSocket::packetReceived, this, &QMdmmServerPrivate::socketPacketReceived);

    QJsonObject ob;
    ob.insert(QStringLiteral("versionNumber"), QMdmmCore::Global::version().toString());
    ob.insert(QStringLiteral("protocolVersion"), QMdmmCore::Protocol::version());
    QMdmmCore::Packet packet(QMdmmCore::Protocol::NotifyVersion, ob);
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

void QMdmmServerPrivate::socketPacketReceived(const QMdmmCore::Packet &packet)
{
    QMdmmSocket *socket = qobject_cast<QMdmmSocket *>(sender());

    if (socket == nullptr)
        return;

    if (packet.type() == QMdmmCore::Protocol::TypeNotify) {
        if ((packet.notifyId() & QMdmmCore::Protocol::NotifyToServerMask) != 0) {
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

QMdmmServer::QMdmmServer(QMdmmServerConfiguration serverConfiguration, QMdmmCore::LogicConfiguration logicConfiguration, QObject *parent)
    : QObject(parent)
    , d(new QMdmmServerPrivate(std::move(serverConfiguration), std::move(logicConfiguration), this))
{
}

bool QMdmmServer::listen()
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
QMdmmServer::~QMdmmServer() = default;
