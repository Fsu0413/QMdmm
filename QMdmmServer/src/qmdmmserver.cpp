#include "qmdmmserver.h"

#include <QMdmmCore/QMdmmAgent>
#include <QMdmmCore/QMdmmLogic>

#include <QHash>
#include <QTcpServer>
#include <QTcpSocket>

class QMdmmServerPrivate : public QObject
{
    Q_OBJECT

    static QHash<QMdmmProtocol::NotifyId, bool (QMdmmServerPrivate::*)(QIODevice *, const QJsonValue &)> cb;

public:
    QMdmmServerPrivate(const QMdmmServerConfiguration &serverConfiguration, QMdmmServer *p);

    // callbacks
    bool pongClient(QIODevice *socket, const QJsonValue &packetValue);
    bool pingServer(QIODevice *socket, const QJsonValue &packetValue);
    bool signIn(QIODevice *socket, const QJsonValue &packetValue);
    bool observe(QIODevice *socket, const QJsonValue &packetValue);

public slots: // NOLINT(readability-redundant-access-specifiers)
    void serverNewConnection();
    void socketMessageReceived();

public: // NOLINT(readability-redundant-access-specifiers)
    // variables
    QMdmmServerConfiguration serverConfiguration;
    QMdmmServer *p;
    QTcpServer *s;
    QList<QMdmmLogicRunner *> runners;
    QMdmmLogicRunner *current;
};

QHash<QMdmmProtocol::NotifyId, bool (QMdmmServerPrivate::*)(QIODevice *, const QJsonValue &)> QMdmmServerPrivate::cb {
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

bool QMdmmServerPrivate::pongClient(QIODevice *socket, const QJsonValue &packetValue)
{
    return false;
}

bool QMdmmServerPrivate::pingServer(QIODevice *socket, const QJsonValue &packetValue)
{
    return false;
}

bool QMdmmServerPrivate::signIn(QIODevice *socket, const QJsonValue &packetValue)
{
    return false;
}

bool QMdmmServerPrivate::observe(QIODevice *socket, const QJsonValue &packetValue)
{
    return false;
}

void QMdmmServerPrivate::serverNewConnection() // NOLINT(readability-make-member-function-const)
{
    while (s->hasPendingConnections()) {
        QTcpSocket *socket = s->nextPendingConnection();
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
        connect(socket, &QTcpSocket::readyRead, this, &QMdmmServerPrivate::socketMessageReceived);

        QJsonObject ob;
        ob.insert(QStringLiteral("versionNumber"), QMdmmGlobal::version().toString());
        QMdmmPacket packet(QMdmmProtocol::NotifyVersion, ob);
        socket->write(packet);
        socket->flush();
    }
}

void QMdmmServerPrivate::socketMessageReceived()
{
    QIODevice *device = qobject_cast<QIODevice *>(sender());

    if (device == nullptr)
        return;

    bool errorFlag = false;

    while (device->canReadLine()) {
        QByteArray arr = device->readLine();
        QMdmmPacket packet(arr);
        QString packetError;
        bool packetHasError = packet.hasError(&packetError);

        if (packetHasError) {
            // TODO: make use of this error string
            (void)packetError;

            // Don't process more package for this connection. It is not guaranteed to be the desired client
            errorFlag = true;
            break;
        }

        if (packet.type() == QMdmmProtocol::TypeNotify) {
            if ((packet.notifyId() | QMdmmProtocol::NotifyToServerMask) != 0) {
                // These packages should be processed in Server
                bool (QMdmmServerPrivate::*call)(QIODevice *, const QJsonValue &) = cb.value(packet.notifyId());
                if (call != nullptr) {
                    (this->*call)(device, packet.value());
                } else {
                    // incompatible client?
                    errorFlag = true;
                    break;
                }
            } else {
                // TODO: Notify to Logic
            }
        } else {
            // TODO: request / reply
        }
    }

    if (errorFlag) {
        // QIODevice::close() disconnects socket - it is virtual and inherited in QAbstractSocket!
        device->close();
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
