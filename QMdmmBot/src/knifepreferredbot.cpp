// SPDX-License-Identifier: AGPL-3.0-or-later

#include "bot.h"

#include <QMdmmAgent>

KnifePreferredBot::KnifePreferredBot(QMdmmNetworking::Client *parent)
    : Bot(parent)
{
}

// The real strategy lands later (see the C2 backlog item). For now every handler
// only guarantees the reply-or-giveUp contract: it always answers with a legal
// reply or explicitly gives up, so the bot never stalls a match until the server
// times it out. The choices below mirror smoke/main.cpp's competent auto-player.

void KnifePreferredBot::handleRockPaperScissorsRequest(const QStringList &playerNames, int strivedOrder)
{
    Q_UNUSED(playerNames);
    Q_UNUSED(strivedOrder);

    // Any throw is legal; pick Rock until the strategy decides otherwise.
    client()->agent()->rockPaperScissors(QMdmmCore::Data::Rock);
}

void KnifePreferredBot::handleActionOrderRequest(const QList<int> &remainedOrders, int maximumOrder, int selectionNum)
{
    Q_UNUSED(maximumOrder);

    // Take the first `selectionNum` available orders. The server never requests
    // more selections than there are remaining orders, so this is always legal.
    QList<int> order;
    order.reserve(selectionNum);
    for (int i = 0; i < selectionNum; ++i)
        order << remainedOrders.at(i);
    client()->agent()->actionOrder(order);
}

void KnifePreferredBot::handleActionRequest(int currentOrder)
{
    Q_UNUSED(currentOrder);

    QMdmmCore::Player *self = selfPlayer();
    if (self == nullptr) {
        // Not signed in yet; give up rather than send a bogus reply.
        client()->agent()->giveUpRequest();
        return;
    }

    // Buy a knife first: slashing is the only way to actually end a round.
    if (!self->hasKnife()) {
        if (self->canBuyKnife()) {
            client()->agent()->action(QMdmmCore::Data::BuyKnife, QString(), 0);
            return;
        }
        // Cannot buy where we stand (e.g. Village) -> step to any city seat.
        const int seatCount = client()->room()->players().count();
        for (int place = 1; place <= seatCount; ++place) {
            if (self->canMove(place)) {
                client()->agent()->action(QMdmmCore::Data::Move, QString(), place);
                return;
            }
        }
    }

    // Attack a co-located opponent.
    for (QMdmmCore::Player *to : opponents()) {
        if (self->canSlash(to)) {
            client()->agent()->action(QMdmmCore::Data::Slash, to->objectName(), 0);
            return;
        }
        if (self->canKick(to)) {
            client()->agent()->action(QMdmmCore::Data::Kick, to->objectName(), 0);
            return;
        }
    }

    // Buy a horse for extra reach next round.
    if (self->canBuyHorse()) {
        client()->agent()->action(QMdmmCore::Data::BuyHorse, QString(), 0);
        return;
    }

    // Walk toward an opponent (star map: every place is adjacent only to
    // Village, so X -> Village -> target).
    for (QMdmmCore::Player *to : opponents()) {
        const int dest = (self->place() == QMdmmCore::Data::Village) ? to->place() : QMdmmCore::Data::Village;
        if (self->canMove(dest)) {
            client()->agent()->action(QMdmmCore::Data::Move, QString(), dest);
            return;
        }
    }

    client()->agent()->action(QMdmmCore::Data::DoNothing, QString(), 0);
}

void KnifePreferredBot::handleUpgradeRequest(int remainingTimes)
{
    QMdmmCore::Player *self = selfPlayer();
    if (self == nullptr) {
        client()->agent()->giveUpRequest();
        return;
    }

    // Spend every point (knife -> horse -> maxHp), mirroring the server's
    // feasible default. If the total remaining capacity is less than the points
    // to spend, no feasible list exists; give up and let the server fall back.
    int knife = self->upgradeKnifeRemainingTimes();
    int horse = self->upgradeHorseRemainingTimes();
    int maxHp = self->upgradeMaxHpRemainingTimes();
    if (knife + horse + maxHp < remainingTimes) {
        client()->agent()->giveUpRequest();
        return;
    }

    QList<QMdmmCore::Data::UpgradeItem> items;
    items.reserve(remainingTimes);
    int left = remainingTimes;
    const auto take = [&items, &left](QMdmmCore::Data::UpgradeItem item, int &remaining) {
        const int n = qMin(left, remaining);
        for (int i = 0; i < n; ++i)
            items << item;
        left -= n;
        remaining -= n;
    };
    take(QMdmmCore::Data::UpgradeKnife, knife);
    take(QMdmmCore::Data::UpgradeHorse, horse);
    take(QMdmmCore::Data::UpgradeMaxHp, maxHp);
    client()->agent()->upgrade(items);
}
