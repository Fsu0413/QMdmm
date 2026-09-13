// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMBOT_BOT_H
#define QMDMMBOT_BOT_H

#include <QHash>
#include <QList>
#include <QStringList>

#include <QMdmmClient>
#include <QMdmmData>
#include <QMdmmLogicConfiguration>
#include <QMdmmPlayer>

#include <QObject>

class Bot : public QObject
{
    Q_OBJECT

public:
    explicit Bot(QMdmmNetworking::Client *parent);
    ~Bot() override = 0;

    [[nodiscard]] QMdmmNetworking::Client *client();
    [[nodiscard]] const QMdmmNetworking::Client *client() const;

    Q_DISABLE_COPY_MOVE(Bot);

    [[nodiscard]] static Bot *createBot(const QString &style, QMdmmNetworking::Client *parent);
    [[nodiscard]] static bool styleExist(const QString &style);

protected:
    // Returns this bot's own player in the local room mirror, or nullptr before
    // sign-in completes. The client's objectName is its playerName (see the
    // Client class doc), so the self player is looked up by that name.
    [[nodiscard]] QMdmmCore::Player *selfPlayer();

    // Returns every alive player except this bot, in room order.
    [[nodiscard]] QList<QMdmmCore::Player *> opponents();

    // The rules this match is played under, read off the room mirror the way the
    // rest of the match state is: the server broadcasts them and the client keeps
    // them in the room, so a strategy sees the very rules the players are playing
    // under instead of tracking a copy of its own. The rules decide what a place
    // is worth -- a slash is punished in a city and free in the Village, and
    // moving a peer around may not be available at all.
    [[nodiscard]] const QMdmmCore::LogicConfiguration &logicConfiguration() const;

    // Whether this bot can slash that peer without paying for the slash with its
    // life: a slash in a city is punished with the slasher's own HP (see
    // Player::slashPunishHp()), and one that would finish this bot off is never
    // worth taking -- doing nothing is always a legal reply, so a strategy just
    // skips such a slash. A slash that is not possible in the first place, or one
    // from a place that does not punish, is unaffected.
    [[nodiscard]] bool canSlashSafely(const QMdmmCore::Player *to) const;

protected slots: // NOLINT(readability-redundant-access-specifiers)
    // Request handlers: invoked when the server asks this bot to make a choice.
    // Pure virtual so each style subclass is forced to answer with its own
    // strategy; a bot that never answers would otherwise stall the match until
    // the server times it out and disconnects it.
    virtual void handleRockPaperScissorsRequest(const QStringList &playerNames, int strivedOrder) = 0;
    virtual void handleActionOrderRequest(const QList<int> &remainedOrders, int maximumOrder, int selectionNum) = 0;
    virtual void handleActionRequest(int currentOrder) = 0;
    virtual void handleUpgradeRequest(int remainingTimes) = 0;

    // Notification handlers: invoked when the server broadcasts game progress.
    // Virtual for the same reason -- style subclasses track the match here.
    // The base implementations of handleActionNotified() and
    // handleRoundOverNotified() also maintain the revenge memory, so a subclass
    // that overrides either of them must call the base implementation.
    virtual void handleLogicConfigurationNotified();
    virtual void handleRoundStartNotified();
    virtual void handleActionNotified(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace);
    virtual void handleUpgradeNotified(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades);
    virtual void handleRoundOverNotified();
    virtual void handleGameOverNotified(const QStringList &playerNames);

    // Revenge memory: how much of a grudge this bot holds against one peer, 0
    // for a peer that never attacked it. Only a hostile action aimed at this bot
    // (Slash or Kick) earns a grudge -- LetMove deals no damage, so it never
    // counts. Grudges fade every finished round but survive across rounds, and a
    // peer that leaves is forgotten by that decay alone. Style strategies read
    // this when they pick a target.
    [[nodiscard]] double revengeScore(const QString &playerName) const;

    // Threat: how much damage this bot expects from one peer over the coming
    // round. The peer's offensive power is the damage of the weapons it holds
    // (a knife hits for knifeDamage, a horse for horseDamage), discounted by
    // how far away it is: one that already shares this bot's place can strike
    // right away, one that is merely adjacent has to step in first. A peer that
    // is not in the room (or one that is dead) is no threat at all. This is a
    // first cut, so further dimensions are expected to be added here as the
    // strategies need them. Style strategies read this when they pick a target.
    [[nodiscard]] double threatScore(const QString &playerName) const;

    // Target score: how attractive one peer is as a target, which is its
    // weighted revenge score plus its weighted threat score (see the weights
    // below) -- this bot wants to hit back at whoever hurt it, and to take out
    // whoever can hurt it. Both dimensions are read as they come, so a dead peer
    // still scores the grudge it earned (threatScore() is the one death zeroes
    // out). Style strategies read this when they pick a target.
    [[nodiscard]] double targetScore(const QString &playerName) const;

    // Target selection: the peer this bot currently wants to act against, that
    // is, the alive peer with the highest target score. Ties go to the first
    // peer in room order (players are held in a name-ordered map), so the pick
    // is deterministic rather than dependent on iteration order. An empty name
    // means nobody is worth aiming at: either no opponent is left or none of
    // them scores above zero.
    //
    // The zero score does exclude here, unlike in attackTarget(): this is the
    // question a style asks before spending a round on a peer -- walking towards
    // it, or dragging it into reach -- and a round has to be paid for with
    // something. A blow thrown at a peer that is already standing here is not
    // paid for with anything, which is why that question (attackTarget()) is the
    // cheaper one and answers differently.
    [[nodiscard]] QString selectTarget() const;

    // The peer to strike right now: among the peers standing in this bot's place,
    // the one with the highest target score (see Bot::targetScore()), a tie going
    // to room order. The score ranks those peers but does not veto the blow -- a
    // peer that has never wronged this bot and carries no weapon still goes down,
    // and the kill it yields is an upgrade point -- so the first co-located peer
    // is taken when none of them scores at all. nullptr means nobody is standing
    // here to strike.
    [[nodiscard]] QMdmmCore::Player *attackTarget();

private:
    // A hostile action is worth this many grudges; every finished round
    // multiplies all of them by the decay factor, and an entry that has decayed
    // to the epsilon or below is dropped so the table stays bounded over a long
    // match.
    static constexpr double revengePerAttack = 1.0;
    static constexpr double revengeDecayPerRound = 0.8;
    static constexpr double revengeEpsilon = 0.01;

    // A peer sharing this bot's place strikes this round, so its weapons count
    // in full; a peer that is only adjacent has to move in first, so its hit
    // lands a round later and counts for half. A peer further away cannot reach
    // this bot within one round at all.
    static constexpr double threatSamePlaceWeight = 1.0;
    static constexpr double threatAdjacentWeight = 0.5;

    // Target score weights: a grudge and an incoming threat both make a peer
    // worth acting against, so the two dimensions are summed. They are kept
    // apart, and named, because they are not the same unit -- a grudge counts
    // hits taken, a threat counts damage taken -- so a style strategy that wants
    // one of them to dominate can rebalance them here without touching the
    // scoring code. At equal weight the score is a plain sum.
    static constexpr double revengeWeight = 1.0;
    static constexpr double threatWeight = 1.0;

    QHash<QString, double> revenge_;
};

// The three playing styles. Each is a concrete Bot whose strategy is implemented
// (or, for rlBot, deliberately absent) in its own translation unit. They are
// only instantiated through Bot::createBot().

// The knife style: buy the knife and keep it sharp (issue #6 Q3 -- the knife
// first, max HP second, the horse only once both are maxed out), and act on
// whatever stands within reach.
class KnifePreferredBot final : public Bot
{
public:
    explicit KnifePreferredBot(QMdmmNetworking::Client *parent);

protected:
    void handleRockPaperScissorsRequest(const QStringList &playerNames, int strivedOrder) override;
    void handleActionOrderRequest(const QList<int> &remainedOrders, int maximumOrder, int selectionNum) override;
    void handleActionRequest(int currentOrder) override;
    void handleUpgradeRequest(int remainingTimes) override;

private:
    // Whether two slashes would finish every peer still alive off: the knife
    // style's own reading of "the knife is sharp enough" (issue #6 Q3). It is
    // worked out afresh every time because both sides keep upgrading. Once it
    // holds, reach is no longer what this bot is short of, so it stops buying
    // horses. With no peer left the answer is yes -- there is nothing to finish
    // off then, and no horse to buy for it either.
    [[nodiscard]] bool twoSlashesFinishEveryone() const;

    // Whether a slash that a city would punish with this bot's life is worth
    // taking anyway (issue #6 Q3). Such a slash is normally passed up (see
    // Bot::canSlashSafely()); this is the one trade Q3 names as paying: the
    // slash finishes the peer off outright, so the bot banks the kill and the
    // upgrade point that comes with it -- points outlive the round, HP and
    // weapons do not -- while the bot is the stronger side of the two and no
    // third peer is left to profit from the round it spends dying. Q3's example
    // is exactly that duel: two peers alone in a city, the one that outmatches
    // the other cutting it down even though the punish takes it along.
    [[nodiscard]] bool slashIsWorthItsPunish(const QMdmmCore::Player *to) const;
};

class HorsePreferredBot final : public Bot
{
public:
    explicit HorsePreferredBot(QMdmmNetworking::Client *parent);

protected:
    void handleRockPaperScissorsRequest(const QStringList &playerNames, int strivedOrder) override;
    void handleActionOrderRequest(const QList<int> &remainedOrders, int maximumOrder, int selectionNum) override;
    void handleActionRequest(int currentOrder) override;
    void handleUpgradeRequest(int remainingTimes) override;
};

class RlBot final : public Bot
{
public:
    explicit RlBot(QMdmmNetworking::Client *parent);

protected:
    void handleRockPaperScissorsRequest(const QStringList &playerNames, int strivedOrder) override;
    void handleActionOrderRequest(const QList<int> &remainedOrders, int maximumOrder, int selectionNum) override;
    void handleActionRequest(int currentOrder) override;
    void handleUpgradeRequest(int remainingTimes) override;
};

#endif
