// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmagent.h"
#include "qmdmmagent_p.h"

QMdmmAgent::QMdmmAgent(const QString &name, QObject *parent)
    : QObject(parent)
    , d(std::make_unique<QMdmmAgentPrivate>())
{
    setObjectName(name);
}

QMdmmAgent::~QMdmmAgent() = default;

QString QMdmmAgent::screenName() const
{
    return d->screenName;
}

void QMdmmAgent::setScreenName(const QString &name)
{
    if (name != screenName()) {
        d->screenName = name;
        emit screenNameChanged(name, QPrivateSignal());
    }
}

QMdmmCore::Data::AgentState QMdmmAgent::state() const
{
    return d->state;
}

void QMdmmAgent::setState(const QMdmmCore::Data::AgentState &state)
{
    if (d->state != state) {
        d->state = state;
        emit stateChanged(state, QPrivateSignal());
    }
}
