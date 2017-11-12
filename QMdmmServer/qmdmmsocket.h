#ifndef QMDMMSOCKET_H
#define QMDMMSOCKET_H

#include "qmdmmserverglobal.h"
#include <QMdmmCore/QMdmmProtocol>
#include <string>

using std::string;

class QMDMMSERVER_EXPORT QMdmmSocket
{
public:
    QMdmmSocket();
    virtual ~QMdmmSocket();

    bool request(QMdmmProtocol::QMdmmRequestId requestId, const string &requestData, string &replyData);
    void notify(QMdmmProtocol::QMdmmNotifyId notifyId, const string &notifyData);
    void notified(QMdmmProtocol::QMdmmNotifyId notifyId, const string &notifyData);

    virtual bool sendPacket(/* ??? */) = 0;
};

#endif // QMDMMSOCKET_H
