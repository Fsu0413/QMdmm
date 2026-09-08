// SPDX-License-Identifier: AGPL-3.0-or-later

#include "bot.h"

HorsePreferredBot::HorsePreferredBot(QMdmmNetworking::Client *parent)
    : Bot(parent)
{
}

// The strategy is not implemented yet; these are placeholders that keep the
// subclass concrete until the horse-preferred strategy lands.
void HorsePreferredBot::handleRockPaperScissorsRequest(const QStringList &playerNames, int strivedOrder)
{
    Q_UNUSED(playerNames);
    Q_UNUSED(strivedOrder);
}

void HorsePreferredBot::handleActionOrderRequest(const QList<int> &remainedOrders, int maximumOrder, int selectionNum)
{
    Q_UNUSED(remainedOrders);
    Q_UNUSED(maximumOrder);
    Q_UNUSED(selectionNum);
}

void HorsePreferredBot::handleActionRequest(int currentOrder)
{
    Q_UNUSED(currentOrder);
}

void HorsePreferredBot::handleUpgradeRequest(int remainingTimes)
{
    Q_UNUSED(remainingTimes);
}
