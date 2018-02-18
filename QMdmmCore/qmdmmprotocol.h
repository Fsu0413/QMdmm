#ifndef QMDMMPROTOCOL_H
#define QMDMMPROTOCOL_H

#include "qmdmmcoreglobal.h"
#include <string>
using std::string;

#if 0
class QMDMMCORE_EXPORT QMdmmProtocol
#endif

namespace QMdmmProtocol {
enum QMdmmRequestId
{
    // No requests is from server, all requests are from room
    RequestInvalid = 0,

    RequestStoneScissorsCloth, // request: N/A, reply: int ssc
    RequestStriveForFirstOrLast, // request: N/A, reply: int firstOrLast
    RequestOperation, // request: N/A, reply: int operation, string targetName
    RequestUpdate, // request: N/A, reply: int item
};

enum QMdmmNotifyId
{
    NotifyInvalid = 0,

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

    NotifyToRoomMask = 0x8000,
    NotifySpeak, // string contents
    NotifyOperating, // TODO: for ob
};
} // namespace QMdmmProtocol

namespace Json {
class Value;
}

struct QMdmmPacketPrivate;

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

    QMdmmPacket(Type type, QMdmmProtocol::QMdmmRequestId requestId, const Json::Value &value);
    QMdmmPacket(QMdmmProtocol::QMdmmNotifyId notifyId, const Json::Value &value);
    QMdmmPacket(const QMdmmPacket &package);
    QMdmmPacket(const string &str);

    QMdmmPacket &operator=(const QMdmmPacket &package);

    Type type() const;
    QMdmmProtocol::QMdmmRequestId requestId() const;
    QMdmmProtocol::QMdmmNotifyId notifyId() const;
    Json::Value value() const;

    string toString() const;
    bool hasError(string *errorString = nullptr) const;

private:
    QMDMM_D(QMdmmPacket)
};

#endif // QMDMMPROTOCOL_H
