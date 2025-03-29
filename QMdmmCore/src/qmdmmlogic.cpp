// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmlogic.h"
#include "qmdmmlogic_p.h"

#include "qmdmmplayer.h"
#include "qmdmmroom.h"

#include <QHash>
#include <QJsonObject>
#include <QJsonValue>
#include <QMultiHash>

/**
 * @file qmdmmlogic.h
 * @brief This is the file where MDMM Game logic is defined.
 */

namespace QMdmmCore {
#ifndef DOXYGEN
namespace v0 {
#else
inline namespace v1 {
#endif

/**
 * @class Logic
 * @brief The MDMM Game logic.
 *
 * This is the place where logic is run.
 * The logic is basically a state machine, where the states are changed based on the game process.
 *
 * A typical MDMM game run is like following: (ref. @c Logic::State)
 *
 * <table>
 * <thead>
 * <tr>
 * <th>State
 * <th>Definition
 * <th>Accepted Slots
 * <th>Notes
 * </tr>
 * </thead>
 * <tbody>
 * <tr>
 * <td>@c Logic::BeforeRoundStart
 * <td>The game / round has not yet started yet. Typically due to waiting for preparation of agents.
 * <td>@c Logic::addPlayer() @c Logic::removePlayer() @c Logic::roundStart()
 * <td>When @c Logic::addPlayer() or @c Logic::removePlayer() is called, all the upgrades are cleared.
 * </tr>
 * <tr>
 * <td>@c Logic::SscForAction
 * <td>Stone-Scissors-Cloth is requested for actions. This is always the first request per action time.
 * <td>@c Logic::sscReply()
 * <td>@c Logic::requestSscForAction() is emitted when this state enters, @c Logic::sscResult() is emitted when this state exits to report the result.<br />
 *     If result is a tie @c Logic::SscForAction re-enters.<br />
 *     If there is only one winner, he / she gets all the action orders and @c Logic::Action enters.<br />
 *     Else @c Logic::ActionOrder enters.
 * </tr>
 * <tr>
 * <td>@c Logic::ActionOrder
 * <td>If multiple winners has determined in the @c Logic::SscForAction state, then in this action time the winners should determine the action order.
 * <td>@c Logic::actionOrderReply()
 * <td>@c Logic::requestActionOrder() is emitted when this state enters.<br />
 *     If multiple agents chose same action order, @c Logic::SscForActionOrder enters.<br />
 *     Else the actions are got by the choosers, @c Logic::actionOrderResult() is emitted and @c Logic::Action enters.
 * </tr>
 * <tr>
 * <td>@c Logic::SscForActionOrder
 * <td>If multiple winners chose same action order, request Stone-Scissors-Cloth to determine who can actually get the action order.
 * <td>@c Logic::sscReply()
 * <td>@c Logic::requestSscForActionOrder() is emitted when this state enters, @c Logic::sscResult() is emitted when this state exits to report the result.<br />
 *     If result is a tie @c Logic::SscForActionOrder re-enters.<br />
 *     If the result is not a tie, but there are multiple winners, @c Logic::SscForActionOrder re-enters with only winners are requested.<br />
 *     Else the action order is got by the winner. Then if there are still undetermined action orders, @c Logic::ActionOrder enters, else @c Logic::actionOrderResult() is emitted and @c Logic::Action enters.
 * </tr>
 * <tr>
 * <td>@c Logic::Action
 * <td>Request action then apply the action.
 * <td>@c Logic::actionReply()
 * <td>When a player die due to an action before his action order, then his action is skipped.<br />
 *     @c Logic::requestAction() is emitted when this state enters, @c Logic::actionResult() is emitted then action is applied when this state exits.<br />
 *     If there is less than 1 player alive after an action, the remained actions are discarded, @c Logic::roundOver() is emitted and round overs.<br />
 * </tr>
 * <tr>
 * <td>@c Logic::Upgrade
 * <td>Request upgrade then apply the upgrade.
 * <td>@c Logic::upgradeReply()
 * <td>@c Logic::requestUpgrade() is emitted when this state enters, @c Logic::upgradeResult() is emitted when this state exits to report the result.<br />
 *     When all upgradable properties reaches maximum value, @c Logic::gameOver() is emitted and game overs.
 * </tr>
 * </tbody>
 * </table>
 */

/**
 * @enum Logic::State
 * @brief The state of the current game
 *
 * @sa @c QMdmmLogic
 */

/**
 * @var Logic::State Logic::BeforeRoundStart
 * @brief The game / round has not yet started yet. Typically due to waiting for preparation of agents.
 *
 * @var Logic::State Logic::SscForAction
 * @brief Stone-Scissors-Cloth is requested for actions. This is always the first request per action time.
 *
 * @var Logic::State Logic::ActionOrder
 * @brief If multiple winners has determined in the @c Logic::SscForAction state, then in this action time the winners should determine the action order.
 *
 * @var Logic::State Logic::SscForActionOrder
 * @brief If multiple winners chose same action order, request Stone-Scissors-Cloth to determine who can actually get the action order.
 *
 * @var Logic::State Logic::Action
 * @brief Request action then apply the action.
 *
 * @var Logic::State Logic::Upgrade
 * @brief Request upgrade then apply the upgrade.
 */

/**
 * @brief ctor.
 * @param logicConfiguration The configuration of this logic. Usually initialized by Server.
 * @param parent QObject parent.
 */
Logic::Logic(const LogicConfiguration &logicConfiguration, QObject *parent)
    : QObject(parent)
    , d(std::make_unique<p::LogicP>(logicConfiguration, this))
{
}

/**
 * @brief dtor.
 */
Logic::~Logic() = default;

/**
 * @brief Return current state of the logic.
 * @return the current state.
 */
Logic::State Logic::state() const noexcept
{
    return d->state;
}

/**
 * @brief Add a player to the logic
 * @param playerName the internal name of the player
 * @return true if state matches and operation succeeded (or there are no action), otherwise false
 *
 * Can be only called from @c Logic::BeforeRoundStart state.
 */
bool Logic::addPlayer(const QString &playerName)
{
    if (d->state == BeforeRoundStart) {
        if (!d->players.contains(playerName)) {
            if (d->room->addPlayer(playerName) != nullptr) {
                d->players << playerName;
                d->room->resetUpgrades();

                return true;
            }

            Q_UNREACHABLE();
        }
    }

    return false;
}

/**
 * @brief Remove a player from the logic
 * @param playerName the internal name of the player
 * @return true if state matches and operation succeeded (or there are no action), otherwise false
 *
 * Can be only called from @c Logic::BeforeRoundStart state.
 */
bool Logic::removePlayer(const QString &playerName)
{
    if (d->state == BeforeRoundStart) {
        if (d->players.contains(playerName)) {
            if (d->room->removePlayer(playerName)) {
                d->players.removeAll(playerName);
                d->room->resetUpgrades();

                return true;
            }

            Q_UNREACHABLE();
        }
    }

    return false;
}

/**
 * @brief Round start
 * @return true if state matches and operation succeeded (or there are no action), otherwise false
 *
 * Can be only called from @c Logic::BeforeRoundStart state.
 */
bool Logic::roundStart()
{
    if (d->state == BeforeRoundStart) {
        // a game must be started for player number >= 2
        if (d->players.length() >= 2) {
            d->room->prepareForRoundStart();
            d->startSscForAction();

            return true;
        }
    }

    return false;
}

/**
 * @brief Receive Stone-Scissors-Cloth result
 * @param playerName the internal name of the player
 * @param ssc the Stone-Scissors-Cloth of the player
 * @return true if state matches and operation succeeded (or there are no action), otherwise false
 *
 * Can be only called from @c Logic::SscForAction or @c Logic::SscForActionOrder state.
 */
bool Logic::sscReply(const QString &playerName, Data::StoneScissorsCloth ssc)
{
    if (d->state == SscForAction) {
        d->sscForActionReplies.insert(playerName, ssc);
        d->sscForAction();

        return true;
    }

    if (d->state == SscForActionOrder) {
        d->sscForActionOrderReplies.insert(playerName, ssc);
        d->sscForActionOrder();

        return true;
    }

    return false;
}

/**
 * @brief Receive the desired action order result
 * @param playerName the internal name of the player
 * @param desiredOrder the desired action order of the player
 * @return true if state matches and operation succeeded (or there are no action), otherwise false
 *
 * Can be only called from @c Logic::ActionOrder state.
 */
bool Logic::actionOrderReply(const QString &playerName, const QList<int> &desiredOrder)
{
    if (d->state == ActionOrder) {
        foreach (int order, desiredOrder)
            d->desiredActionOrders.insert(order, playerName);
        d->actionOrder();

        return true;
    }

    return false;
}

/**
 * @brief Receive the action performed by a player
 * @param playerName the internal name of the player
 * @param action the action to do by the player
 * @param toPlayer the internal name of the target player
 * @param toPlace the target place
 * @return true if state matches and operation succeeded (or there are no action), otherwise false
 *
 * Can be only called from @c Logic::Action state.
 */
bool Logic::actionReply(const QString &playerName, Data::Action action, const QString &toPlayer, int toPlace)
{
    if (d->state == Action) {
        if (d->actionFeasible(playerName, action, toPlayer, toPlace)) {
            d->applyAction(playerName, action, toPlayer, toPlace);
            d->startAction();

            return true;
        }
    }

    return false;
}

/**
 * @brief Receive the upgrade items of a player
 * @param playerName the internal name of the player
 * @param items the upgrade items
 * @return true if state matches and operation succeeded (or there are no action), otherwise false
 *
 * Can be only called from @c Logic::Upgrade state.
 */
bool Logic::upgradeReply(const QString &playerName, const QList<Data::UpgradeItem> &items)
{
    if (d->state == Upgrade) {
        d->upgrades.insert(playerName, items);
        d->upgrade();

        return true;
    }

    return false;
}

/**
 * @fn Logic::requestSscForAction(const QStringList &playerNames, QPrivateSignal)
 * @brief emits when Stone-Scissors-Cloth is requested for actions
 * @param playerNames the requested player names, without dead players
 *
 * Can be only emitted in @c Logic::SscForAction state.
 */

/**
 * @fn Logic::sscResult(const QHash<QString, QMdmmData::StoneScissorsCloth> &replies, QPrivateSignal)
 * @brief emits when Stone-Scissors-Cloth is all replied
 * @param replies the replies of Stone-Scissors-Cloth (key = internal name of player, value = Stone-Scissors-Cloth)
 *
 * Can be only emitted in @c Logic::SscForAction and @c Logic::SscForActionOrder state.
 */

/**
 * @fn Logic::requestActionOrder(const QString &playerName, const QList<int> &availableOrders, int maximumOrderNum, int selections, QPrivateSignal)
 * @brief emits when desired action orders are requested
 * @param playerName one of the requested player names
 * @param availableOrders the available (remained) orders
 * @param maximumOrderNum total number of action orders
 * @param selections remained count of selections
 *
 * Can be only emitted in @c Logic::ActionOrder state.
 */

/**
 * @fn Logic::actionOrderResult(const QHash<int, QString> &result, QPrivateSignal)
 * @brief emits when action order is confirmed
 * @param result the result of action orders. (key = action order, value = internal name of player)
 *
 * Can be only emitted in @c Logic::ActionOrder state.
 */

/**
 * @fn Logic::requestSscForActionOrder(const QStringList &playerNames, int strivedOrder, QPrivateSignal)
 * @brief emits when Stone-Scissors-Cloth is requested for striving for a specific action order
 * @param playerNames the requested player names with same desired action order
 * @param strivedOrder the strived action order
 *
 * Can be only emitted in @c Logic::SscForActionOrder state.
 */

/**
 * @fn Logic::requestAction(const QString &playerName, int actionOrder, QPrivateSignal)
 * @brief emits when a action should be made
 * @param playerName the requested player name
 * @param actionOrder current action order
 *
 * Can be only emitted in @c Logic::Action state.
 */

/**
 * @fn Logic::actionResult(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPlace, QPrivateSignal)
 * @brief emits when action is confirmed for current action order
 * @param playerName the requested player name
 * @param action the action to be made
 * @param toPlayer the target player name
 * @param toPlace the target place
 *
 * Can be only emitted in @c Logic::Action state.
 */

/**
 * @fn Logic::roundOver(QPrivateSignal)
 * @brief emits when round is over
 *
 * Can be only emitted in @c Logic::Action state.
 */

/**
 * @fn Logic::requestUpgrade(const QString &playerName, int upgradePoint, QPrivateSignal)
 * @brief emits when round overs and there is upgrade point remaining for a player
 * @param playerName the requested player
 * @param upgradePoint the remaining point for upgrading
 *
 * Can be only emitted in @c Logic::Upgrade state.
 */

/**
 * @fn Logic::upgradeResult(const QHash<QString, QList<QMdmmData::UpgradeItem>> &upgrades, QPrivateSignal);
 * @brief emits when upgrade has confirmed
 * @param upgrades the upgrades performed by each player (key: the internal name of player, value: list of upgrade items)
 *
 * Can be only emitted in @c Logic::Upgrade state.
 */

/**
 * @fn Logic::gameOver(const QStringList &playerNames, QPrivateSignal)
 * @brief emits when game is over
 * @param playerNames the internal names of the winners
 *
 * Can be only emitted in @c Logic::Upgrade state.
 */
} // namespace v0
} // namespace QMdmmCore
