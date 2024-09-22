// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSOCKET_P
#define QMDMMSOCKET_P

#include "qmdmmsocket.h"

#include <QIODevice>
#include <QObject>
#include <QPointer>

class QMDMMSERVER_EXPORT QMdmmSocketPrivate final : public QObject
{
    Q_OBJECT

public:
    QMdmmSocketPrivate(QIODevice *socket, QMdmmSocket::Type type, QMdmmSocket *p);

    QPointer<QIODevice> socket;
    QMdmmSocket *p;

    bool hasError;
    QMdmmSocket::Type type;

public slots: // NOLINT(readability-redundant-access-specifiers)
    void sendPacket(QMdmmPacket packet);

    void messageReceived();
};

#endif
