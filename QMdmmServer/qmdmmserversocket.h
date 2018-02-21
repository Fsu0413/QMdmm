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

    virtual void disconnect() = 0;

    void request(QMdmmProtocol::QMdmmRequestId requestId, const Json::Value &requestData);
    void replyed(QMdmmProtocol::QMdmmRequestId requestId, const Json::Value &replyData);
    void notify(QMdmmProtocol::QMdmmNotifyId notifyId, const Json::Value &notifyData);
    void notified(QMdmmProtocol::QMdmmNotifyId notifyId, const Json::Value &notifyData);

protected:
    virtual bool send(const string &data) = 0;
    void received(const string &data);

private:
    QMDMM_D(QMdmmServerSocket)
    QMDMM_DISABLE_COPY(QMdmmServerSocket)
};

#endif // QMDMMSOCKET_H
