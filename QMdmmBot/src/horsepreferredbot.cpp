// SPDX-License-Identifier: AGPL-3.0-or-later

#include "bot.h"

#include <QMdmmAgent>

HorsePreferredBot::HorsePreferredBot(QMdmmNetworking::Client *parent)
    : Bot(parent)
{
}

// The horse style (issue #6 C3, the sibling of the knife style C2). Where the
// knife style sharpens a knife and walks up to whatever it wants dead, this one
// rides: the horse is bought and sharpened first (Q4: horse damage, then knife
// damage, then max HP), and what the horse is for is the pull-kick loop. A kick
// costs nothing and needs nothing but a horse and a peer in the same city -- and
// it throws that peer back into the Village by itself (see Player::kick()). So
// the round goes: drag a peer out of the Village into the city this bot stands
// in (LetMove), kick it, and do it again. A peer that has to spend its own
// rounds on getting away is a peer that is not spending them on anything else,
// and every kick lands without this bot ever paying a city's punish for a slash.
//
// The Village is where that loop is not available: a kick is forbidden there
// (Player::canKick()), and with LetMove turned off the rules do not allow
// dragging a peer anywhere at all (LogicConfiguration::enableLetMove()). What is
// left then is ordinary play -- Q4's "walk up and slash it". That walk goes
// through the Village anyway (every place is adjacent only to the Village), and
// the Village is where a slash is free, while a city charges the slasher its own
// HP for one; so the Village is where this style is happiest to meet a peer. A
// city slash is still taken when it is survivable (see Bot::canSlashSafely())
// -- passing it up is not a vow of poverty, it is a preference for the kick,
// which is free.
//
// A round starts every player bare, at full HP and at its own seat (see
// Room::prepareForRoundStart()), and it holds as many action times as it takes
// for it to be over. So the horse is bought again at the start of every round,
// and the loop is played out inside one round.
//
// Every handler keeps the reply-or-giveUp contract: it always answers with a
// legal reply or explicitly gives up, so the bot never stalls a match until the
// server times it out.

void HorsePreferredBot::handleRockPaperScissorsRequest(const QStringList &playerNames, int strivedOrder)
{
    Q_UNUSED(playerNames);
    Q_UNUSED(strivedOrder);

    // Any throw is legal. Which throw to use is a question about the action order
    // this style would like (issue #6 Q5, the C4 item) and not one about its
    // weapon of choice, so this is Rock until that item lands. It is also why two
    // of these bots still tie every Rock-Paper-Scissors and never get past the
    // first action time of a round (the D1 backlog item).
    client()->agent()->rockPaperScissors(QMdmmCore::Data::Rock);
}

void HorsePreferredBot::handleActionOrderRequest(const QList<int> &remainedOrders, int maximumOrder, int selectionNum)
{
    Q_UNUSED(maximumOrder);

    // Take the first `selectionNum` available orders. The server never requests
    // more selections than there are remaining orders, so this is always legal.
    QList<int> order;
    order.reserve(selectionNum);
    for (int i = 0; i < selectionNum; ++i)
        order << remainedOrders.at(i);
    client()->agent()->actionOrder(order);
}

void HorsePreferredBot::handleActionRequest(int currentOrder)
{
    Q_UNUSED(currentOrder);

    QMdmmCore::Player *self = selfPlayer();
    if (self == nullptr) {
        // Not signed in yet; give up rather than send a bogus reply.
        client()->agent()->giveUpRequest();
        return;
    }

    // A peer standing here goes down for free: a kick costs nothing and needs no
    // knife, and it is the very thing this style bought its horse for. Only
    // where a kick is impossible -- the Village forbids it, and it takes a horse
    // -- does a slash come into it, and then only a survivable one (see
    // Bot::canSlashSafely()).
    if (QMdmmCore::Player *to = attackTarget(); to != nullptr) {
        if (self->canKick(to)) {
            client()->agent()->action(QMdmmCore::Data::Kick, to->objectName(), 0);
            return;
        }
        if (canSlashSafely(to)) {
            client()->agent()->action(QMdmmCore::Data::Slash, to->objectName(), 0);
            return;
        }
    }

    // The horse first (Q4): it is the weapon this style is named after, and the
    // one the loop is built on. Shopping happens in a city -- the Village sells
    // nothing -- and the rules may keep the shops to this bot's starting city,
    // in which case the horse waits until the walk has taken it back there.
    if (!self->hasHorse() && self->canBuyHorse()) {
        client()->agent()->action(QMdmmCore::Data::BuyHorse, QString(), 0);
        return;
    }

    // The pull, with the loop's second half to follow next action time: a peer
    // worth a round is dragged out of the Village and into the city this bot
    // stands in, where a kick can reach it (see pullTarget()).
    const QString victimName = pullTarget();
    if (!victimName.isEmpty()) {
        client()->agent()->action(QMdmmCore::Data::LetMove, victimName, self->place());
        return;
    }

    // The knife is bought too, just not first (Q4). It is the weapon for the
    // places a kick cannot go -- the Village -- and for the rounds the horse is
    // not in hand yet.
    if (!self->hasKnife() && self->canBuyKnife()) {
        client()->agent()->action(QMdmmCore::Data::BuyKnife, QString(), 0);
        return;
    }

    // Walk. What this bot is walking towards is either the peer the score picks
    // or a shop, and the map sends both walks through the Village (see
    // wantedPlace()).
    const int wanted = wantedPlace();
    if (wanted >= 0) {
        const int dest = self->canMove(wanted) ? wanted : QMdmmCore::Data::Village;
        if (self->canMove(dest)) {
            client()->agent()->action(QMdmmCore::Data::Move, QString(), dest);
            return;
        }
    }

    client()->agent()->action(QMdmmCore::Data::DoNothing, QString(), 0);
}

void HorsePreferredBot::handleUpgradeRequest(int remainingTimes)
{
    QMdmmCore::Player *self = selfPlayer();
    if (self == nullptr) {
        client()->agent()->giveUpRequest();
        return;
    }

    // Spend every point, horse first and max HP last (Q4): the horse is what
    // this style kicks with, the knife is what it falls back to where a kick is
    // not allowed, and max HP is what keeps it standing while it works the loop.
    // If the total remaining capacity is less than the points to spend, no
    // feasible list exists; give up and let the server fall back.
    int horse = self->upgradeHorseRemainingTimes();
    int knife = self->upgradeKnifeRemainingTimes();
    int maxHp = self->upgradeMaxHpRemainingTimes();
    if (horse + knife + maxHp < remainingTimes) {
        client()->agent()->giveUpRequest();
        return;
    }

    QList<QMdmmCore::Data::UpgradeItem> items;
    items.reserve(remainingTimes);
    int left = remainingTimes;
    const auto take = [&items, &left](QMdmmCore::Data::UpgradeItem item, int &remaining) {
        const int n = qMin(left, remaining);
        for (int i = 0; i < n; ++i)
            items << item;
        left -= n;
        remaining -= n;
    };
    take(QMdmmCore::Data::UpgradeHorse, horse);
    take(QMdmmCore::Data::UpgradeKnife, knife);
    take(QMdmmCore::Data::UpgradeMaxHp, maxHp);
    client()->agent()->upgrade(items);
}

QString HorsePreferredBot::pullTarget() const
{
    const QMdmmCore::Room *room = client()->room();
    const QMdmmCore::Player *self = room->player(client()->objectName());
    if (self == nullptr)
        return {};

    // A kick only lands in a city, and the only peer this bot can drag into its
    // own place is one standing in an adjacent place. From a city that place is
    // the Village, so a bot standing in the Village has nobody to drag anywhere
    // worth dragging them to.
    if (self->place() == QMdmmCore::Data::Village)
        return {};

    // The pull costs a whole round, so it is only spent on a peer the score rates
    // (see Bot::selectTarget(), and Bot::attackTarget() for the strike it is
    // deliberately not): a peer that has never wronged this bot and carries no
    // weapon is dragged over and kicked straight back out, which buys nothing,
    // while a blow thrown at whatever is already here costs nothing to try.
    QString targetName = selectTarget();
    if (targetName.isEmpty())
        return {};

    const QMdmmCore::Player *victim = room->player(targetName);
    if (victim == nullptr)
        return {};

    // The place asked for is this bot's own, which is the pull the rules name as
    // the one that brings a peer from an adjacent place into this one. Whether a
    // peer may be dragged around at all is also the rules' business, and
    // Player::canLetMove() is where they say so (see
    // QMdmmCore::LogicConfiguration::enableLetMove()): a match that has turned
    // dragging off answers this with a no, and the loop is over before it starts
    // (Q4's degenerate case).
    if (!self->canLetMove(victim, self->place()))
        return {};

    return targetName;
}

int HorsePreferredBot::wantedPlace()
{
    const QMdmmCore::Room *room = client()->room();
    const QMdmmCore::Player *self = room->player(client()->objectName());
    if (self == nullptr)
        return -1;

    // The peer the score rates, if it rates one. As in the knife style, a peer
    // nobody rates does not leave this bot without a direction either: an unarmed
    // stranger still goes down to a kick or a knife like any other, and the kill
    // it yields is still an upgrade point (see the fallback at the end).
    const QString targetName = selectTarget();
    const QMdmmCore::Player *rated = targetName.isEmpty() ? nullptr : room->player(targetName);

    // What this bot is short of: a weapon it has not got and cannot buy from
    // where it stands. A round start leaves every player bare, so this is the
    // ordinary way to start a round, not an edge case; the Village sells
    // nothing, and the rules may keep the shops to this bot's starting city.
    const bool shortOfHorse = !self->hasHorse() && !self->canBuyHorse();
    const bool shortOfKnife = !self->hasKnife() && !self->canBuyKnife();
    const bool shopping = shortOfHorse || shortOfKnife;

    // A rated peer one step away wins the round: closing in on it is the round's
    // chance at a fight, and only a rated peer is worth the pull's round either.
    if (rated != nullptr && self->canMove(rated->place()))
        return rated->place();

    // Otherwise the round is for the shopping that is not possible here, and the
    // city to walk back to is this bot's starting one: it is a city the shops are
    // never kept away from, and the walk to it is the same star-shaped walk as
    // any other.
    if (shopping)
        return self->initialPlace();

    // Nothing to buy: the walk goes towards a peer -- the rated one, or failing
    // that the first peer alive.
    if (rated != nullptr)
        return rated->place();

    const QList<QMdmmCore::Player *> others = opponents();
    if (!others.isEmpty())
        return others.first()->place();

    return -1;
}
