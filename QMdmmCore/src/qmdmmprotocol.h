// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMPROTOCOL_H
#define QMDMMPROTOCOL_H

#include "qmdmmcoreglobal.h"

#include <QByteArray>
#include <QJsonObject>
#include <QSharedData>

#include <cstdint>

QMDMM_EXPORT_NAME(QMdmmProtocol)
QMDMM_EXPORT_NAME(QMdmmPacket)

namespace QMdmmCore {

#ifndef DOXYGEN
namespace v0 {
#else
inline namespace v1 {
#endif

namespace Protocol {

enum RequestId : uint8_t
{
    // No requests is from server, all requests are from Logic
    RequestInvalid = 0,

    RequestStoneScissorsCloth, // request: array { string playerName } playerNames, int strivedOrder (or 0 for action) reply: int ssc
    RequestActionOrder, // request: array { int } remainedOrders, int maximumOrder, int selectionNum, reply: array { int } orders
    RequestAction, // request: int currentOrder, reply: int(Action) action, optional string toPlayer, optional int toPlace
    RequestUpgrade, // request: int remaningTimes, reply: array { int } item
};

enum NotifyId : uint16_t
{
    NotifyInvalid = 0,

    NotifyFromServerMask = 0x1000,
    NotifyPongServer, // int ping-id
    NotifyVersion, // string versionNumber, int protocolVersion

    NotifyFromAgentMask = 0x2000,
    NotifyLogicConfiguration, // broadcast, object (see QMdmmLogicConfiguration in qmdmmlogic.h)
    NotifyAgentStateChanged, // string playerName, int (AgentState) state
    NotifyPlayerAdded, // string playerName, string screenName, int(AgentState) agentState
    NotifyPlayerRemoved, // string playerName
    NotifyGameStart, // broadcast
    NotifyRoundStart, // broadcast
    NotifyStoneScissorsCloth, // broadcast, object { playerName: int ssc }
    NotifyActionOrder, // broadcast, array { string playerName } [totalActionNum]
    NotifyAction, // broadcast, string playerName, int(Action) action, optional string toPlayer, optional int toPlace
    NotifyRoundOver, // broadcast
    NotifyUpgrade, // broadcast, string playerName, int item
    NotifyGameOver, // broadcast, string winnerPlayerName
    NotifySpoken, // broadcast, string playerName, string content
    NotifyOperated, // TODO: for ob

    NotifyToServerMask = 0x4000,
    NotifyPingServer, // int ping-id
    NotifySignIn, // string playerName, string screenName, int(AgentState) agentState
    NotifyObserve, // string observerName, string playerName

    NotifyToAgentMask = 0x8000,
    NotifySpeak, // string
    NotifyOperate, // TODO: for ob
};

enum PacketType : uint8_t
{
    TypeInvalid = 0,

    TypeRequest,
    TypeReply,
    TypeNotify,
};

QMDMMCORE_EXPORT extern int version() noexcept;

} // namespace Protocol

#ifndef DOXYGEN
// Cannot pimpl following class since it inherits QSharedData
// So put it to header file and inherit QJsonObject, in order not to affect binary compatibility when more data come in
// ATTENTION: neither of the inherited 2 classes have virtual dtor

// documentation is not needed since it is purely internal to QMdmmPacket
struct QMDMMCORE_EXPORT PacketData final : public QSharedData, public QJsonObject
{
    PacketData();
    PacketData(Protocol::PacketType type, Protocol::RequestId requestId, Protocol::NotifyId notifyId, const QJsonValue &value);

    PacketData(const QJsonObject &ob) noexcept(noexcept(QJsonObject(ob)));
    PacketData &operator=(const QJsonObject &ob) noexcept(noexcept(QJsonObject::operator=(ob)));

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    QString error;
};
#endif

class QMDMMCORE_EXPORT Packet final
{
public:
    Packet();
    Packet(Protocol::PacketType type, Protocol::RequestId requestId, const QJsonValue &value);
    Packet(Protocol::NotifyId notifyId, const QJsonValue &value);

    [[nodiscard]] Protocol::PacketType type() const;
    [[nodiscard]] Protocol::RequestId requestId() const;
    [[nodiscard]] Protocol::NotifyId notifyId() const;
    [[nodiscard]] QJsonValue value() const;

    [[nodiscard]] QByteArray serialize() const;
    [[nodiscard]] operator QByteArray() const
    {
        return serialize();
    }
    bool hasError(QString *errorString = nullptr) const;

    static QMDMMCORE_EXPORT Packet fromJson(const QByteArray &serialized, QString *errorString = nullptr);

#ifndef DOXYGEN
private:
    QSharedDataPointer<PacketData> d;
#endif
};

} // namespace v0

#ifndef DOXYGEN
inline namespace v1 {
using v0::Packet;
using v0::PacketData;
namespace Protocol = v0::Protocol;
} // namespace v1
#endif

} // namespace QMdmmCore

Q_DECLARE_METATYPE(QMdmmCore::Packet)

#endif // QMDMMPROTOCOL_H
