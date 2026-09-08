// SPDX-License-Identifier: AGPL-3.0-or-later

#include "bot.h"

#include <exception>

rlBot::rlBot(QMdmmNetworking::Client *parent)
    : Bot(parent)
{
    // The rl style is recognized by the configuration but not implemented yet.
    // Constructing one is a hard failure instead of silently joining a game
    // with no strategy.
    std::terminate();
}

// Unreachable at runtime (the constructor terminates), but the handlers must
// still be defined so that rlBot stays a concrete class.
void rlBot::handleRockPaperScissorsRequest(const QStringList &playerNames, int strivedOrder)
{
    Q_UNUSED(playerNames);
    Q_UNUSED(strivedOrder);
}

void rlBot::handleActionOrderRequest(const QList<int> &remainedOrders, int maximumOrder, int selectionNum)
{
    Q_UNUSED(remainedOrders);
    Q_UNUSED(maximumOrder);
    Q_UNUSED(selectionNum);
}

void rlBot::handleActionRequest(int currentOrder)
{
    Q_UNUSED(currentOrder);
}

void rlBot::handleUpgradeRequest(int remainingTimes)
{
    Q_UNUSED(remainingTimes);
}
