// SPDX-License-Identifier: AGPL-3.0-or-later

#include "bot.h"

#include <QMdmmAgent>

#include <algorithm>

KnifePreferredBot::KnifePreferredBot(QMdmmNetworking::Client *parent)
    : Bot(parent)
{
}

// The knife style (issue #6 C2). The knife comes first everywhere: it is bought
// before anything else, it is upgraded before max HP (which in turn comes before
// the horse, see Q3), and it is what the style strikes with rather than spending
// the round on a kick. The horse it does buy is bought for reach, and only while
// two slashes do not yet finish every peer off. Every handler keeps the
// reply-or-giveUp contract: it always answers with a legal reply or explicitly
// gives up, so the bot never stalls a match until the server times it out.

void KnifePreferredBot::handleRockPaperScissorsRequest(const QStringList &playerNames, int strivedOrder)
{
    Q_UNUSED(playerNames);
    Q_UNUSED(strivedOrder);

    // Any throw is legal; pick Rock until the strategy decides otherwise.
    client()->agent()->rockPaperScissors(QMdmmCore::Data::Rock);
}

void KnifePreferredBot::handleActionOrderRequest(const QList<int> &remainedOrders, int maximumOrder, int selectionNum)
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

void KnifePreferredBot::handleActionRequest(int currentOrder)
{
    Q_UNUSED(currentOrder);

    QMdmmCore::Player *self = selfPlayer();
    if (self == nullptr) {
        // Not signed in yet; give up rather than send a bogus reply.
        client()->agent()->giveUpRequest();
        return;
    }

    // Buy a knife first: slashing is the only way to actually end a round, and
    // the kill is what earns an upgrade point.
    if (!self->hasKnife()) {
        if (self->canBuyKnife()) {
            client()->agent()->action(QMdmmCore::Data::BuyKnife, QString(), 0);
            return;
        }
        // Cannot buy where we stand (the Village never sells, and the rules may
        // keep the shops to one's starting city) -> step to any city seat.
        const int seatCount = static_cast<int>(client()->room()->players().count());
        for (int place = 1; place <= seatCount; ++place) {
            if (self->canMove(place)) {
                client()->agent()->action(QMdmmCore::Data::Move, QString(), place);
                return;
            }
        }
    }

    // Strike whoever stands here, with the knife. What that costs is the place's
    // business: a slash inside the Village is free, while a slash in a city is
    // punished with this bot's own HP -- so one that would finish it off is
    // passed up unless it is the single trade Q3 names as paying (see
    // slashIsWorthItsPunish()). A kick is free, but it needs a horse and is not
    // possible inside the Village.
    if (QMdmmCore::Player *to = attackTarget(); to != nullptr) {
        if (canSlashSafely(to) || slashIsWorthItsPunish(to)) {
            client()->agent()->action(QMdmmCore::Data::Slash, to->objectName(), 0);
            return;
        }
        if (self->canKick(to)) {
            client()->agent()->action(QMdmmCore::Data::Kick, to->objectName(), 0);
            return;
        }
    }

    // Buy a horse for extra reach next round, but only while two slashes do not
    // yet finish every peer off: past that point what this bot is short of is
    // staying power, which the upgrade phase covers, not reach (Q3: no more
    // horses).
    if (!twoSlashesFinishEveryone() && self->canBuyHorse()) {
        client()->agent()->action(QMdmmCore::Data::BuyHorse, QString(), 0);
        return;
    }

    // Walk toward the peer this bot wants to act against (star map: every place
    // is adjacent only to Village, so X -> Village -> target). With no peer
    // worth aiming at the first opponent is marched on instead -- an unarmed
    // peer nobody holds a grudge against still goes down to a knife.
    QMdmmCore::Player *marchOn = nullptr;
    const QString targetName = selectTarget();
    if (!targetName.isEmpty())
        marchOn = client()->room()->player(targetName);
    const QList<QMdmmCore::Player *> others = opponents();
    if (marchOn == nullptr && !others.isEmpty())
        marchOn = others.first();
    if (marchOn != nullptr) {
        const int dest = (self->place() == QMdmmCore::Data::Village) ? marchOn->place() : QMdmmCore::Data::Village;
        if (self->canMove(dest)) {
            client()->agent()->action(QMdmmCore::Data::Move, QString(), dest);
            return;
        }
    }

    client()->agent()->action(QMdmmCore::Data::DoNothing, QString(), 0);
}

void KnifePreferredBot::handleUpgradeRequest(int remainingTimes)
{
    QMdmmCore::Player *self = selfPlayer();
    if (self == nullptr) {
        client()->agent()->giveUpRequest();
        return;
    }

    // Spend every point, knife first and horse last (Q3): the knife is what ends
    // rounds, max HP is what keeps this bot alive long enough to keep using it,
    // and the horse is the one weapon this style does not live on. If the total
    // remaining capacity is less than the points to spend, no feasible list
    // exists; give up and let the server fall back.
    int knife = self->upgradeKnifeRemainingTimes();
    int horse = self->upgradeHorseRemainingTimes();
    int maxHp = self->upgradeMaxHpRemainingTimes();
    if (knife + horse + maxHp < remainingTimes) {
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
    take(QMdmmCore::Data::UpgradeKnife, knife);
    take(QMdmmCore::Data::UpgradeMaxHp, maxHp);
    take(QMdmmCore::Data::UpgradeHorse, horse);
    client()->agent()->upgrade(items);
}

QMdmmCore::Player *KnifePreferredBot::attackTarget()
{
    const QMdmmCore::Room *room = client()->room();
    const QMdmmCore::Player *self = room->player(client()->objectName());
    if (self == nullptr)
        return nullptr;

    QMdmmCore::Player *firstHere = nullptr;
    QMdmmCore::Player *bestScored = nullptr;
    double bestScore = 0.0;

    // A strict comparison from the zero start keeps the first of several equally
    // good peers (room order) and leaves a peer that scores nothing to the
    // fallback below.
    for (QMdmmCore::Player *to : opponents()) {
        if (to->place() != self->place())
            continue;
        if (firstHere == nullptr)
            firstHere = to;
        const double score = targetScore(to->objectName());
        if (score > bestScore) {
            bestScore = score;
            bestScored = to;
        }
    }

    return (bestScored != nullptr) ? bestScored : firstHere;
}

bool KnifePreferredBot::twoSlashesFinishEveryone() const
{
    const QMdmmCore::Room *room = client()->room();
    const QMdmmCore::Player *self = room->player(client()->objectName());
    if (self == nullptr)
        return false;

    // Both sides upgrade as the match goes on, so this asks the question afresh
    // rather than remembering an answer from an earlier round: a peer is a
    // problem while two slashes of this bot's knife would still leave it
    // standing.
    const int knifeDamage = self->knifeDamage();
    const auto survivesTwoSlashes = [self, knifeDamage](const QMdmmCore::Player *peer) { return peer != self && knifeDamage * 2 < peer->maxHp(); };
    const QList<const QMdmmCore::Player *> alive = room->alivePlayers();
    return std::ranges::none_of(alive, survivesTwoSlashes);
}

bool KnifePreferredBot::slashIsWorthItsPunish(const QMdmmCore::Player *to) const
{
    const QMdmmCore::Room *room = client()->room();
    const QMdmmCore::Player *self = room->player(client()->objectName());
    if (self == nullptr || to == nullptr || !self->canSlash(to))
        return false;

    // The reckless blow is the knife style's own endgame (see
    // twoSlashesFinishEveryone()); before it arrives, a punished slash is not
    // worth this bot's life.
    if (!twoSlashesFinishEveryone())
        return false;

    // The slash has to finish the peer off right now: the kill, and the upgrade
    // point that comes with it, is what the bot's life buys.
    if (to->hp() > self->knifeDamage())
        return false;

    // The bot has to be the stronger side of the two -- Q3's "outmatches it":
    // it is one blow away from the kill, while the peer cannot finish this bot
    // off in one blow of its own. A peer that could is no trade at all.
    int peerBlow = 0;
    if (to->hasKnife())
        peerBlow = to->knifeDamage();
    if (to->hasHorse() && to->place() != QMdmmCore::Data::Village)
        peerBlow = qMax(peerBlow, to->horseDamage());
    if (peerBlow >= self->maxHp())
        return false;

    // And nobody else may be left to profit from the round this bot spends
    // dying: Q3's example is a duel, where that is all the slash costs.
    return room->alivePlayersCount() <= 2;
}
