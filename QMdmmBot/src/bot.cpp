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

double Bot::threatScore(const QString &playerName) const
{
    const QMdmmCore::Room *room = client()->room();
    const QMdmmCore::Player *self = room->player(client()->objectName());
    const QMdmmCore::Player *threat = room->player(playerName);

    // Before sign-in there is no self player yet, and a peer that is not in the
    // room (or is already dead) cannot hurt us.
    if (self == nullptr || threat == nullptr || self->dead() || threat->dead())
        return 0.0;

    // Reach: a peer in the same place can hit us right now, a peer that is
    // merely adjacent has to move in first and so lands a round later, and a
    // peer further away cannot reach us within one round at all.
    double reach = 0.0;
    if (threat->place() == self->place())
        reach = threatSamePlaceWeight;
    else if (QMdmmCore::Data::isPlaceAdjacent(threat->place(), self->place()))
        reach = threatAdjacentWeight;
    else
        return 0.0;

    // Offensive power: the weapons the peer holds. The hit lands wherever this
    // bot is standing -- its own place, or here after the move in -- and a horse
    // cannot be used inside the Village.
    double damage = 0.0;
    if (threat->hasKnife())
        damage += threat->knifeDamage();
    if (threat->hasHorse() && self->place() != QMdmmCore::Data::Village)
        damage += threat->horseDamage();

    return reach * damage;
}

double Bot::targetScore(const QString &playerName) const
{
    // Both dimensions are weighted and then added; the two terms are kept apart
    // so the weighting reads as one step and the sum as another.
    const double grudge = revengeWeight * revengeScore(playerName);
    const double threat = threatWeight * threatScore(playerName);

    return grudge + threat;
}

QString Bot::selectTarget() const
{
    const QMdmmCore::Room *room = client()->room();
    const QString selfName = client()->objectName();

    QString target;
    double best = 0.0;

    // Only alive peers are candidates, and a strict comparison from the zero
    // start means a peer that scores nothing is never picked and that the first
    // of several equally good peers wins.
    const QList<const QMdmmCore::Player *> alive = room->alivePlayers();
    for (const QMdmmCore::Player *player : alive) {
        if (player->objectName() == selfName)
            continue;
        const double score = targetScore(player->objectName());
        if (score > best) {
            best = score;
            target = player->objectName();
        }
    }

    return target;
}

const QMdmmCore::LogicConfiguration &Bot::logicConfiguration() const
{
    // The mirror holds the rules the server broadcast. Before that broadcast, and
    // for every rule it left out, the getters answer from
    // LogicConfiguration::defaults() -- which is the same answer Player works the
    // punish out from, so the two can never disagree.
    return client()->room()->logicConfiguration();
}

bool Bot::canSlashSafely(const QMdmmCore::Player *to) const
{
    const QMdmmCore::Player *self = client()->room()->player(client()->objectName());

    // Before sign-in there is no self player, and a peer that is not in the room
    // is nothing to slash at.
    if (self == nullptr || to == nullptr)
        return false;

    if (!self->canSlash(to))
        return false;

    // A slash pays for itself in HP, so a bot leaves out the ones that would take
    // it to its own death threshold. Where that threshold lies is a rule of the
    // match, so it is asked for rather than assumed.
    const int hpLeft = self->hp() - self->slashPunishHp();
    return logicConfiguration().zeroHpAsDead() ? (hpLeft > 0) : (hpLeft >= 0);
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
