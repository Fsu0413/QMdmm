// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMPROTOCOL_H
#define QMDMMPROTOCOL_H

#include "qmdmmcoreglobal.h"

#if 0
class QMDMMCORE_EXPORT QMdmmProtocol
#endif

namespace QMdmmProtocol {
enum QMdmmRequestId
{
    // No requests is from server, all requests are from Logic
    RequestInvalid = 0,

    RequestStoneScissorsCloth, // request: N/A, reply: int ssc
    RequestStriveForOperateOrder, // request: int totalOperationNum, int eachOperationNum, reply: array { int order }
    RequestOperation, // request: int currentOrder, reply: string operation, optional string toPlayer, optional int toPosition
    RequestUpdate, // request: int remaningTimes, reply: int item
};

enum QMdmmNotifyId
{
    NotifyInvalid = 0,

    NotifyFromServerMask = 0x1000,
    NotifyPingClient, // int ping-id
    NotifyPongServer, // int ping-id
    NotifyVersion, // string versionNumber
    NotifyConnected, // string playerName, bool isReconnect, (to player)int connectionId
    NoitfyDisconnected, // string playerName

    NotifyFromLogicMask = 0x2000,
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
} // namespace QMdmmProtocol

struct QMdmmPacketPrivate;
class QJsonValue;

class QMDMMCORE_EXPORT QMdmmPacket
{
public:
    enum Type
    {
        TypeRequest,
        TypeReply,
        TypeNotify,

        TypeInvalid = -1
    };

    QMdmmPacket(Type type, QMdmmProtocol::QMdmmRequestId requestId, const QJsonValue &value);
    QMdmmPacket(QMdmmProtocol::QMdmmNotifyId notifyId, const QJsonValue &value);
    QMdmmPacket(const QMdmmPacket &package);
    explicit QMdmmPacket(const QByteArray &serialized);

    QMdmmPacket &operator=(const QMdmmPacket &package);

    [[nodiscard]] Type type() const;
    [[nodiscard]] QMdmmProtocol::QMdmmRequestId requestId() const;
    [[nodiscard]] QMdmmProtocol::QMdmmNotifyId notifyId() const;
    [[nodiscard]] QJsonValue value() const;

    [[nodiscard]] QByteArray serialize() const;
    bool hasError(QString *errorString = nullptr) const;

private:
    QMdmmPacketPrivate *const d;
};

#endif // QMDMMPROTOCOL_H
