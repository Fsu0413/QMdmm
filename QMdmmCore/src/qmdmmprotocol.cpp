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

namespace QMdmmCore {

#ifndef DOXYGEN
namespace v0 {
#else
inline namespace v1 {
#endif

/**
 * @namespace Protocol
 * @brief The namespace for protocol
 */

/**
 * @enum Protocol::RequestId
 * @brief The IDs for requests and replies
 */

/**
 * @var Protocol::RequestId Protocol::RequestInvalid
 * @brief An invalid request / reply
 */

/**
 * @var Protocol::RequestId Protocol::RequestStoneScissorsCloth
 * @brief A request of Stone-Scissors-Cloth
 */

/**
 * @var Protocol::RequestId Protocol::RequestActionOrder
 * @brief A request of action order
 */

/**
 * @var Protocol::RequestId Protocol::RequestAction
 * @brief A request of action
 */

/**
 * @var Protocol::RequestId Protocol::RequestUpgrade
 * @brief A request of upgrade
 */

/**
 * @enum Protocol::NotifyId
 * @brief The IDs for notifies
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyInvalid
 * @brief An invalid notify
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyFromServerMask
 * @brief A mask of notify from server
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyPongServer
 * @brief A notify from server of a ping-pong (heartbeat)
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyVersion
 * @brief A notify from server of version number
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyFromAgentMask
 * @brief A mask of notify from agent
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyLogicConfiguration
 * @brief A notify from agent of logic configuration
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyAgentStateChanged
 * @brief A notify from agent of agent state
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyPlayerAdded
 * @brief A notify from agent of player added
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyPlayerRemoved
 * @brief A notify from agent of player removed
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyGameStart
 * @brief A notify from agent of game started
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyRoundStart
 * @brief A notify from agent of round started
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyStoneScissorsCloth
 * @brief A notify from agent of Stone-Scissors-Cloth
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyActionOrder
 * @brief A notify from agent of action order
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyAction
 * @brief A notify from agent of action
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyRoundOver
 * @brief A notify from agent of round over
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyUpgrade
 * @brief A notify from agent of upgrade
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyGameOver
 * @brief A notify from agent of game over
 */

/**
 * @var Protocol::NotifyId Protocol::NotifySpoken
 * @brief A notify from agent of agent spoken
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyOperated
 * @brief A notify from agent of agent operated
 *
 * @todo OB functionality
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyToServerMask
 * @brief A mask of notify to server
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyPingServer
 * @brief A notify to server of a ping-pong (heartbeat)
 */

/**
 * @var Protocol::NotifyId Protocol::NotifySignIn
 * @brief A notify to server of sign in
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyObserve
 * @brief A notify to server of observe
 *
 * @todo OB functionality
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyToAgentMask
 * @brief A mask of notify to agent
 */

/**
 * @var Protocol::NotifyId Protocol::NotifySpeak
 * @brief A notify to agent of speaking
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyOperate
 * @brief A notify to agent of operating
 *
 * @todo OB functionality
 */

/**
 * @enum Protocol::PacketType
 * @brief The type of a packet
 */

/**
 * @var Protocol::PacketType Protocol::TypeInvalid
 * @brief An invalid packet
 */

/**
 * @var Protocol::PacketType Protocol::TypeRequest
 * @brief A request packet
 */

/**
 * @var Protocol::PacketType Protocol::TypeReply
 * @brief A reply packet
 */

/**
 * @var Protocol::PacketType Protocol::TypeNotify
 * @brief A notify packet
 */

/**
 * @brief get the protocol version of current implementation
 * @return the version of protocol
 *
 * Currently only version 0 is implemented. Different protocol version is incompatible.
 */
int Protocol::version() noexcept
{
    return 0;
}

#ifndef DOXYGEN

PacketData::PacketData()
{
    insert(QStringLiteral("type"), static_cast<int>(Protocol::TypeInvalid));
    insert(QStringLiteral("requestId"), static_cast<int>(Protocol::RequestInvalid));
    insert(QStringLiteral("notifyId"), static_cast<int>(Protocol::NotifyInvalid));
    insert(QStringLiteral("value"), QJsonValue());
}

PacketData::PacketData(Protocol::PacketType type, Protocol::RequestId requestId, Protocol::NotifyId notifyId, const QJsonValue &v)
{
    insert(QStringLiteral("type"), static_cast<int>(type));
    insert(QStringLiteral("requestId"), static_cast<int>(requestId));
    insert(QStringLiteral("notifyId"), static_cast<int>(notifyId));
    insert(QStringLiteral("value"), v);
}

PacketData::PacketData(const QJsonObject &ob) noexcept(noexcept(QJsonObject(ob)))
    : QJsonObject(ob)
{
}

PacketData &PacketData::operator=(const QJsonObject &ob) noexcept(noexcept(QJsonObject::operator=(ob)))
{
    QJsonObject::operator=(ob);
    return *this;
}

#endif

/**
 * @class Packet
 * @brief A packet for QMdmm protocol
 *
 * A packet of QMdmm Protocol is a JSON object, encoded in a single line.
 */

/**
 * @brief ctor.
 */
Packet::Packet()
    : d(new PacketData)
{
}

/**
 * @brief ctor of a request or reply packet
 * @param type the packet type
 * @param requestId the request ID
 * @param value the value / payload of the packet
 */
Packet::Packet(Protocol::PacketType type, Protocol::RequestId requestId, const QJsonValue &value)
    : d(new PacketData(type, requestId, Protocol::NotifyInvalid, value))
{
}

/**
 * @brief ctor of a notify packet
 * @param notifyId the notify ID
 * @param value the value / payload of the packet
 */
Packet::Packet(Protocol::NotifyId notifyId, const QJsonValue &value)
    : d(new PacketData(Protocol::TypeNotify, Protocol::RequestInvalid, notifyId, value))
{
}

/**
 * @brief get the type of this packet
 * @return packet type
 */
Protocol::PacketType Packet::type() const
{
    return static_cast<Protocol::PacketType>(d->value(QStringLiteral("type")).toInt(Protocol::TypeInvalid));
}

/**
 * @brief get the request ID of this packet
 * @return request ID
 */
Protocol::RequestId Packet::requestId() const
{
    Protocol::PacketType t = type();
    if (t == Protocol::TypeRequest || t == Protocol::TypeReply)
        return static_cast<Protocol::RequestId>(d->value(QStringLiteral("requestId")).toInt(Protocol::RequestInvalid));

    return Protocol::RequestInvalid;
}

/**
 * @brief get the notify ID of this packet
 * @return notify ID
 */
Protocol::NotifyId Packet::notifyId() const
{
    Protocol::PacketType t = type();
    if (t == Protocol::TypeNotify)
        return static_cast<Protocol::NotifyId>(d->value(QStringLiteral("notifyId")).toInt(Protocol::NotifyInvalid));

    return Protocol::NotifyInvalid;
}

/**
 * @brief get the value / payload of this packet
 * @return value / payload
 */
QJsonValue Packet::value() const
{
    return d->value(QStringLiteral("value"));
}

/**
 * @brief serialize the packet
 * @return serialized byte array for sending
 *
 * To deserialize the returned QByteArray, use @c Packet::fromJson() function.
 */
QByteArray Packet::serialize() const
{
    // TODO: abnormal case
    QJsonDocument doc(*d);
    return doc.toJson(QJsonDocument::Compact);
}

/**
 * @fn Packet::operator QByteArray() const
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
bool Packet::hasError(QString *errorString) const
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
 * This does the opposite of @c Packet::serialize() function.
 */
Packet Packet::fromJson(const QByteArray &serialized, QString *errorString)
{
    if (errorString == nullptr) {
        static QString _errorString;
        errorString = &_errorString;
    }

    errorString->clear();

    Packet ret;

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
} // namespace v0
} // namespace QMdmmCore
