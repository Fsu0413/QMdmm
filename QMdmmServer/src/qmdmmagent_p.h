// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMAGENT_P
#define QMDMMAGENT_P

#include "qmdmmagent.h"

#include <QMdmmProtocol>

#include <QObject>
#include <QPointer>

struct QMDMMSERVER_PRIVATE_EXPORT QMdmmAgentPrivate final
{
    QMdmmAgentPrivate(QMdmmAgent *a);

    QMdmmAgent *a;
    QString screenName;
    QMdmmProtocol::AgentState state;
};

#endif
