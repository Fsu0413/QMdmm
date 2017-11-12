#ifndef QMDMMSERVERPLAYER_H
#define QMDMMSERVERPLAYER_H

#include "qmdmmserverglobal.h"
#include <QMdmmCore/QMdmmPlayer>
#include <string>

struct QMdmmServerPlayerPrivate;
class QMdmmSocket;

using std::string;

class QMDMMSERVER_EXPORT QMdmmServerPlayer : public QMdmmPlayer
{
public:
    QMdmmServerPlayer();
    ~QMdmmServerPlayer();

    void setSocket(QMdmmSocket *socket);
    QMdmmSocket *socket() const;

    void request(int requestId, const string &requestData);
    void notify(int notifyId, const string &notifyData);
    void reply(int requestId, const string &replyData);

    int connectId() const;

    // void notifyServer(int notifyId, const string &notifyData); // chat only?

private:
    QMDMM_D(QMdmmServerPlayer)
};

#endif // QMDMMSERVERPLAYER_H
