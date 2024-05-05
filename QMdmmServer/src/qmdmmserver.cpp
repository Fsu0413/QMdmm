#include "qmdmmserver.h"

#include "qmdmmagent.h"
#include "qmdmmlogicrunner.h"

#include <QHash>
#include <QPointer>
#include <QTcpServer>
#include <QTcpSocket>

class QMdmmSocketPrivate : public QObject
{
    Q_OBJECT

public:
    QMdmmSocketPrivate(QIODevice *socket, QMdmmSocket::Type type, QMdmmSocket *p)
        : QObject(p)
        , socket(socket)
        , p(p)
        , hasError(false)
        , type(type)
    {
        connect(socket, &QIODevice::aboutToClose, this, &QMdmmSocketPrivate::deleteLater);
        connect(socket, &QIODevice::readyRead, this, &QMdmmSocketPrivate::messageReceived);
        connect(p, &QMdmmSocket::sendPacket, this, &QMdmmSocketPrivate::sendPacket);
    }

    QPointer<QIODevice> socket;
    QMdmmSocket *p;

    bool hasError;
    QMdmmSocket::Type type;

public slots:
    void sendPacket(QMdmmPacket packet)
    {
        if (socket == nullptr)
            return;

        socket->write(packet);
        if (type == QMdmmSocket::TypeQTcpSocket)
            static_cast<QTcpSocket *>(socket.data())->flush();
    }

    void messageReceived()
    {
        if (socket == nullptr)
            return;

        while (socket->canReadLine()) {
            QByteArray arr = socket->readLine();
            QMdmmPacket packet(arr);
            QString packetError;
            bool packetHasError = packet.hasError(&packetError);

            if (packetHasError) {
                // TODO: make use of this error string
                (void)packetError;

                // Don't process more package for this connection. It is not guaranteed to be the desired client
                p->setHasError(true);
                break;
            }

            emit p->packetReceived(packet, QMdmmSocket::QPrivateSignal());
            if (hasError)
                break;
        }
    }
};

QMdmmSocket::QMdmmSocket(QIODevice *socket, Type type, QObject *parent)
    : QObject(parent)
    , d(new QMdmmSocketPrivate(socket, type, this))
{
}

QMdmmSocket::~QMdmmSocket()
{
    // No need to delete d.
}

void QMdmmSocket::setHasError(bool hasError)
{
    d->hasError = hasError;

    // QIODevice::close() disconnects socket - it is virtual and inherited in QAbstractSocket!
    if (hasError && d->socket != nullptr)
        d->socket->close();
}

bool QMdmmSocket::hasError() const
{
    return d->hasError;
}

class QMdmmServerPrivate : public QObject
{
    Q_OBJECT

    static QHash<QMdmmProtocol::NotifyId, bool (QMdmmServerPrivate::*)(QMdmmSocket *, const QJsonValue &)> cb;

public:
    QMdmmServerPrivate(const QMdmmServerConfiguration &serverConfiguration, QMdmmServer *p);

    // callbacks
    bool pongClient(QMdmmSocket *, const QJsonValue &);
    bool pingServer(QMdmmSocket *, const QJsonValue &);
    bool signIn(QMdmmSocket *socket, const QJsonValue &packetValue);
    bool observe(QMdmmSocket *, const QJsonValue &);

public slots: // NOLINT(readability-redundant-access-specifiers)
    void serverNewConnection();
    void socketPacketReceived(QMdmmPacket packet);

public: // NOLINT(readability-redundant-access-specifiers)
    // variables
    QMdmmServerConfiguration serverConfiguration;
    QMdmmServer *p;
    QTcpServer *s;
    QList<QMdmmLogicRunner *> runners;
    QMdmmLogicRunner *current;
};

QHash<QMdmmProtocol::NotifyId, bool (QMdmmServerPrivate::*)(QMdmmSocket *, const QJsonValue &)> QMdmmServerPrivate::cb {
    std::make_pair(QMdmmProtocol::NotifyPongClient, &QMdmmServerPrivate::pongClient),
    std::make_pair(QMdmmProtocol::NotifyPingServer, &QMdmmServerPrivate::pingServer),
    std::make_pair(QMdmmProtocol::NotifySignIn, &QMdmmServerPrivate::signIn),
    std::make_pair(QMdmmProtocol::NotifyObserve, &QMdmmServerPrivate::observe),
};

QMdmmServerPrivate::QMdmmServerPrivate(const QMdmmServerConfiguration &serverConfiguration, QMdmmServer *p)
    : QObject(p)
    , serverConfiguration(serverConfiguration)
    , p(p)
    , s(new QTcpServer(this))
    , current(nullptr)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
    // Qt pre-6.3: old behavior
    connect(s, &QTcpServer::newConnection, this, &QMdmmServerPrivate::serverNewConnection);
#else
    // Qt post-6.4: new pendingConnectionAvailable signal, emitted after connection is added to pending connection queue instead of the connection is estabilished
    connect(s, &QTcpServer::pendingConnectionAvailable, this, &QMdmmServerPrivate::serverNewConnection);
#endif
    s->listen(QHostAddress::Any, serverConfiguration.port);
}

bool QMdmmServerPrivate::pongClient(QMdmmSocket *, const QJsonValue &)
{
    return true;
}

bool QMdmmServerPrivate::pingServer(QMdmmSocket *, const QJsonValue &)
{
    return true;
}

bool QMdmmServerPrivate::signIn(QMdmmSocket *socket, const QJsonValue &packetValue)
{
    return false;
}

bool QMdmmServerPrivate::observe(QMdmmSocket *, const QJsonValue &)
{
    // not implemented by now
    return false;
}

void QMdmmServerPrivate::serverNewConnection() // NOLINT(readability-make-member-function-const)
{
    while (s->hasPendingConnections()) {
        QTcpSocket *socket = s->nextPendingConnection();
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
        QMdmmSocket *mdmmSocket = new QMdmmSocket(socket, QMdmmSocket::TypeQTcpSocket, this);
        connect(mdmmSocket, &QMdmmSocket::packetReceived, this, &QMdmmServerPrivate::socketPacketReceived);

        QJsonObject ob;
        ob.insert(QStringLiteral("versionNumber"), QMdmmGlobal::version().toString());
        QMdmmPacket packet(QMdmmProtocol::NotifyVersion, ob);
        emit mdmmSocket->sendPacket(packet);
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
            bool (QMdmmServerPrivate::*call)(QMdmmSocket *, const QJsonValue &) = cb.value(packet.notifyId());
            if (call != nullptr)
                (this->*call)(socket, packet.value());
            else
                socket->setHasError(true);
        }
    }
}

QMdmmServer::QMdmmServer(const QMdmmServerConfiguration &serverConfiguration, QObject *parent)
    : QObject(parent)
    , d(new QMdmmServerPrivate(serverConfiguration, this))
{
}

QMdmmServer::~QMdmmServer()
{
    // No need to delete d.
    // It will always be deleted by QObject dtor
}

#include "qmdmmserver.moc"
