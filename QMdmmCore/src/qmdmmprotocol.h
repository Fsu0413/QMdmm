// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMPROTOCOL_H
#define QMDMMPROTOCOL_H

#include "qmdmmcoreglobal.h"

#include <QByteArray>
#include <QJsonObject>
#include <QSharedData>

#if 0
class QMDMMCORE_EXPORT QMdmmProtocol
#endif

namespace QMdmmProtocol {

enum RequestId
{
    // No requests is from server, all requests are from Logic
    RequestInvalid = 0,

    RequestStoneScissorsCloth, // request: int strivedOrder (or 0 for action), array { string playerName } opponents, reply: int ssc
    RequestActionOrder, // request: array { int } remainedOrders, int maximumOrder, int selectionNum, reply: array { int } orders
    RequestAction, // request: int currentOrder, reply: int(Action) action, optional string toPlayer, optional int toPlace
    RequestUpdate, // request: int remaningTimes, reply: array { int } item
};

enum NotifyId
{
    NotifyInvalid = 0,

    NotifyFromServerMask = 0x1000,
    NotifyPongServer, // int ping-id
    NotifyVersion, // string versionNumber

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

enum PacketType
{
    TypeRequest,
    TypeReply,
    TypeNotify,

    TypeInvalid = -1
};
} // namespace QMdmmProtocol

// Failed to pimpl following class since it inherits QSharedData
// So put it to header file and inherit QJsonObject, in order not to affect binary compatibility when more data come in
// ATTENTION: neither of the inherited 2 classes have virtual dtor
struct QMDMMCORE_EXPORT QMdmmPacketData final : public QSharedData, public QJsonObject
{
    QMdmmPacketData();
    QMdmmPacketData(QMdmmProtocol::PacketType type, QMdmmProtocol::RequestId requestId, QMdmmProtocol::NotifyId notifyId, const QJsonValue &value);

    QMdmmPacketData(const QJsonObject &ob);
    QMdmmPacketData &operator=(const QJsonObject &ob);

    QString error;
};

class QMDMMCORE_EXPORT QMdmmPacket final
{
public:
    QMdmmPacket();
    QMdmmPacket(QMdmmProtocol::PacketType type, QMdmmProtocol::RequestId requestId, const QJsonValue &value);
    QMdmmPacket(QMdmmProtocol::NotifyId notifyId, const QJsonValue &value);

    [[nodiscard]] QMdmmProtocol::PacketType type() const;
    [[nodiscard]] QMdmmProtocol::RequestId requestId() const;
    [[nodiscard]] QMdmmProtocol::NotifyId notifyId() const;
    [[nodiscard]] QJsonValue value() const;

    [[nodiscard]] QByteArray serialize() const;
    bool hasError(QString *errorString = nullptr) const;

    static QMdmmPacket fromJson(const QByteArray &serialized, QString *errorString = nullptr);
    [[nodiscard]] inline operator QByteArray() const // NOLINT(readability-redundant-inline-specifier)
    {
        return serialize();
    }

private:
    QSharedDataPointer<QMdmmPacketData> d;
};

Q_DECLARE_METATYPE(QMdmmPacket)

#endif // QMDMMPROTOCOL_H
