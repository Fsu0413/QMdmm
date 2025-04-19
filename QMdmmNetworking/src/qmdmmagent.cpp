// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmagent.h"
#include "qmdmmagent_p.h"

namespace QMdmmNetworking {
namespace v0 {

Agent::Agent(const QString &name, QObject *parent)
    : QObject(parent)
    , d(std::make_unique<p::AgentP>())
{
    setObjectName(name);
}

Agent::~Agent() = default;

QString Agent::screenName() const
{
    return d->screenName;
}

void Agent::setScreenName(const QString &name)
{
    if (name != screenName()) {
        d->screenName = name;
        emit screenNameChanged(name, QPrivateSignal());
    }
}

QMdmmCore::Data::AgentState Agent::state() const
{
    return d->state;
}

void Agent::setState(const QMdmmCore::Data::AgentState &state)
{
    if (d->state != state) {
        d->state = state;
        emit stateChanged(state, QPrivateSignal());
    }
}
} // namespace v0
} // namespace QMdmmNetworking
