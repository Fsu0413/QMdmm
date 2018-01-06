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

    QMdmmServerRoom *room();
    const QMdmmServerRoom *room() const;

    virtual bool prepare() = 0;

    virtual bool listen() = 0;
    virtual bool isListening() const = 0;

    virtual void exec() = 0;

    void socketConnected(QMdmmServerSocket *socket);
    void socketDisconnected(QMdmmServerSocket *socket);

    void addPlayer(QMdmmServerSocket *socket, const string &playerName);
    bool reconnectPlayer(QMdmmServerSocket *socket, int connectId, const string &playerName);

    // MUST BE called in Room thread, synchornized it to Server thread by QMdmmSocket.
    // Do not return until Client replied or timeout occurred.
    bool requestSocket(QMdmmServerSocket *socket, QMdmmProtocol::QMdmmRequestId requestId, const string &requestData, string &replyData);

    // called in either Room thread or Server thread. If called in Room thread, synchornized it to Server thread by QMdmmSocket.
    void notifySocket(QMdmmServerSocket *socket, QMdmmProtocol::QMdmmNotifyId notifyId, const string &notifyData);

    // MUST BE called in Server thread
    void notifyServer(QMdmmServerSocket *socket, QMdmmProtocol::QMdmmNotifyId notifyId, const string &notifyData);

private:
    QMDMM_D(QMdmmServer)
    QMDMM_DISABLE_COPY(QMdmmServer)
};

#endif // QMDMMSERVER_H
