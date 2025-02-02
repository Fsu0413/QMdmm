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

/**
 * @class QMdmmLogic
 * @brief The MDMM Game logic.
 *
 * This is the place where logic is run.
 * The logic is basically a state machine, where the states are changed based on the game process.
 *
 * A typical MDMM game run is like following: (ref. @c QMdmmLogic::State)
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
 * <td>@c QMdmmLogic::BeforeRoundStart
 * <td>The game / round has not yet started yet. Typically due to waiting for preparation of agents.
 * <td>@c QMdmmLogic::addPlayer() @c QMdmmLogic::removePlayer() @c QMdmmLogic::roundStart()
 * <td>When @c QMdmmLogic::addPlayer() or @c QMdmmLogic::removePlayer() is called, all the upgrades are cleared.
 * </tr>
 * <tr>
 * <td>@c QMdmmLogic::SscForAction
 * <td>Stone-Scissors-Cloth is requested for actions. This is always the first request per action time.
 * <td>@c QMdmmLogic::sscReply()
 * <td>@c QMdmmLogic::requestSscForAction() is emitted when this state enters, @c QMdmmLogic::sscResult() is emitted when this state exits to report the result.<br />
 *     If result is a tie @c QMdmmLogic::SscForAction re-enters.<br />
 *     If there is only one winner, he / she gets all the action orders and @c QMdmmLogic::Action enters.<br />
 *     Else @c QMdmmLogic::ActionOrder enters.
 * </tr>
 * <tr>
 * <td>@c QMdmmLogic::ActionOrder
 * <td>If multiple winners has determined in the @c QMdmmLogic::SscForAction state, then in this action time the winners should determine the action order.
 * <td>@c QMdmmLogic::actionOrderReply()
 * <td>@c QMdmmLogic::requestActionOrder() is emitted when this state enters.<br />
 *     If multiple agents chose same action order, @c QMdmmLogic::SscForActionOrder enters.<br />
 *     Else the actions are got by the choosers, @c QMdmmLogic::actionOrderResult() is emitted and @c QMdmmLogic::Action enters.
 * </tr>
 * <tr>
 * <td>@c QMdmmLogic::SscForActionOrder
 * <td>If multiple winners chose same action order, request Stone-Scissors-Cloth to determine who can actually get the action order.
 * <td>@c QMdmmLogic::sscReply()
 * <td>@c QMdmmLogic::requestSscForActionOrder() is emitted when this state enters, @c QMdmmLogic::sscResult() is emitted when this state exits to report the result.<br />
 *     If result is a tie @c QMdmmLogic::SscForActionOrder re-enters.<br />
 *     If the result is not a tie, but there are multiple winners, @c QMdmmLogic::SscForActionOrder re-enters with only winners are requested.<br />
 *     Else the action order is got by the winner. Then if there are still undetermined action orders, @c QMdmmLogic::ActionOrder enters, else @c QMdmmLogic::actionOrderResult() is emitted and @c QMdmmLogic::Action enters.
 * </tr>
 * <tr>
 * <td>@c QMdmmLogic::Action
 * <td>Request action then apply the action.
 * <td>@c QMdmmLogic::actionReply()
 * <td>When a player die due to an action before his action order, then his action is skipped.<br />
 *     @c QMdmmLogic::requestAction() is emitted when this state enters, @c QMdmmLogic::actionResult() is emitted then action is applied when this state exits.<br />
 *     If there is less than 1 player alive after an action, the remained actions are discarded, @c QMdmmLogic::roundOver() is emitted and round overs.<br />
 * </tr>
 * <tr>
 * <td>@c QMdmmLogic::Upgrade
 * <td>Request upgrade then apply the upgrade.
 * <td>@c QMdmmLogic::upgradeReply()
 * <td>@c QMdmmLogic::requestUpgrade() is emitted when this state enters, @c QMdmmLogic::upgradeResult() is emitted when this state exits to report the result.<br />
 *     When all upgradable properties reaches maximum value, @c QMdmmLogic::gameOver() is emitted and game overs.
 * </tr>
 * </tbody>
 * </table>
 */

/**
 * @enum QMdmmLogic::State
 * @brief The state of the current game
 *
 * @sa @c QMdmmLogic
 */

/**
 * @var QMdmmLogic::State QMdmmLogic::BeforeRoundStart
 * @brief The game / round has not yet started yet. Typically due to waiting for preparation of agents.
 *
 * @var QMdmmLogic::State QMdmmLogic::SscForAction
 * @brief Stone-Scissors-Cloth is requested for actions. This is always the first request per action time.
 *
 * @var QMdmmLogic::State QMdmmLogic::ActionOrder
 * @brief If multiple winners has determined in the @c QMdmmLogic::SscForAction state, then in this action time the winners should determine the action order.
 *
 * @var QMdmmLogic::State QMdmmLogic::SscForActionOrder
 * @brief If multiple winners chose same action order, request Stone-Scissors-Cloth to determine who can actually get the action order.
 *
 * @var QMdmmLogic::State QMdmmLogic::Action
 * @brief Request action then apply the action.
 *
 * @var QMdmmLogic::State QMdmmLogic::Upgrade
 * @brief Request upgrade then apply the upgrade.
 */

/**
 * @brief ctor.
 * @param logicConfiguration The configuration of this logic. Usually initialized by Server.
 * @param parent QObject parent.
 */
QMdmmLogic::QMdmmLogic(const QMdmmLogicConfiguration &logicConfiguration, QObject *parent)
    : QObject(parent)
    , d(std::make_unique<QMdmmLogicPrivate>(logicConfiguration, this))
{
}

/**
 * @brief dtor.
 */
QMdmmLogic::~QMdmmLogic() = default;

/**
 * @brief Return current state of the logic.
 * @return the current state.
 */
QMdmmLogic::State QMdmmLogic::state() const noexcept
{
    return d->state;
}

/**
 * @brief Add a player to the logic
 * @param playerName the internal name of the player
 *
 * Can be only called from @c QMdmmLogic::BeforeRoundStart state.
 */
void QMdmmLogic::addPlayer(const QString &playerName)
{
    if (d->state == BeforeRoundStart) {
        if (!d->players.contains(playerName)) {
            if (d->room->addPlayer(playerName) != nullptr) {
                d->players << playerName;
                d->room->resetUpgrades();
            }
        }
    }
}

/**
 * @brief Remove a player from the logic
 * @param playerName the internal name of the player
 *
 * Can be only called from @c QMdmmLogic::BeforeRoundStart state.
 */
void QMdmmLogic::removePlayer(const QString &playerName)
{
    if (d->state == BeforeRoundStart) {
        if (d->players.contains(playerName)) {
            if (d->room->removePlayer(playerName)) {
                d->players.removeAll(playerName);
                d->room->resetUpgrades();
            }
        }
    }
}

/**
 * @brief Round start
 *
 * Can be only called from @c QMdmmLogic::BeforeRoundStart state.
 */
void QMdmmLogic::roundStart()
{
    if (d->state == BeforeRoundStart) {
        d->room->prepareForRoundStart();
        d->startSscForAction();
    }
}

/**
 * @brief Receive Stone-Scissors-Cloth result
 * @param playerName the internal name of the player
 * @param ssc the Stone-Scissors-Cloth of the player
 *
 * Can be only called from @c QMdmmLogic::SscForAction or @c QMdmmLogic::SscForActionOrder state.
 */
void QMdmmLogic::sscReply(const QString &playerName, QMdmmData::StoneScissorsCloth ssc)
{
    if (d->state == SscForAction) {
        d->sscForActionReplies.insert(playerName, ssc);
        d->sscForAction();
    } else if (d->state == SscForActionOrder) {
        d->sscForActionOrderReplies.insert(playerName, ssc);
        d->sscForActionOrder();
    }
}

/**
 * @brief Receive the desired action order result
 * @param playerName the internal name of the player
 * @param desiredOrder the desired action order of the player
 *
 * Can be only called from @c QMdmmLogic::ActionOrder state.
 */
void QMdmmLogic::actionOrderReply(const QString &playerName, const QList<int> &desiredOrder)
{
    if (d->state == ActionOrder) {
        foreach (int order, desiredOrder)
            d->desiredActionOrders.insert(order, playerName);
        d->actionOrder();
    }
}

/**
 * @brief Receive the action performed by a player
 * @param playerName the internal name of the player
 * @param action the action to do by the player
 * @param toPlayer the internal name of the target player
 * @param toPlace the target place
 *
 * Can be only called from @c QMdmmLogic::Action state.
 */
void QMdmmLogic::actionReply(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPlace)
{
    if (d->state == Action) {
        if (d->actionFeasible(playerName, action, toPlayer, toPlace)) {
            d->applyAction(playerName, action, toPlayer, toPlace);
            d->startAction();
        }
    }
}

/**
 * @brief Receive the upgrade items of a player
 * @param playerName the internal name of the player
 * @param items the upgrade items
 *
 * Can be only called from @c QMdmmLogic::Upgrade state.
 */
void QMdmmLogic::upgradeReply(const QString &playerName, const QList<QMdmmData::UpgradeItem> &items)
{
    if (d->state == Upgrade) {
        d->upgrades.insert(playerName, items);
        d->upgrade();
    }
}

/**
 * @fn QMdmmLogic::requestSscForAction(const QStringList &playerNames, QPrivateSignal)
 * @brief emits when Stone-Scissors-Cloth is requested for actions
 * @param playerNames the requested player names, without dead players
 *
 * Can be only emitted in @c QMdmmLogic::SscForAction state.
 */

/**
 * @fn QMdmmLogic::sscResult(const QHash<QString, QMdmmData::StoneScissorsCloth> &replies, QPrivateSignal)
 * @brief emits when Stone-Scissors-Cloth is all replied
 * @param replies the replies of Stone-Scissors-Cloth (key = internal name of player, value = Stone-Scissors-Cloth)
 *
 * Can be only emitted in @c QMdmmLogic::SscForAction and @c QMdmmLogic::SscForActionOrder state.
 */

/**
 * @fn QMdmmLogic::requestActionOrder(const QString &playerName, const QList<int> &availableOrders, int maximumOrderNum, int selections, QPrivateSignal)
 * @brief emits when desired action orders are requested
 * @param playerName one of the requested player names
 * @param availableOrders the available (remained) orders
 * @param maximumOrderNum total number of action orders
 * @param selections remained count of selections
 *
 * Can be only emitted in @c QMdmmLogic::ActionOrder state.
 */

/**
 * @fn QMdmmLogic::actionOrderResult(const QHash<int, QString> &result, QPrivateSignal)
 * @brief emits when action order is confirmed
 * @param result the result of action orders. (key = action order, value = internal name of player)
 *
 * Can be only emitted in @c QMdmmLogic::ActionOrder state.
 */

/**
 * @fn QMdmmLogic::requestSscForActionOrder(const QStringList &playerNames, int strivedOrder, QPrivateSignal)
 * @brief emits when Stone-Scissors-Cloth is requested for striving for a specific action order
 * @param playerNames the requested player names with same desired action order
 * @param strivedOrder the strived action order
 *
 * Can be only emitted in @c QMdmmLogic::SscForActionOrder state.
 */

/**
 * @fn QMdmmLogic::requestAction(const QString &playerName, int actionOrder, QPrivateSignal)
 * @brief emits when a action should be made
 * @param playerName the requested player name
 * @param actionOrder current action order
 *
 * Can be only emitted in @c QMdmmLogic::Action state.
 */

/**
 * @fn QMdmmLogic::actionResult(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPlace, QPrivateSignal)
 * @brief emits when action is confirmed for current action order
 * @param playerName the requested player name
 * @param action the action to be made
 * @param toPlayer the target player name
 * @param toPlace the target place
 *
 * Can be only emitted in @c QMdmmLogic::Action state.
 */

/**
 * @fn QMdmmLogic::roundOver(QPrivateSignal)
 * @brief emits when round is over
 *
 * Can be only emitted in @c QMdmmLogic::Action state.
 */

/**
 * @fn QMdmmLogic::requestUpgrade(const QString &playerName, int upgradePoint, QPrivateSignal)
 * @brief emits when round overs and there is upgrade point remaining for a player
 * @param playerName the requested player
 * @param upgradePoint the remaining point for upgrading
 *
 * Can be only emitted in @c QMdmmLogic::Upgrade state.
 */

/**
 * @fn QMdmmLogic::upgradeResult(const QHash<QString, QList<QMdmmData::UpgradeItem>> &upgrades, QPrivateSignal);
 * @brief emits when upgrade has confirmed
 * @param upgrades the upgrades performed by each player (key: the internal name of player, value: list of upgrade items)
 *
 * Can be only emitted in @c QMdmmLogic::Upgrade state.
 */

/**
 * @fn QMdmmLogic::gameOver(const QStringList &playerNames, QPrivateSignal)
 * @brief emits when game is over
 * @param playerNames the internal names of the winners
 *
 * Can be only emitted in @c QMdmmLogic::Upgrade state.
 */
