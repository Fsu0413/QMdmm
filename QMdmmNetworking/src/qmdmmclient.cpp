// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmclient.h"
#include "qmdmmclient_p.h"

#include <QMdmmRoom>

#include <QDateTime>
#include <QJsonArray>

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

bool QMdmmClient::connectToHost(const QString &host, QMdmmCore::Data::AgentState initialState)
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

QMdmmCore::Room *QMdmmClient::room()
{
    return d->room;
}

const QMdmmCore::Room *QMdmmClient::room() const
{
    return d->room;
}

void QMdmmClient::notifySpeak(const QString &content)
{
    // Although JSON is native UTF-8 we decided to use Base64 anyway.
    // This can make our request / response all in one line.
    if (d->socket != nullptr)
        emit d->socket->sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifySpeak, QString::fromLatin1(content.toUtf8().toBase64())));
}

void QMdmmClient::notifyOperate(const void *todo)
{
    Q_UNIMPLEMENTED();
    Q_UNUSED(todo);

    if (d->socket != nullptr)
        emit d->socket->sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::NotifyOperate, {}));
}

void QMdmmClient::requestTimeout()
{
    // This should be a definitely invalid reply, to trigger default reply logic implemented in server.
    if (d->socket != nullptr && d->currentRequest != QMdmmCore::Protocol::RequestInvalid) {
        d->currentRequest = QMdmmCore::Protocol::RequestInvalid;
        emit d->socket->sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::TypeReply, d->currentRequest, {}));
    }
}

void QMdmmClient::replyStoneScissorsCloth(QMdmmCore::Data::StoneScissorsCloth stoneScissorsCloth)
{
    if (d->socket != nullptr && d->currentRequest == QMdmmCore::Protocol::RequestStoneScissorsCloth) {
        d->currentRequest = QMdmmCore::Protocol::RequestInvalid;
        emit d->socket->sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::TypeReply, QMdmmCore::Protocol::RequestStoneScissorsCloth, static_cast<int>(stoneScissorsCloth)));
    }
}

void QMdmmClient::replyActionOrder(const QList<int> &actionOrder)
{
    if (d->socket != nullptr && d->currentRequest == QMdmmCore::Protocol::RequestActionOrder) {
        d->currentRequest = QMdmmCore::Protocol::RequestInvalid;
        QJsonArray arr;
        foreach (int a, actionOrder)
            arr.append(a);

        emit d->socket->sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::TypeReply, QMdmmCore::Protocol::RequestActionOrder, arr));
    }
}

void QMdmmClient::replyAction(QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace)
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

void QMdmmClient::replyUpgrade(const QList<QMdmmCore::Data::UpgradeItem> &upgrades)
{
    if (d->socket != nullptr && d->currentRequest == QMdmmCore::Protocol::RequestUpgrade) {
        d->currentRequest = QMdmmCore::Protocol::RequestInvalid;
        QJsonArray arr;
        foreach (QMdmmCore::Data::UpgradeItem it, upgrades)
            arr.append(static_cast<int>(it));

        emit d->socket->sendPacket(QMdmmCore::Packet(QMdmmCore::Protocol::TypeReply, QMdmmCore::Protocol::RequestUpgrade, arr));
    }
}
