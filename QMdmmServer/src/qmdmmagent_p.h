// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMAGENT_P
#define QMDMMAGENT_P

#include "qmdmmagent.h"

#include <QMdmmProtocol>

#include <QObject>
#include <QPointer>

class QMDMMSERVER_EXPORT QMdmmAgentPrivate final : public QObject
{
    Q_OBJECT

public:
    QMdmmAgentPrivate(QMdmmAgent *a);

    void setSocket(QMdmmSocket *_socket);

    QMdmmAgent *a;
    bool ready;
    QString screenName;
    QPointer<QMdmmSocket> socket;
    bool isReconnect;

public slots: // NOLINT(readability-redundant-access-specifiers)
    void packetReceived(QMdmmPacket packet);
    void socketDisconnected();
    void socketReconnected();
};

#endif
