#ifndef QMDMMSERVER_H
#define QMDMMSERVER_H

#include "qmdmmprotocol.h"
#include "qmdmmserverglobal.h"
#include <string>

using std::string;

struct QMdmmServerPrivate;
class QMdmmRoom;
class QMdmmSocket;

class QMDMMSERVER_EXPORT QMdmmServer
{
public:
    QMdmmServer();
    virtual ~QMdmmServer();

    QMdmmRoom *room();
    const QMdmmRoom *room() const;

    virtual bool prepare() = 0;

    virtual bool listen() = 0;
    virtual bool isListening() const = 0;

    virtual void exec() = 0;

    void socketConnected(QMdmmSocket *socket);
    void socketDisconnected(QMdmmSocket *socket);

    void addPlayer(QMdmmSocket *socket, const string &playerName);
    bool reconnectPlayer(QMdmmSocket *socket, int connectId, const string &playerName);

    void requestSocket(QMdmmSocket *socket, QMdmmProtocol::QMdmmRequestId requestId, const string &requestData);
    void notifySocket(QMdmmSocket *socket, QMdmmProtocol::QMdmmNotifyId notifyId, const string &notifyData);

    void replyToServer(QMdmmSocket *socket, QMdmmProtocol::QMdmmRequestId requestId, const string &replyData);
    void notifyServer(QMdmmSocket *socket, QMdmmProtocol::QMdmmNotifyId notifyId, const string &notifyData);

private:
    QMDMM_D(QMdmmServer)
    QMDMM_DISABLE_COPY(QMdmmServer)
};

#endif // QMDMMSERVER_H
