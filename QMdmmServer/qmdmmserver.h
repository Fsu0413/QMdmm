// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSERVER_H
#define QMDMMSERVER_H

#include "qmdmmserverglobal.h"
#include <QMdmmCore/QMdmmProtocol>
#include <json/json.h>
#include <string>

struct QMdmmServerPrivate;
class QMdmmServerRoom;
class QMdmmServerSocket;

class QMDMMSERVER_EXPORT QMdmmServer
{
public:
    QMdmmServer();
    virtual ~QMdmmServer();

    virtual bool prepare() = 0;

    virtual bool listen() = 0;
    virtual bool isListening() const = 0;

    virtual void exec() = 0;

    void socketConnected(QMdmmServerSocket *socket);
    void socketDisconnected(QMdmmServerSocket *socket);

    void addPlayer(QMdmmServerSocket *socket, const std::string &playerName);
    bool reconnectPlayer(QMdmmServerSocket *socket, const std::string &playerName, int connectionId);

    // MUST BE called in Server thread
    void notifyServer(QMdmmServerSocket *socket, QMdmmProtocol::QMdmmNotifyId notifyId, const Json::Value &notifyData);
    void replyToServer(QMdmmServerSocket *socket, QMdmmProtocol::QMdmmRequestId requestId, const Json::Value &replyData);

private:
    QMdmmServerPrivate *const d;
    QMDMM_DISABLE_COPY(QMdmmServer)
};

#endif // QMDMMSERVER_H
