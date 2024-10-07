// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMAGENT_P
#define QMDMMAGENT_P

#include "qmdmmagent.h"

struct QMDMMNETWORKING_PRIVATE_EXPORT QMdmmAgentPrivate final
{
    QString screenName;
    QMdmmData::AgentState state = QMdmmData::StateOffline;
};

#endif
