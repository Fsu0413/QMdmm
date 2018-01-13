#ifndef QMDMMSERVERPLAYER_H
#define QMDMMSERVERPLAYER_H

#include "qmdmmserverglobal.h"
#include <QMdmmCore/QMdmmPlayer>
#include <QMdmmCore/QMdmmProtocol>
#include <string>

struct QMdmmServerPlayerPrivate;
class QMdmmServerSocket;

using std::string;

class QMDMMSERVER_EXPORT QMdmmServerPlayer : public QMdmmPlayer
{
public:
    QMdmmServerPlayer();
    ~QMdmmServerPlayer();

    void setSocket(QMdmmServerSocket *socket);
    QMdmmServerSocket *socket() const;

    // This request MUST BLOCK room thread, should be done in QMdmmSocket
    // TODO: make it private
    void request(QMdmmProtocol::QMdmmRequestId requestId, const string &requestData);
    void notify(QMdmmProtocol::QMdmmNotifyId notifyId, const string &notifyData);

    int connectId() const;

private:
    QMDMM_D(QMdmmServerPlayer)
};

#endif // QMDMMSERVERPLAYER_H
