// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmprotocol.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSharedData>

/**
 * @file qmdmmprotocol.h
 * @brief QMdmm protocol definitions
 */

/**
 * @namespace QMdmmProtocol
 * @brief The namespace for protocol
 */

/**
 * @enum QMdmmProtocol::RequestId
 * @brief The IDs for requests and replies
 */

/**
 * @var QMdmmProtocol::RequestId QMdmmProtocol::RequestInvalid
 * @brief An invalid request / reply
 */

/**
 * @var QMdmmProtocol::RequestId QMdmmProtocol::RequestStoneScissorsCloth
 * @brief A request of Stone-Scissors-Cloth
 */

/**
 * @var QMdmmProtocol::RequestId QMdmmProtocol::RequestActionOrder
 * @brief A request of action order
 */

/**
 * @var QMdmmProtocol::RequestId QMdmmProtocol::RequestAction
 * @brief A request of action
 */

/**
 * @var QMdmmProtocol::RequestId QMdmmProtocol::RequestUpgrade
 * @brief A request of upgrade
 */

/**
 * @enum QMdmmProtocol::NotifyId
 * @brief The IDs for notifies
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyInvalid
 * @brief An invalid notify
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyFromServerMask
 * @brief A mask of notify from server
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyPongServer
 * @brief A notify from server of a ping-pong (heartbeat)
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyVersion
 * @brief A notify from server of version number
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyFromAgentMask
 * @brief A mask of notify from agent
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyLogicConfiguration
 * @brief A notify from agent of logic configuration
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyAgentStateChanged
 * @brief A notify from agent of agent state
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyPlayerAdded
 * @brief A notify from agent of player added
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyPlayerRemoved
 * @brief A notify from agent of player removed
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyGameStart
 * @brief A notify from agent of game started
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyRoundStart
 * @brief A notify from agent of round started
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyStoneScissorsCloth
 * @brief A notify from agent of Stone-Scissors-Cloth
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyActionOrder
 * @brief A notify from agent of action order
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyAction
 * @brief A notify from agent of action
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyRoundOver
 * @brief A notify from agent of round over
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyUpgrade
 * @brief A notify from agent of upgrade
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyGameOver
 * @brief A notify from agent of game over
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifySpoken
 * @brief A notify from agent of agent spoken
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyOperated
 * @brief A notify from agent of agent operated
 *
 * @todo OB functionality
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyToServerMask
 * @brief A mask of notify to server
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyPingServer
 * @brief A notify to server of a ping-pong (heartbeat)
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifySignIn
 * @brief A notify to server of sign in
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyObserve
 * @brief A notify to server of observe
 *
 * @todo OB functionality
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyToAgentMask
 * @brief A mask of notify to agent
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifySpeak
 * @brief A notify to agent of speaking
 */

/**
 * @var QMdmmProtocol::NotifyId QMdmmProtocol::NotifyOperate
 * @brief A notify to agent of operating
 *
 * @todo OB functionality
 */

/**
 * @enum QMdmmProtocol::PacketType
 * @brief The type of a packet
 */

/**
 * @var QMdmmProtocol::PacketType QMdmmProtocol::TypeInvalid
 * @brief An invalid packet
 */

/**
 * @var QMdmmProtocol::PacketType QMdmmProtocol::TypeRequest
 * @brief A request packet
 */

/**
 * @var QMdmmProtocol::PacketType QMdmmProtocol::TypeReply
 * @brief A reply packet
 */

/**
 * @var QMdmmProtocol::PacketType QMdmmProtocol::TypeNotify
 * @brief A notify packet
 */

/**
 * @brief get the protocol version of current implementation
 * @return the version of protocol
 *
 * Currently only version 0 is implemented. Different protocol version is incompatible.
 */
int QMdmmProtocol::protocolVersion() noexcept
{
    return 0;
}

#ifndef DOXYGEN

QMdmmPacketData::QMdmmPacketData()
{
    insert(QStringLiteral("type"), static_cast<int>(QMdmmProtocol::TypeInvalid));
    insert(QStringLiteral("requestId"), static_cast<int>(QMdmmProtocol::RequestInvalid));
    insert(QStringLiteral("notifyId"), static_cast<int>(QMdmmProtocol::NotifyInvalid));
    insert(QStringLiteral("value"), QJsonValue());
}

QMdmmPacketData::QMdmmPacketData(QMdmmProtocol::PacketType type, QMdmmProtocol::RequestId requestId, QMdmmProtocol::NotifyId notifyId, const QJsonValue &v)
{
    insert(QStringLiteral("type"), static_cast<int>(type));
    insert(QStringLiteral("requestId"), static_cast<int>(requestId));
    insert(QStringLiteral("notifyId"), static_cast<int>(notifyId));
    insert(QStringLiteral("value"), v);
}

QMdmmPacketData::QMdmmPacketData(const QJsonObject &ob) noexcept(noexcept(QJsonObject(ob)))
    : QJsonObject(ob)
{
}

QMdmmPacketData &QMdmmPacketData::operator=(const QJsonObject &ob) noexcept(noexcept(QJsonObject::operator=(ob)))
{
    QJsonObject::operator=(ob);
    return *this;
}

#endif

/**
 * @class QMdmmPacket
 * @brief A packet for QMdmm protocol
 *
 * A packet of QMdmm Protocol is a JSON object, encoded in a single line.
 */

/**
 * @brief ctor.
 */
QMdmmPacket::QMdmmPacket()
    : d(new QMdmmPacketData)
{
}

/**
 * @brief ctor of a request or reply packet
 * @param type the packet type
 * @param requestId the request ID
 * @param value the value / payload of the packet
 */
QMdmmPacket::QMdmmPacket(QMdmmProtocol::PacketType type, QMdmmProtocol::RequestId requestId, const QJsonValue &value)
    : d(new QMdmmPacketData(type, requestId, QMdmmProtocol::NotifyInvalid, value))
{
}

/**
 * @brief ctor of a notify packet
 * @param notifyId the notify ID
 * @param value the value / payload of the packet
 */
QMdmmPacket::QMdmmPacket(QMdmmProtocol::NotifyId notifyId, const QJsonValue &value)
    : d(new QMdmmPacketData(QMdmmProtocol::TypeNotify, QMdmmProtocol::RequestInvalid, notifyId, value))
{
}

/**
 * @brief get the type of this packet
 * @return packet type
 */
QMdmmProtocol::PacketType QMdmmPacket::type() const
{
    return static_cast<QMdmmProtocol::PacketType>(d->value(QStringLiteral("type")).toInt(QMdmmProtocol::TypeInvalid));
}

/**
 * @brief get the request ID of this packet
 * @return request ID
 */
QMdmmProtocol::RequestId QMdmmPacket::requestId() const
{
    return static_cast<QMdmmProtocol::RequestId>(d->value(QStringLiteral("requestId")).toInt(QMdmmProtocol::RequestInvalid));
}

/**
 * @brief get the notify ID of this packet
 * @return notify ID
 */
QMdmmProtocol::NotifyId QMdmmPacket::notifyId() const
{
    return static_cast<QMdmmProtocol::NotifyId>(d->value(QStringLiteral("notifyId")).toInt(QMdmmProtocol::NotifyInvalid));
}

/**
 * @brief get the value / payload of this packet
 * @return value / payload
 */
QJsonValue QMdmmPacket::value() const
{
    return d->value(QStringLiteral("value"));
}

/**
 * @brief serialize the packet
 * @return serialized byte array for sending
 *
 * To deserialize the returned QByteArray, use @c QMdmmPacket::fromJson() function.
 */
QByteArray QMdmmPacket::serialize() const
{
    QJsonDocument doc(*d);
    return doc.toJson(QJsonDocument::Compact).append('\n');
}

/**
 * @fn QMdmmPacket::operator QByteArray() const
 * @brief serialize the packet
 * @return serialized byte array for sending
 *
 * overload conversion.
 */

/**
 * @brief checks if this packet has error
 * @param errorString (out) the optional error string
 * @return if the packet has error
 */
bool QMdmmPacket::hasError(QString *errorString) const
{
    if (errorString != nullptr)
        *errorString = d->error;

    return !d->error.isEmpty();
}

/**
 * @brief deserialize the byte array
 * @param serialized the serialized byte array
 * @param errorString (out) the optional error string
 * @return the deserialized packet
 *
 * This does the opposite of @c QMdmmPacket::serialize() function.
 */
QMdmmPacket QMdmmPacket::fromJson(const QByteArray &serialized, QString *errorString)
{
    if (errorString == nullptr) {
        static QString _errorString;
        errorString = &_errorString;
    }

    errorString->clear();

    QMdmmPacket ret;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(serialized, &err);

    if (err.error != QJsonParseError::NoError) {
        *errorString = QStringLiteral("Json error: ").append(err.errorString());
        ret.d->error = *errorString;
        return ret;
    }

    if (!doc.isObject()) {
        *errorString = QStringLiteral("Document is not object");
        ret.d->error = *errorString;
        return ret;
    }

    *(ret.d) = doc.object();

    if (!ret.d->contains(QStringLiteral("type"))) {
        *errorString = QStringLiteral("'type' is non-existent");
        ret.d->error = *errorString;
        return ret;
    }
    if (!ret.d->value(QStringLiteral("type")).isDouble()) {
        *errorString = QStringLiteral("'type' is not number");
        ret.d->error = *errorString;
        return ret;
    }

    if (!ret.d->contains(QStringLiteral("requestId"))) {
        *errorString = QStringLiteral("'requestId' is non-existent");
        ret.d->error = *errorString;
        return ret;
    }
    if (!ret.d->value(QStringLiteral("requestId")).isDouble()) {
        *errorString = QStringLiteral("'requestId' is not number");
        ret.d->error = *errorString;
        return ret;
    }

    if (!ret.d->contains(QStringLiteral("notifyId"))) {
        *errorString = QStringLiteral("'notifyId' is non-existent");
        ret.d->error = *errorString;
        return ret;
    }
    if (!ret.d->value(QStringLiteral("notifyId")).isDouble()) {
        *errorString = QStringLiteral("'notifyId' is not number");
        ret.d->error = *errorString;
        return ret;
    }

    if (!ret.d->contains(QStringLiteral("value"))) {
        *errorString = QStringLiteral("'value' is non-existent");
        ret.d->error = *errorString;
        return ret;
    }

    return ret;
}
