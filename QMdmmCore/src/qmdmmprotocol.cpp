// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmprotocol.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSharedData>

#include <cmath>
#include <limits>

/**
 * @file qmdmmprotocol.h
 * @brief QMdmm protocol definitions
 */

namespace QMdmmCore {

#ifndef DOXYGEN
namespace v0 {
#endif

/**
 * @namespace Protocol
 * @brief The namespace for protocol
 */

/**
 * @enum Protocol::RequestId
 * @brief The IDs for requests and replies
 *
 * No requests come from the server; all requests originate from the Logic.
 */

/**
 * @var Protocol::RequestId Protocol::RequestInvalid
 * @brief An invalid request / reply
 */

/**
 * @var Protocol::RequestId Protocol::RequestRockPaperScissors
 * @brief A request of Rock-Paper-Scissors
 *
 * Wire format — request: @c {"playerNames": [string], "strivedOrder": int} (a @c strivedOrder of
 * @c 0 selects an action instead of a strived order); reply: @c int rps.
 */

/**
 * @var Protocol::RequestId Protocol::RequestActionOrder
 * @brief A request of action order
 *
 * Wire format — request: @c {"remainedOrders": [int], "maximumOrder": int, "selectionNum": int};
 * reply: @c [int] orders.
 */

/**
 * @var Protocol::RequestId Protocol::RequestAction
 * @brief A request of action
 *
 * Wire format — request: @c int currentOrder; reply: @c {"action": int(Action),
 * "toPlayer": string (optional), "toPlace": int (optional)}.
 */

/**
 * @var Protocol::RequestId Protocol::RequestUpgrade
 * @brief A request of upgrade
 *
 * Wire format — request: @c int remainingTimes; reply: @c [int] item.
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
 *
 * Wire format: @c int64 epoch-milliseconds timestamp (echoed from the ping).
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyVersion
 * @brief A notify from server of version number
 *
 * Wire format: @c {"versionNumber": string, "protocolVersion": int}.
 *
 * When @c protocolVersion differs from @c Protocol::version() the client disconnects: the wire
 * protocol is incompatible, so it drops the connection via @c disconnectFromHost() rather than
 * entering the auto-reconnect loop (which would re-hit the same mismatch). A @c versionNumber
 * mismatch is tolerated — the wire protocol is still compatible — and is currently a no-op.
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyFromAgentMask
 * @brief A mask of notify from agent
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyLogicConfiguration
 * @brief A notify from agent of logic configuration
 *
 * Wire format: broadcast, @c object (see QMdmmCore::LogicConfiguration in qmdmmlogic.h).
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyAgentStateChanged
 * @brief A notify from agent of agent state
 *
 * Wire format: @c {"playerName": string, "agentState": int (AgentState)}.
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyPlayerAdded
 * @brief A notify from agent of player added
 *
 * Wire format: @c {"playerName": string, "screenName": string, "agentState": int (AgentState)}.
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyPlayerRemoved
 * @brief A notify from agent of player removed
 *
 * Wire format: @c {"playerName": string}.
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyGameStart
 * @brief A notify from agent of game started
 *
 * Wire format: broadcast, empty @c object.
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyRoundStart
 * @brief A notify from agent of round started
 *
 * Wire format: broadcast, empty @c object.
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyRockPaperScissors
 * @brief A notify from agent of Rock-Paper-Scissors
 *
 * Wire format: broadcast, @c {playerName: int rps} (an object keyed by player name, value is the
 * Rock-Paper-Scissors choice).
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyActionOrder
 * @brief A notify from agent of action order
 *
 * Wire format: broadcast, @c [string] (an array where index @c i is the player taking order @c i).
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyAction
 * @brief A notify from agent of action
 *
 * Wire format: broadcast, @c {"playerName": string, "action": int (Action),
 * "toPlayer": string (optional), "toPlace": int (optional)}.
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyRoundOver
 * @brief A notify from agent of round over
 *
 * Wire format: broadcast, empty @c object.
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyUpgrade
 * @brief A notify from agent of upgrade
 *
 * Wire format: broadcast, @c {playerName: [int item]} (an object keyed by player name, value is
 * the list of upgrade items).
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyGameOver
 * @brief A notify from agent of game over
 *
 * Wire format: broadcast, @c [string] (the array of winning player names).
 */

/**
 * @var Protocol::NotifyId Protocol::NotifySpoken
 * @brief A notify from agent of agent spoken
 *
 * Wire format: broadcast, @c {"playerName": string, "content": string}.
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
 *
 * Wire format: @c int64 epoch-milliseconds timestamp.
 */

/**
 * @var Protocol::NotifyId Protocol::NotifySignIn
 * @brief A notify to server of sign in
 *
 * Wire format: @c {"playerName": string, "screenName": string, "agentState": int (AgentState),
 * "lastRoundEventSeq": int}.
 */

/**
 * @var Protocol::NotifyId Protocol::NotifyObserve
 * @brief A notify to server of observe
 *
 * Wire format: @c {"observerName": string, "playerName": string}.
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
 *
 * Wire format: @c string (base64-encoded UTF-8 content).
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
 *
 * The top-level object carries four keys:
 * - @c "type": @c int, one of Protocol::PacketType.
 * - @c "requestId": @c int, one of Protocol::RequestId. Meaningful only for request/reply packets,
 *   and @c Protocol::RequestInvalid for notify/invalid packets.
 * - @c "notifyId": @c int, one of Protocol::NotifyId. Meaningful only for notify packets, and
 *   @c Protocol::NotifyInvalid for request/reply/invalid packets.
 * - @c "value": the payload. Its shape depends on the requestId (for request/reply) or notifyId
 *   (for notify); see the per-ID documentation below.
 *
 * Unknown keys are preserved and ignored: @c PacketData derives from @c QJsonObject, so
 * deserialization copies the whole object and only validates the four keys above. Extra keys
 * round-trip through @c serialize() unchanged and are not rejected.
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

namespace {

bool isPacketTypeValid(int type)
{
    switch (static_cast<Protocol::PacketType>(type)) {
    case Protocol::TypeInvalid:
    case Protocol::TypeRequest:
    case Protocol::TypeReply:
    case Protocol::TypeNotify:
        return true;
    default:
        return false;
    }
}

bool isRequestIdValid(int requestId)
{
    switch (static_cast<Protocol::RequestId>(requestId)) {
    case Protocol::RequestInvalid:
    case Protocol::RequestRockPaperScissors:
    case Protocol::RequestActionOrder:
    case Protocol::RequestAction:
    case Protocol::RequestUpgrade:
        return true;
    default:
        return false;
    }
}

bool isNotifyIdValid(int notifyId)
{
    switch (static_cast<Protocol::NotifyId>(notifyId)) {
    case Protocol::NotifyInvalid:
    case Protocol::NotifyPongServer:
    case Protocol::NotifyVersion:
    case Protocol::NotifyLogicConfiguration:
    case Protocol::NotifyAgentStateChanged:
    case Protocol::NotifyPlayerAdded:
    case Protocol::NotifyPlayerRemoved:
    case Protocol::NotifyGameStart:
    case Protocol::NotifyRoundStart:
    case Protocol::NotifyRockPaperScissors:
    case Protocol::NotifyActionOrder:
    case Protocol::NotifyAction:
    case Protocol::NotifyRoundOver:
    case Protocol::NotifyUpgrade:
    case Protocol::NotifyGameOver:
    case Protocol::NotifySpoken:
    case Protocol::NotifyOperated:
    case Protocol::NotifyPingServer:
    case Protocol::NotifySignIn:
    case Protocol::NotifyObserve:
    case Protocol::NotifySpeak:
    case Protocol::NotifyOperate:
        return true;
    default:
        return false;
    }
}

// JSON numbers are doubles; enum fields must be integral values. Returns true and
// stores the value in @p out if @p value is a whole number that fits in an int.
// Rejects fractional values (e.g. 1.5) and out-of-int-range values (e.g. 1e30),
// which toInt() would otherwise silently truncate or wrap.
bool toIntegral(const QJsonValue &value, int *out)
{
    const double d = value.toDouble();
    if (d != std::floor(d) || d < static_cast<double>(std::numeric_limits<int>::min()) || d > static_cast<double>(std::numeric_limits<int>::max()))
        return false;

    *out = static_cast<int>(d);
    return true;
}

} // namespace

/**
 * @brief deserialize the byte array
 * @param serialized the serialized byte array
 * @return the deserialized packet
 *
 * This does the opposite of @c Packet::serialize() function.
 * The error, if any, is only observable through @c Packet::hasError().
 */
Packet Packet::fromJson(const QByteArray &serialized)
{
    Packet ret;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(serialized, &err);

    if (err.error != QJsonParseError::NoError) {
        ret.d->error = QStringLiteral("Json error: ").append(err.errorString());
        return ret;
    }

    if (!doc.isObject()) {
        ret.d->error = QStringLiteral("Document is not object");
        return ret;
    }

    *(ret.d) = doc.object();

    int typeInt = 0;
    if (!ret.d->contains(QStringLiteral("type"))) {
        ret.d->error = QStringLiteral("'type' is non-existent");
        return ret;
    }
    if (!ret.d->value(QStringLiteral("type")).isDouble()) {
        ret.d->error = QStringLiteral("'type' is not number");
        return ret;
    }
    if (!toIntegral(ret.d->value(QStringLiteral("type")), &typeInt)) {
        ret.d->error = QStringLiteral("'type' is not an integer");
        return ret;
    }
    if (!isPacketTypeValid(typeInt)) {
        ret.d->error = QStringLiteral("'type' is out of range");
        return ret;
    }

    int requestIdInt = 0;
    if (!ret.d->contains(QStringLiteral("requestId"))) {
        ret.d->error = QStringLiteral("'requestId' is non-existent");
        return ret;
    }
    if (!ret.d->value(QStringLiteral("requestId")).isDouble()) {
        ret.d->error = QStringLiteral("'requestId' is not number");
        return ret;
    }
    if (!toIntegral(ret.d->value(QStringLiteral("requestId")), &requestIdInt)) {
        ret.d->error = QStringLiteral("'requestId' is not an integer");
        return ret;
    }
    if (!isRequestIdValid(requestIdInt)) {
        ret.d->error = QStringLiteral("'requestId' is out of range");
        return ret;
    }

    int notifyIdInt = 0;
    if (!ret.d->contains(QStringLiteral("notifyId"))) {
        ret.d->error = QStringLiteral("'notifyId' is non-existent");
        return ret;
    }
    if (!ret.d->value(QStringLiteral("notifyId")).isDouble()) {
        ret.d->error = QStringLiteral("'notifyId' is not number");
        return ret;
    }
    if (!toIntegral(ret.d->value(QStringLiteral("notifyId")), &notifyIdInt)) {
        ret.d->error = QStringLiteral("'notifyId' is not an integer");
        return ret;
    }
    if (!isNotifyIdValid(notifyIdInt)) {
        ret.d->error = QStringLiteral("'notifyId' is out of range");
        return ret;
    }

    if (!ret.d->contains(QStringLiteral("value"))) {
        ret.d->error = QStringLiteral("'value' is non-existent");
        return ret;
    }

    const auto type = static_cast<Protocol::PacketType>(typeInt);
    const auto requestId = static_cast<Protocol::RequestId>(requestIdInt);
    const auto notifyId = static_cast<Protocol::NotifyId>(notifyIdInt);

    if (type == Protocol::TypeRequest || type == Protocol::TypeReply) {
        if (requestId == Protocol::RequestInvalid) {
            ret.d->error = QStringLiteral("'requestId' is invalid for a request/reply packet");
            return ret;
        }
        if (notifyId != Protocol::NotifyInvalid) {
            ret.d->error = QStringLiteral("'notifyId' should be invalid for a request/reply packet");
            return ret;
        }
    } else if (type == Protocol::TypeNotify) {
        if (notifyId == Protocol::NotifyInvalid) {
            ret.d->error = QStringLiteral("'notifyId' is invalid for a notify packet");
            return ret;
        }
        if (requestId != Protocol::RequestInvalid) {
            ret.d->error = QStringLiteral("'requestId' should be invalid for a notify packet");
            return ret;
        }
    } else {
        if (requestId != Protocol::RequestInvalid) {
            ret.d->error = QStringLiteral("'requestId' should be invalid for an invalid packet");
            return ret;
        }
        if (notifyId != Protocol::NotifyInvalid) {
            ret.d->error = QStringLiteral("'notifyId' should be invalid for an invalid packet");
            return ret;
        }
    }

    return ret;
}

#ifndef DOXYGEN
} // namespace v0
#endif

} // namespace QMdmmCore
