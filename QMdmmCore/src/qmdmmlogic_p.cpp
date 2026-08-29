// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmlogic_p.h"
#include "qmdmmlogic.h"

#include "qmdmmplayer.h"
#include "qmdmmroom.h"

#include <QHash>

namespace QMdmmCore {

namespace p {

LogicP::LogicP(const LogicConfiguration &logicConfiguration, Logic *q)
    : q(q)
    , room(new Room(logicConfiguration, q))
    , state(Logic::BeforeRoundStart)
    , currentStrivingActionOrder(0)
    , currentActionOrder(0)
{
}

bool LogicP::actionFeasible(const QString &fromPlayer, Data::Action action, const QString &toPlayer, int toPlace) const
{
    const Player *from = room->player(fromPlayer);
    switch (action) {
    case Data::DoNothing: {
        return from->alive();
    }
    case Data::BuyKnife: {
        return from->canBuyKnife();
    }
    case Data::BuyHorse: {
        return from->canBuyHorse();
    }
    case Data::Slash: {
        const Player *to = room->player(toPlayer);
        return from->canSlash(to);
    }
    case Data::Kick: {
        const Player *to = room->player(toPlayer);
        return from->canKick(to);
    }
    case Data::Move: {
        return from->canMove(toPlace);
    }
    case Data::LetMove: {
        const Player *to = room->player(toPlayer);
        return from->canLetMove(to, toPlace);
    }
    default:
        break;
    }

    return false;
}

// NOLINTNEXTLINE(readability-make-member-function-const): Action is ought not to be const
bool LogicP::applyAction(const QString &fromPlayer, Data::Action action, const QString &toPlayer, int toPlace)
{
    emit q->actionResult(fromPlayer, action, toPlayer, toPlace, Logic::QPrivateSignal());

    Player *from = room->player(fromPlayer);
    switch (action) {
    case Data::DoNothing: {
        return from->doNothing();
    }
    case Data::BuyKnife: {
        return from->buyKnife();
    }
    case Data::BuyHorse: {
        return from->buyHorse();
    }
    case Data::Slash: {
        Player *to = room->player(toPlayer);
        return from->slash(to);
    }
    case Data::Kick: {
        Player *to = room->player(toPlayer);
        return from->kick(to);
    }
    case Data::Move: {
        return from->move(toPlace);
    }
    case Data::LetMove: {
        Player *to = room->player(toPlayer);
        return from->letMove(to, toPlace);
    }
    default:
        break;
    }

    return false;
}

void LogicP::startSscForAction()
{
    sscForActionReplies.clear();
    sscForActionWinners.clear();
    state = Logic::SscForAction;
    emit q->requestSscForAction(room->alivePlayerNames(), Logic::QPrivateSignal());
}

void LogicP::sscForAction()
{
    if (sscForActionReplies.count() == room->alivePlayersCount()) {
        emit q->sscResult(sscForActionReplies, Logic::QPrivateSignal());
        sscForActionWinners = Data::stoneScissorsClothWinners(sscForActionReplies);
        if (sscForActionWinners.isEmpty()) {
            // restart due to tie
            startSscForAction();
        } else {
            confirmedActionOrders.clear();
            actionOrderYieldedPlayers.clear();
            startActionOrder();
        }
    }
}

void LogicP::startActionOrder()
{
    QHash<QString, int> remainingActionCount;
    QList<int> remainingActionOrders;

    int i = 1;
    foreach (const QString &player, sscForActionWinners) {
        remainingActionCount[player] = remainingActionCount.value(player, 0) + 1;
        remainingActionOrders << (i++);
    }
    for (QHash<int, QString>::const_iterator it = confirmedActionOrders.constBegin(); it != confirmedActionOrders.constEnd(); ++it) {
        --remainingActionCount[it.value()];
        remainingActionOrders.removeAll(it.key());
    }

    for (QHash<QString, int>::iterator it = remainingActionCount.begin(); it != remainingActionCount.end();) {
        if (it.value() == 0)
            it = remainingActionCount.erase(it);
        else
            ++it;
    }

    // Players who yielded (accepted whatever order is left) no longer compete;
    // they are excluded from the next request round and assigned automatically.
    QHash<QString, int> strivingActionCount;
    for (QHash<QString, int>::const_iterator it = remainingActionCount.constBegin(); it != remainingActionCount.constEnd(); ++it) {
        if (!actionOrderYieldedPlayers.contains(it.key()))
            strivingActionCount.insert(it.key(), it.value());
    }

    if (strivingActionCount.count() <= 1) {
        // At most one player is still competing, so every remaining order can be
        // assigned automatically. Hand the leftover orders (ascending) to whoever
        // still has unconfirmed picks — yielders and, if any, a single conflict
        // loser — in name order for determinism. Yielders accept whatever order
        // they get, so the exact pairing carries no semantic weight.
        QStringList remainingPlayers = remainingActionCount.keys();
        remainingPlayers.sort();
        int orderIndex = 0;
        foreach (const QString &player, remainingPlayers) {
            const int count = remainingActionCount.value(player);
            for (int k = 0; k < count; ++k)
                confirmedActionOrders[remainingActionOrders[orderIndex++]] = player;
        }

        emit q->actionOrderResult(confirmedActionOrders, Logic::QPrivateSignal());
        currentActionOrder = 0;
        startAction();
    } else {
        desiredActionOrders.clear();
        actionOrderRemainingSelections.clear();
        for (QHash<QString, int>::const_iterator it = strivingActionCount.constBegin(); it != strivingActionCount.constEnd(); ++it)
            actionOrderRemainingSelections.insert(it.key(), it.value());
        state = Logic::ActionOrder;
        for (QHash<QString, int>::const_iterator it = strivingActionCount.constBegin(); it != strivingActionCount.constEnd(); ++it)
            emit q->requestActionOrder(it.key(), remainingActionOrders, (int)(sscForActionWinners.length()), it.value(), Logic::QPrivateSignal());
    }
}

void LogicP::actionOrder()
{
    if (actionOrderRemainingSelections.isEmpty())
        startSscForActionOrder();
}

void LogicP::startSscForActionOrder()
{
    QList<int> orders = desiredActionOrders.uniqueKeys();
    foreach (int order, orders) {
        if (desiredActionOrders.count(order) == 1) {
            confirmedActionOrders[order] = desiredActionOrders.value(order);
            desiredActionOrders.remove(order);
        }
    }

    if (desiredActionOrders.isEmpty()) {
        startActionOrder();
    } else {
        currentStrivingActionOrder = 0;
        for (int i = 1; i <= sscForActionWinners.length(); ++i) {
            if (desiredActionOrders.constFind(i) != desiredActionOrders.constEnd()) {
                currentStrivingActionOrder = i;
                break;
            }
        }

        Q_ASSERT(currentStrivingActionOrder != 0);
        QStringList striving = desiredActionOrders.values(currentStrivingActionOrder);
        sscForActionOrderReplies.clear();
        state = Logic::SscForActionOrder;
        emit q->requestSscForActionOrder(striving, currentStrivingActionOrder, Logic::QPrivateSignal());
    }
}

void LogicP::sscForActionOrder()
{
    if (QStringList striving = desiredActionOrders.values(currentStrivingActionOrder); sscForActionOrderReplies.count() == striving.count()) {
        emit q->sscResult(sscForActionOrderReplies, Logic::QPrivateSignal());
        if (QStringList sscForActionOrderWinners = Data::stoneScissorsClothWinners(sscForActionOrderReplies); !sscForActionOrderWinners.isEmpty()) {
            foreach (const QString &winner, sscForActionOrderWinners)
                striving.removeAll(winner);
            foreach (const QString &loser, striving)
                desiredActionOrders.remove(currentStrivingActionOrder, loser);
        }
        startSscForActionOrder();
    }
}

void LogicP::startAction()
{
    if (!room->isRoundOver()) {
        while (++currentActionOrder <= sscForActionWinners.length()) {
            QString currentPlayer = confirmedActionOrders.value(currentActionOrder);
            Player *p = room->player(currentPlayer);
            if (p->alive()) {
                state = Logic::Action;
                emit q->requestAction(currentPlayer, currentActionOrder, Logic::QPrivateSignal());
                return;
            }
        }

        startSscForAction();
    } else {
        emit q->roundOver(Logic::QPrivateSignal());
        startUpgrade();
    }
}

void LogicP::startUpgrade()
{
    if (room->isRoundOver()) {
        upgrades.clear();
        foreach (const Player *p, room->players()) {
            if (p->upgradePoint() > 0) {
                state = Logic::Upgrade;
                emit q->requestUpgrade(p->objectName(), p->upgradePoint(), Logic::QPrivateSignal());
            }
        }
    } else {
        // ??
    }
}

// NOLINTNEXTLINE(readability-make-member-function-const, readability-function-cognitive-complexity)
void LogicP::upgrade()
{
    if (room->isRoundOver()) {
        int n = 0;
        foreach (const Player *p, room->players()) {
            if (p->upgradePoint() > 0)
                ++n;
        }

        if (upgrades.count() == n) {
            for (QHash<QString, QList<Data::UpgradeItem>>::const_iterator it = upgrades.constBegin(); it != upgrades.constEnd(); ++it) {
                Player *up = room->player(it.key());
                const QList<Data::UpgradeItem> &items = it.value();
                foreach (Data::UpgradeItem item, items) {
                    bool success = false;
                    switch (item) {
                    case Data::UpgradeKnife:
                        success = up->upgradeKnife();
                        break;
                    case Data::UpgradeHorse:
                        success = up->upgradeHorse();
                        break;
                    case Data::UpgradeMaxHp:
                        success = up->upgradeMaxHp();
                        break;
                    default:
                        break;
                    }
                    Q_ASSERT(success);
                    Q_UNUSED(success);
                }
            }

            state = Logic::BeforeRoundStart;

            if (QStringList winners; room->isGameOver(&winners)) {
                room->resetUpgrades();
                emit q->gameOver(winners, Logic::QPrivateSignal());
            } else {
                emit q->upgradeResult(upgrades, Logic::QPrivateSignal());
            }
        }
    }
}

} // namespace p

} // namespace QMdmmCore
