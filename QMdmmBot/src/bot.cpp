// SPDX-License-Identifier: AGPL-3.0-or-later

#include "bot.h"

#include <QMdmmAgent>

Bot::Bot(QMdmmNetworking::Client *parent)
    : QObject(parent)
{
    QMdmmNetworking::Agent *agent = client()->agent();

    // Requests: the server asks this bot for a choice. Each handler is a
    // virtual slot so a style subclass overrides it to implement its strategy.
    connect(agent, &QMdmmNetworking::Agent::rockPaperScissorsRequested, this, &Bot::handleRockPaperScissorsRequest);
    connect(agent, &QMdmmNetworking::Agent::actionOrderRequested, this, &Bot::handleActionOrderRequest);
    connect(agent, &QMdmmNetworking::Agent::actionRequested, this, &Bot::handleActionRequest);
    connect(agent, &QMdmmNetworking::Agent::upgradeRequested, this, &Bot::handleUpgradeRequest);

    // Notifications: the server broadcasts game progress.
    connect(agent, &QMdmmNetworking::Agent::logicConfigurationNotified, this, &Bot::handleLogicConfigurationNotified);
    connect(agent, &QMdmmNetworking::Agent::roundStartNotified, this, &Bot::handleRoundStartNotified);
    connect(agent, &QMdmmNetworking::Agent::actionNotified, this, &Bot::handleActionNotified);
    connect(agent, &QMdmmNetworking::Agent::upgradeNotified, this, &Bot::handleUpgradeNotified);
    connect(agent, &QMdmmNetworking::Agent::roundOverNotified, this, &Bot::handleRoundOverNotified);
    connect(agent, &QMdmmNetworking::Agent::gameOverNotified, this, &Bot::handleGameOverNotified);
}

// There are no other suitable pure virtual functions so...
// let's make the dtor pure virtual so that it must be inherited
Bot::~Bot() = default;

// Request handlers: the base implementations do nothing. Style subclasses
// override them to reply through the agent's bare-verb methods with their
// strategy.

void Bot::handleRockPaperScissorsRequest(const QStringList &playerNames, int strivedOrder)
{
    Q_UNUSED(playerNames);
    Q_UNUSED(strivedOrder);
}

void Bot::handleActionOrderRequest(const QList<int> &remainedOrders, int maximumOrder, int selectionNum)
{
    Q_UNUSED(remainedOrders);
    Q_UNUSED(maximumOrder);
    Q_UNUSED(selectionNum);
}

void Bot::handleActionRequest(int currentOrder)
{
    Q_UNUSED(currentOrder);
}

void Bot::handleUpgradeRequest(int remainingTimes)
{
    Q_UNUSED(remainingTimes);
}

// Notification handlers: the base implementations do nothing. Style subclasses
// override them to maintain their own view of the match.

void Bot::handleLogicConfigurationNotified()
{
}

void Bot::handleRoundStartNotified()
{
}

void Bot::handleActionNotified(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace)
{
    Q_UNUSED(playerName);
    Q_UNUSED(action);
    Q_UNUSED(toPlayer);
    Q_UNUSED(toPlace);
}

void Bot::handleUpgradeNotified(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades)
{
    Q_UNUSED(upgrades);
}

void Bot::handleRoundOverNotified()
{
}

void Bot::handleGameOverNotified(const QStringList &playerNames)
{
    Q_UNUSED(playerNames);
}

QMdmmNetworking::Client *Bot::client()
{
    // TODO: nolintnextline comment
    return static_cast<QMdmmNetworking::Client *>(parent());
}

const QMdmmNetworking::Client *Bot::client() const
{
    // TODO: nolintnextline comment
    return static_cast<const QMdmmNetworking::Client *>(parent());
}

Bot *Bot::createBot(const QString &style, QMdmmNetworking::Client *parent)
{
    // TODO: derived class
    Q_UNUSED(style);
    Q_UNUSED(parent);
    return nullptr;
}

bool Bot::styleExist(const QString &style)
{
    // TODO: derived class
    Q_UNUSED(style);
    return false;
}
