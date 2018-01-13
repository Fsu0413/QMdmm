#ifndef QMDMMPROTOCOL_H
#define QMDMMPROTOCOL_H

#include "qmdmmcoreglobal.h"

#if 0
class QMDMMCORE_EXPORT QMdmmProtocol
#endif

namespace QMdmmProtocol {
enum QMdmmRequestId
{
    // No requests is from server, all requests are from room

    RequestStoneScissorsCloth, // request: N/A, reply: int ssc
    RequestStriveForFirstOrLast, // request: N/A, reply: int firstOrLast
    RequestOperation, // request: N/A, reply: int operation, string targetName
    RequestUpdate, // request: N/A, reply: int item
};

enum QMdmmNotifyId
{
    NotifyFromServerMask = 0x1000,
    NotifyPingClient, // int ping-id
    NotifyPongServer, // int ping-id
    NotifySpoken, // broadcast, string playerName, string contents

    NotifyFromRoomMask = 0x2000,
    NotifyGameStart, // broadcast
    NotifyRoundStart, // broadcast
    NotifyStoneScissorsCloth, // broadcast, struct { string playerName, int ssc } detail[3]
    NotifyFirstOrLast, // broadcast, struct { string playerName, int firstOrLast } detail[2]
    NotifyOperation, // broadcast, string playerName, int operation, string targetName
    NotifyRoundOver, // broadcast
    NotifyUpdate, // broadcast, string playerName, int item

    NotifyToServerMask = 0x4000,
    NotifyPongClient, // int ping-id
    NotifyPingServer, // int ping-id
    NotifySpeak, // string contents
    NotifyOperating, // TODO: for ob

    NotifyToRoomMask = 0x8000,
};
} // namespace QMdmmProtocol

#endif // QMDMMPROTOCOL_H
