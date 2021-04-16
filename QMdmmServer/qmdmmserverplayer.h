// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSERVERPLAYER_H
#define QMDMMSERVERPLAYER_H

#include "qmdmmserverglobal.h"
#include <QMdmmCore/QMdmmPlayer>
#include <QMdmmCore/QMdmmProtocol>
#include <json/json.h>
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

    void request(QMdmmProtocol::QMdmmRequestId requestId, const Json::Value &requestData);
    void notify(QMdmmProtocol::QMdmmNotifyId notifyId, const Json::Value &notifyData);

    int connectionId() const;

private:
    QMDMM_D(QMdmmServerPlayer)
};

#endif // QMDMMSERVERPLAYER_H
