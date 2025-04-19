// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMAGENT_P
#define QMDMMAGENT_P

#include "qmdmmagent.h"
namespace QMdmmNetworking {
namespace p {

struct QMDMMNETWORKING_PRIVATE_EXPORT AgentP final
{
    QString screenName;
    QMdmmCore::Data::AgentState state = QMdmmCore::Data::StateOffline;
};

} // namespace p
} // namespace QMdmmNetworking
#endif
