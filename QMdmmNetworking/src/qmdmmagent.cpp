// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmagent.h"
#include "qmdmmagent_p.h"

/**
 * @file qmdmmagent.h
 * @brief This is the file where the networking Agent is defined.
 */

namespace QMdmmNetworking {
namespace v0 {

/**
 * @class Agent
 * @brief The agent representing a player on the server side.
 *
 * An Agent is the server-side counterpart of a connected client. It stores the
 * screen name and the connection state of the player, and is used by @c LogicRunner
 * to send requests to and receive replies from the client.
 */

/**
 * @property Agent::screenName
 * @brief the screen name (display name) of the agent.
 */

/**
 * @property Agent::state
 * @brief the connection state of the agent.
 *
 * This is a combination of flags defined in @c QMdmmCore::Data::AgentState (e.g.
 * whether the agent is online, is a bot, or is trusted).
 */

/**
 * @brief ctor.
 * @param name The internal name of the agent
 * @param parent QObject parent.
 */
Agent::Agent(const QString &name, QObject *parent)
    : QObject(parent)
    , d(std::make_unique<p::AgentP>())
{
    setObjectName(name);
}

/**
 * @brief dtor.
 */
Agent::~Agent() = default;

/**
 * @brief getter of property @c screenName
 * @return @c screenName
 */
QString Agent::screenName() const
{
    return d->screenName;
}

/**
 * @brief setter of property @c screenName
 * @param name @c screenName
 */
void Agent::setScreenName(const QString &name)
{
    if (name != screenName()) {
        d->screenName = name;
        emit screenNameChanged(name, QPrivateSignal());
    }
}

/**
 * @brief getter of property @c state
 * @return @c state
 */
QMdmmCore::Data::AgentState Agent::state() const
{
    return d->state;
}

/**
 * @brief setter of property @c state
 * @param state @c state
 */
void Agent::setState(const QMdmmCore::Data::AgentState &state)
{
    if (d->state != state) {
        d->state = state;
        emit stateChanged(state, QPrivateSignal());
    }
}

/**
 * @fn Agent::screenNameChanged(const QString &name, QPrivateSignal)
 * @brief notify signal for property @c screenName
 * @param name the new screen name
 */

/**
 * @fn Agent::stateChanged(QMdmmCore::Data::AgentState state, QPrivateSignal)
 * @brief notify signal for property @c state
 * @param state the new state
 */

} // namespace v0
} // namespace QMdmmNetworking
