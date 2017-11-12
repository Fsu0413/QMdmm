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

    // This request MUST BLOCK room thread, should be done in QMdmmSocket
    // TODO: make it private
    void request(int requestId, const string &requestData);
    void notify(int notifyId, const string &notifyData);

    int connectId() const;

private:
    QMDMM_D(QMdmmServerPlayer)
};

#endif // QMDMMSERVERPLAYER_H
