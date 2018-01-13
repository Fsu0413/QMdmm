#ifndef QMDMMSOCKET_H
#define QMDMMSOCKET_H

#include "qmdmmserverglobal.h"
#include <QMdmmCore/QMdmmProtocol>
#include <string>

using std::string;
class QMdmmServer;
struct QMdmmServerSocketPrivate;

class QMDMMSERVER_EXPORT QMdmmServerSocket
{
public:
    QMdmmServerSocket();
    virtual ~QMdmmServerSocket();

    void setServer(QMdmmServer *server);
    QMdmmServer *getServer() const;

    void request(QMdmmProtocol::QMdmmRequestId requestId, const string &requestData);
    void replyed(QMdmmProtocol::QMdmmRequestId requestId, const string &replyData);
    void notify(QMdmmProtocol::QMdmmNotifyId notifyId, const string &notifyData);
    void notified(QMdmmProtocol::QMdmmNotifyId notifyId, const string &notifyData);

protected:
    virtual bool sendRequest(QMdmmProtocol::QMdmmRequestId requestId, const string &requestData) = 0;
    virtual bool sendNotify(QMdmmProtocol::QMdmmNotifyId notifyId, const string &notifyData) = 0;

private:
    QMDMM_D(QMdmmServerSocket)
    QMDMM_DISABLE_COPY(QMdmmServerSocket)
};

#endif // QMDMMSOCKET_H
