// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMCLIENT_P
#define QMDMMCLIENT_P

#include "qmdmmclient.h"
#include "qmdmmsocket.h"

#include <QLocalSocket>
#include <QPointer>
#include <QTcpSocket>

class QMDMMSERVER_EXPORT QMdmmClientPrivate final
{
public:
    QMdmmSocket *socket;

    QMdmmClientPrivate();
};

#endif
