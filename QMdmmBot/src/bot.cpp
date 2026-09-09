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

// Bot is abstract because the four request handlers are pure virtual (a style
// subclass must override all of them to become concrete). The destructor is
// also pure virtual and gets a defaulted definition here.
Bot::~Bot() = default;

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

QMdmmCore::Player *Bot::selfPlayer()
{
    return client()->room()->player(client()->objectName());
}

QList<QMdmmCore::Player *> Bot::opponents()
{
    QList<QMdmmCore::Player *> result;
    const QString selfName = client()->objectName();
    const QList<QMdmmCore::Player *> alive = client()->room()->alivePlayers();
    for (QMdmmCore::Player *player : alive) {
        if (player->objectName() != selfName)
            result << player;
    }
    return result;
}

Bot *Bot::createBot(const QString &style, QMdmmNetworking::Client *parent)
{
    if (style == QStringLiteral("knifePreferred"))
        return new KnifePreferredBot(parent);
    if (style == QStringLiteral("horsePreferred"))
        return new HorsePreferredBot(parent);
    if (style == QStringLiteral("rl"))
        return new RlBot(parent);
    return nullptr;
}

bool Bot::styleExist(const QString &style)
{
    return style == QStringLiteral("knifePreferred") || style == QStringLiteral("horsePreferred") || style == QStringLiteral("rl");
}
