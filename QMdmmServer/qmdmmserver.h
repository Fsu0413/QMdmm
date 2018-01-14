#ifndef QMDMMSERVER_H
#define QMDMMSERVER_H

#include "qmdmmserverglobal.h"
#include <QMdmmCore/QMdmmProtocol>
#include <string>

using std::string;

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

    void addPlayer(QMdmmServerSocket *socket, const string &playerName);
    bool reconnectPlayer(QMdmmServerSocket *socket, int connectId);

    // MUST BE called in Server thread
    void notifyServer(QMdmmServerSocket *socket, QMdmmProtocol::QMdmmNotifyId notifyId, const string &notifyData);
    void replyToServer(QMdmmServerSocket *socket, QMdmmProtocol::QMdmmRequestId requestId, const string &replyData);

private:
    QMDMM_D(QMdmmServer)
    QMDMM_DISABLE_COPY(QMdmmServer)
};

#endif // QMDMMSERVER_H