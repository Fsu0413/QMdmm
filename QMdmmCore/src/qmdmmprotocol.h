// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMPROTOCOL_H
#define QMDMMPROTOCOL_H

#include "qmdmmcoreglobal.h"

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

    RequestStoneScissorsCloth, // request: N/A, reply: int ssc
    RequestStriveForOperateOrder, // request: int totalOperationNum, int eachOperationNum, reply: array { int order }
    RequestOperation, // request: int currentOrder, reply: string operation, optional string toPlayer, optional int toPosition
    RequestUpdate, // request: int remaningTimes, reply: int item
};

enum NotifyId
{
    NotifyInvalid = 0,

    NotifyFromServerMask = 0x1000,
    NotifyPingClient, // int ping-id
    NotifyPongServer, // int ping-id
    NotifyVersion, // string versionNumber
    NotifyConnected, // string playerName, bool isReconnect, (to player)int connectionId
    NoitfyDisconnected, // string playerName
    NotifyStartReconnectMarshal,
    NotifyEndReconnectMarshal,

    NotifyFromLogicMask = 0x2000,
    NotifyLogicConfiguration, // broadcast, object (see QMdmmLogicConfiguration in qmdmmlogic.h)
    NotifyGameStart, // broadcast
    NotifyRoundStart, // broadcast
    NotifyStoneScissorsCloth, // broadcast, object { string playerName: int ssc } detail[playerNum]
    NotifyOperateOrder, // broadcast, array { string playerName } [totalOperationNum]
    NotifyOperation, // broadcast, string playerName, string operation, optional string toPlayer, optional int toPosition
    NotifyRoundOver, // broadcast
    NotifyUpdate, // broadcast, string playerName, int item
    NotifyGameOver, // broadcast, string winnerPlayerName
    NotifyOb, // TODO: for ob
    NotifySpoken, // broadcast, string playerName, string contents

    NotifyToServerMask = 0x4000,
    NotifyPongClient, // int ping-id
    NotifyPingServer, // int ping-id
    NotifySignIn, // string playerName, bool isReconnect, optional int connectionId
    NotifyObserve, // string observerName, string playerName

    NotifyToLogicMask = 0x8000,
    NotifySpeak, // string contents
    NotifyOperating, // TODO: for ob
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
    explicit QMdmmPacket(const QByteArray &serialized);

    [[nodiscard]] QMdmmProtocol::PacketType type() const;
    [[nodiscard]] QMdmmProtocol::RequestId requestId() const;
    [[nodiscard]] QMdmmProtocol::NotifyId notifyId() const;
    [[nodiscard]] QJsonValue value() const;

    [[nodiscard]] QByteArray serialize() const;
    bool hasError(QString *errorString = nullptr) const;

private:
    QSharedDataPointer<QMdmmPacketData> d;
};

Q_DECLARE_METATYPE(QMdmmPacket)

#endif // QMDMMPROTOCOL_H
