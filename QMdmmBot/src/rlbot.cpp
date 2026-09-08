// SPDX-License-Identifier: AGPL-3.0-or-later

#include "bot.h"

#include <exception>

RlBot::RlBot(QMdmmNetworking::Client *parent)
    : Bot(parent)
{
    // The rl style is recognized by the configuration but not implemented yet.
    // Constructing one is a hard failure instead of silently joining a game
    // with no strategy.
    qCritical().noquote() << QStringLiteral("The Reinforcement Learning playing style is not implemented yet. Terminating.");
    std::terminate();
}

// Unreachable at runtime (the constructor terminates), but the handlers must
// still be defined so that RlBot stays a concrete class.
void RlBot::handleRockPaperScissorsRequest(const QStringList &playerNames, int strivedOrder)
{
    Q_UNUSED(playerNames);
    Q_UNUSED(strivedOrder);
    Q_UNREACHABLE();
}

void RlBot::handleActionOrderRequest(const QList<int> &remainedOrders, int maximumOrder, int selectionNum)
{
    Q_UNUSED(remainedOrders);
    Q_UNUSED(maximumOrder);
    Q_UNUSED(selectionNum);
    Q_UNREACHABLE();
}

void RlBot::handleActionRequest(int currentOrder)
{
    Q_UNUSED(currentOrder);
    Q_UNREACHABLE();
}

void RlBot::handleUpgradeRequest(int remainingTimes)
{
    Q_UNUSED(remainingTimes);
    Q_UNREACHABLE();
}
