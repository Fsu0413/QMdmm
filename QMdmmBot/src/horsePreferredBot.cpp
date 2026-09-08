// SPDX-License-Identifier: AGPL-3.0-or-later

#include "bot.h"

horsePreferredBot::horsePreferredBot(QMdmmNetworking::Client *parent)
    : Bot(parent)
{
}

// The strategy is not implemented yet; these are placeholders that keep the
// subclass concrete until the horse-preferred strategy lands.
void horsePreferredBot::handleRockPaperScissorsRequest(const QStringList &playerNames, int strivedOrder)
{
    Q_UNUSED(playerNames);
    Q_UNUSED(strivedOrder);
}

void horsePreferredBot::handleActionOrderRequest(const QList<int> &remainedOrders, int maximumOrder, int selectionNum)
{
    Q_UNUSED(remainedOrders);
    Q_UNUSED(maximumOrder);
    Q_UNUSED(selectionNum);
}

void horsePreferredBot::handleActionRequest(int currentOrder)
{
    Q_UNUSED(currentOrder);
}

void horsePreferredBot::handleUpgradeRequest(int remainingTimes)
{
    Q_UNUSED(remainingTimes);
}
