// SPDX-License-Identifier: AGPL-3.0-or-later

#include "bot.h"

knifePreferredBot::knifePreferredBot(QMdmmNetworking::Client *parent)
    : Bot(parent)
{
}

// The strategy is not implemented yet; these are placeholders that keep the
// subclass concrete until the knife-preferred strategy lands.
void knifePreferredBot::handleRockPaperScissorsRequest(const QStringList &playerNames, int strivedOrder)
{
    Q_UNUSED(playerNames);
    Q_UNUSED(strivedOrder);
}

void knifePreferredBot::handleActionOrderRequest(const QList<int> &remainedOrders, int maximumOrder, int selectionNum)
{
    Q_UNUSED(remainedOrders);
    Q_UNUSED(maximumOrder);
    Q_UNUSED(selectionNum);
}

void knifePreferredBot::handleActionRequest(int currentOrder)
{
    Q_UNUSED(currentOrder);
}

void knifePreferredBot::handleUpgradeRequest(int remainingTimes)
{
    Q_UNUSED(remainingTimes);
}
