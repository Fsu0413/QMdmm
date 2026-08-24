// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmagent.h"
#include "qmdmmagent_p.h"

/**
 * @file qmdmmagent.h
 * @brief This is the file where the networking Agent is defined.
 */

namespace QMdmmNetworking {
#ifndef DOXYGEN
namespace v0 {
#endif

/**
 * @class Agent
 * @brief The controller for one player, bridging the logic side and the operation side.
 *
 * An Agent is the unified player abstraction with two ports:
 * - the logic side (@c LogicRunner on the server, @c Client on the client) drives the
 *   agent through the requestXxx / notifyXxx methods and listens to the replyXxx /
 *   spoken / operated signals;
 * - the operation side (@c ServerConnection for the wire, or GUI / Bot for a local
 *   player) listens to the xxxRequested / xxxNotified signals and answers by calling
 *   the bare-verb methods (stoneScissorsCloth / actionOrder / action / upgrade) and
 *   speak / operate.
 *
 * Reply contract: the operation side must answer asynchronously (e.g. via
 * QTimer::singleShot or after an event-loop round-trip), never synchronously from
 * inside the xxxRequested handler. A synchronous reply re-enters the request handler
 * and violates the designed async request / reply flow.
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

// Controller interface — notifications (logic side → operation side).

/**
 * @brief Notify the player of the logic configuration.
 */
void Agent::notifyLogicConfiguration()
{
    emit logicConfigurationNotified(QPrivateSignal());
}

/**
 * @brief Notify the player that an agent's state changed.
 * @param playerName the internal name of the changed player
 * @param agentState the new state
 */
void Agent::notifyAgentStateChange(const QString &playerName, const QMdmmCore::Data::AgentState &agentState)
{
    emit agentStateChangeNotified(playerName, agentState, QPrivateSignal());
}

/**
 * @brief Notify the player that another player joined.
 * @param playerName the internal name of the added player
 * @param screenName the screen name of the added player
 * @param agentState the state of the added player
 */
void Agent::notifyPlayerAdd(const QString &playerName, const QString &screenName, const QMdmmCore::Data::AgentState &agentState)
{
    emit playerAddNotified(playerName, screenName, agentState, QPrivateSignal());
}

/**
 * @brief Notify the player that another player left.
 * @param playerName the internal name of the removed player
 */
void Agent::notifyPlayerRemove(const QString &playerName)
{
    emit playerRemoveNotified(playerName, QPrivateSignal());
}

/**
 * @brief Notify the player that the game started.
 */
void Agent::notifyGameStart()
{
    emit gameStartNotified(QPrivateSignal());
}

/**
 * @brief Notify the player that a round started.
 */
void Agent::notifyRoundStart()
{
    emit roundStartNotified(QPrivateSignal());
}

/**
 * @brief Notify the player of the stone-scissors-cloth replies of a round.
 * @param replies the stone-scissors-cloth choice of each player
 */
void Agent::notifyStoneScissorsCloth(const QHash<QString, QMdmmCore::Data::StoneScissorsCloth> &replies)
{
    emit stoneScissorsClothNotified(replies, QPrivateSignal());
}

/**
 * @brief Notify the player of the action order of a round.
 * @param result the action order, keyed by order index
 */
void Agent::notifyActionOrder(const QHash<int, QString> &result)
{
    emit actionOrderNotified(result, QPrivateSignal());
}

/**
 * @brief Notify the player of an action another player took.
 * @param playerName the internal name of the acting player
 * @param action the action taken
 * @param toPlayer the target player (for slash / kick / let-move)
 * @param toPlace the target place (for move / let-move)
 */
void Agent::notifyAction(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace)
{
    emit actionNotified(playerName, action, toPlayer, toPlace, QPrivateSignal());
}

/**
 * @brief Notify the player that the round is over.
 */
void Agent::notifyRoundOver()
{
    emit roundOverNotified(QPrivateSignal());
}

/**
 * @brief Notify the player of the upgrade choices of a round.
 * @param upgrades the upgrade items each player chose
 */
void Agent::notifyUpgrade(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades)
{
    emit upgradeNotified(upgrades, QPrivateSignal());
}

/**
 * @brief Notify the player that the game is over.
 * @param playerNames the winners
 */
void Agent::notifyGameOver(const QStringList &playerNames)
{
    emit gameOverNotified(playerNames, QPrivateSignal());
}

/**
 * @brief Notify the player that another player spoke.
 * @param playerName the internal name of the speaking player
 * @param content the spoken content
 */
void Agent::notifySpeak(const QString &playerName, const QString &content)
{
    emit speakNotified(playerName, content, QPrivateSignal());
}

/**
 * @brief Notify the player that another player operated.
 * @param playerName the internal name of the operating player
 * @param todo the operation (semantics not yet defined)
 */
void Agent::notifyOperate(const QString &playerName, const QJsonValue &todo)
{
    emit operateNotified(playerName, todo, QPrivateSignal());
}

// Controller interface — requests (logic side → operation side).

/**
 * @brief Request a Stone-Scissors-Cloth choice.
 * @param playerNames the players involved in the Stone-Scissors-Cloth
 * @param strivedOrder the action order being strived for (0 if not applicable)
 */
void Agent::requestStoneScissorsCloth(const QStringList &playerNames, int strivedOrder)
{
    emit stoneScissorsClothRequested(playerNames, strivedOrder, QPrivateSignal());
}

/**
 * @brief Request the desired action order.
 * @param remainedOrders the available (remained) orders
 * @param maximumOrder the total number of action orders
 * @param selectionNum the count of selections to make
 */
void Agent::requestActionOrder(const QList<int> &remainedOrders, int maximumOrder, int selectionNum)
{
    emit actionOrderRequested(remainedOrders, maximumOrder, selectionNum, QPrivateSignal());
}

/**
 * @brief Request an action.
 * @param currentOrder the current action order
 */
void Agent::requestAction(int currentOrder)
{
    emit actionRequested(currentOrder, QPrivateSignal());
}

/**
 * @brief Request an upgrade.
 * @param remainingTimes the remaining upgrade points
 */
void Agent::requestUpgrade(int remainingTimes)
{
    emit upgradeRequested(remainingTimes, QPrivateSignal());
}

// Controller interface — replies and player actions (operation side → logic side).

/**
 * @brief Reply with a Stone-Scissors-Cloth choice.
 * @param ssc the chosen Stone-Scissors-Cloth
 */
void Agent::stoneScissorsCloth(QMdmmCore::Data::StoneScissorsCloth ssc)
{
    emit replyStoneScissorsCloth(ssc, QPrivateSignal());
}

/**
 * @brief Reply with the desired action order.
 * @param order the desired action order
 */
void Agent::actionOrder(const QList<int> &order)
{
    emit replyActionOrder(order, QPrivateSignal());
}

/**
 * @brief Reply with an action.
 * @param act the action to make
 * @param toPlayer the target player name
 * @param toPlace the target place
 */
void Agent::action(QMdmmCore::Data::Action act, const QString &toPlayer, int toPlace)
{
    emit replyAction(act, toPlayer, toPlace, QPrivateSignal());
}

/**
 * @brief Reply with the upgrade choices.
 * @param items the list of upgrade items
 */
void Agent::upgrade(const QList<QMdmmCore::Data::UpgradeItem> &items)
{
    emit replyUpgrade(items, QPrivateSignal());
}

/**
 * @brief Give up on the current request (trigger the server's default reply).
 *
 * The operation side calls this instead of replying when it cannot answer the current request.
 * It forwards as the @c requestTimedOut signal, which the logic side (the client's connection)
 * turns into a "give up" wire reply.
 */
void Agent::requestTimeout()
{
    emit requestTimedOut(QPrivateSignal());
}

/**
 * @brief The player speaks a message.
 * @param content the content of the message
 */
void Agent::speak(const QString &content)
{
    emit spoken(content, QPrivateSignal());
}

/**
 * @brief The player performs an operation.
 * @param todo the operation data (semantics not yet defined)
 */
void Agent::operate(const QJsonValue &todo)
{
    emit operated(todo, QPrivateSignal());
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

/**
 * @fn Agent::logicConfigurationNotified(QPrivateSignal)
 * @brief emitted when the logic configuration is notified
 */

/**
 * @fn Agent::agentStateChangeNotified(const QString &playerName, const QMdmmCore::Data::AgentState &agentState, QPrivateSignal)
 * @brief emitted when an agent's state change is notified
 * @param playerName the changed player
 * @param agentState the new state
 */

/**
 * @fn Agent::playerAddNotified(const QString &playerName, const QString &screenName, const QMdmmCore::Data::AgentState &agentState, QPrivateSignal)
 * @brief emitted when a player join is notified
 * @param playerName the added player
 * @param screenName the added player's screen name
 * @param agentState the added player's state
 */

/**
 * @fn Agent::playerRemoveNotified(const QString &playerName, QPrivateSignal)
 * @brief emitted when a player leave is notified
 * @param playerName the removed player
 */

/**
 * @fn Agent::gameStartNotified(QPrivateSignal)
 * @brief emitted when the game start is notified
 */

/**
 * @fn Agent::roundStartNotified(QPrivateSignal)
 * @brief emitted when a round start is notified
 */

/**
 * @fn Agent::stoneScissorsClothNotified(const QHash<QString, QMdmmCore::Data::StoneScissorsCloth> &replies, QPrivateSignal)
 * @brief emitted when the stone-scissors-cloth replies are notified
 * @param replies the choice of each player
 */

/**
 * @fn Agent::actionOrderNotified(const QHash<int, QString> &result, QPrivateSignal)
 * @brief emitted when the action order is notified
 * @param result the action order, keyed by order index
 */

/**
 * @fn Agent::actionNotified(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace, QPrivateSignal)
 * @brief emitted when an action is notified
 * @param playerName the acting player
 * @param action the action taken
 * @param toPlayer the target player
 * @param toPlace the target place
 */

/**
 * @fn Agent::roundOverNotified(QPrivateSignal)
 * @brief emitted when the round over is notified
 */

/**
 * @fn Agent::upgradeNotified(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades, QPrivateSignal)
 * @brief emitted when the upgrade choices are notified
 * @param upgrades the upgrade items each player chose
 */

/**
 * @fn Agent::gameOverNotified(const QStringList &playerNames, QPrivateSignal)
 * @brief emitted when the game over is notified
 * @param playerNames the winners
 */

/**
 * @fn Agent::speakNotified(const QString &playerName, const QString &content, QPrivateSignal)
 * @brief emitted when another player's speech is notified
 * @param playerName the speaking player
 * @param content the spoken content
 */

/**
 * @fn Agent::operateNotified(const QString &playerName, const QJsonValue &todo, QPrivateSignal)
 * @brief emitted when another player's operation is notified
 * @param playerName the operating player
 * @param todo the operation
 */

/**
 * @fn Agent::stoneScissorsClothRequested(const QStringList &playerNames, int strivedOrder, QPrivateSignal)
 * @brief emitted when a Stone-Scissors-Cloth choice is requested
 * @param playerNames the players involved in the Stone-Scissors-Cloth
 * @param strivedOrder the action order being strived for
 */

/**
 * @fn Agent::actionOrderRequested(const QList<int> &remainedOrders, int maximumOrder, int selectionNum, QPrivateSignal)
 * @brief emitted when the desired action order is requested
 * @param remainedOrders the available (remained) orders
 * @param maximumOrder the total number of action orders
 * @param selectionNum the count of selections to make
 */

/**
 * @fn Agent::actionRequested(int currentOrder, QPrivateSignal)
 * @brief emitted when an action is requested
 * @param currentOrder the current action order
 */

/**
 * @fn Agent::upgradeRequested(int remainingTimes, QPrivateSignal)
 * @brief emitted when an upgrade is requested
 * @param remainingTimes the remaining upgrade points
 */

/**
 * @fn Agent::replyStoneScissorsCloth(QMdmmCore::Data::StoneScissorsCloth ssc, QPrivateSignal)
 * @brief emitted when a Stone-Scissors-Cloth reply is made
 * @param ssc the chosen Stone-Scissors-Cloth
 */

/**
 * @fn Agent::replyActionOrder(const QList<int> &order, QPrivateSignal)
 * @brief emitted when an action order reply is made
 * @param order the desired action order
 */

/**
 * @fn Agent::replyAction(QMdmmCore::Data::Action act, const QString &toPlayer, int toPlace, QPrivateSignal)
 * @brief emitted when an action reply is made
 * @param act the action to make
 * @param toPlayer the target player name
 * @param toPlace the target place
 */

/**
 * @fn Agent::replyUpgrade(const QList<QMdmmCore::Data::UpgradeItem> &items, QPrivateSignal)
 * @brief emitted when an upgrade reply is made
 * @param items the list of upgrade items
 */

/**
 * @fn Agent::requestTimedOut(QPrivateSignal)
 * @brief emitted when the operation side gives up on the current request
 */

/**
 * @fn Agent::spoken(const QString &content, QPrivateSignal)
 * @brief emitted when the player speaks
 * @param content the content of the message
 */

/**
 * @fn Agent::operated(const QJsonValue &todo, QPrivateSignal)
 * @brief emitted when the player operates
 * @param todo the operation data
 */

#ifndef DOXYGEN
} // namespace v0
#endif
} // namespace QMdmmNetworking
