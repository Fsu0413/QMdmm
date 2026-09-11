// SPDX-License-Identifier: AGPL-3.0-or-later

#include "bot.h"

#include <QMdmmAgent>

using namespace Qt::StringLiterals;

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

// Notification handlers: apart from the revenge memory (see Bot::revengeScore()),
// which handleActionNotified() and handleRoundOverNotified() maintain, the base
// implementations do nothing. Style subclasses override them to maintain their
// own view of the match.

void Bot::handleLogicConfigurationNotified()
{
}

void Bot::handleRoundStartNotified()
{
}

void Bot::handleActionNotified(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace)
{
    Q_UNUSED(toPlace);

    // Revenge memory: only Slash and Kick are hostile. LetMove moves a player
    // against their will but deals no damage, so it never earns a grudge.
    if (action != QMdmmCore::Data::Slash && action != QMdmmCore::Data::Kick)
        return;
    if (toPlayer != client()->objectName())
        return;

    revenge_[playerName] += revengePerAttack;
}

void Bot::handleUpgradeNotified(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades)
{
    Q_UNUSED(upgrades);
}

void Bot::handleRoundOverNotified()
{
    // Revenge memory: a round has passed, so every grudge fades a little. An
    // entry is only dropped once it has become negligible, which is what lets a
    // grudge outlive the round it was earned in.
    const QStringList attackers = revenge_.keys();
    for (const QString &attacker : attackers) {
        const double decayed = revenge_.value(attacker) * revengeDecayPerRound;
        if (decayed <= revengeEpsilon)
            revenge_.remove(attacker);
        else
            revenge_.insert(attacker, decayed);
    }
}

void Bot::handleGameOverNotified(const QStringList &playerNames)
{
    Q_UNUSED(playerNames);
}

QMdmmNetworking::Client *Bot::client()
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast): Bot always has Client as parent; skip dynamic_cast cost
    return static_cast<QMdmmNetworking::Client *>(parent());
}

const QMdmmNetworking::Client *Bot::client() const
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast): same as above
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

double Bot::revengeScore(const QString &playerName) const
{
    // A peer that never attacked us is missing from the table, and QHash::value
    // hands back a default-constructed double for it -- exactly the 0 wanted.
    return revenge_.value(playerName);
}

Bot *Bot::createBot(const QString &style, QMdmmNetworking::Client *parent)
{
    if (style == u"knifePreferred"_s)
        return new KnifePreferredBot(parent);
    if (style == u"horsePreferred"_s)
        return new HorsePreferredBot(parent);
    if (style == u"rl"_s)
        return new RlBot(parent);
    return nullptr;
}

bool Bot::styleExist(const QString &style)
{
    return style == u"knifePreferred"_s || style == u"horsePreferred"_s || style == u"rl"_s;
}
