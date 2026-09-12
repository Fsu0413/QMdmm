// SPDX-License-Identifier: AGPL-3.0-or-later

#include "test.h"

#include <QMdmmAgent>
#include <QMdmmClient>
#include <QMdmmData>
#include <QMdmmPlayer>
#include <QMdmmRoom>

#include <QTest>

#include "bot.h"

using namespace Qt::StringLiterals;

// NOLINTBEGIN
// Exempt from clang-tidy by policy; see AGENTS.md.

// The state under test (the revenge memory, the threat assessment) lives in the
// Bot base class, but its read accessors are protected and the four request
// handlers are pure virtual (a style subclass must implement its strategy
// there). This subclass only lifts the read accessors into public scope and
// supplies the minimal concrete request handlers; it deliberately leaves the
// notification handlers untouched, so the base class implementations are the
// ones under test here. The notifications are delivered through the public
// Agent API, which also pins the signal connections Bot makes in its
// constructor.
class ProbeBot final : public Bot
{
public:
    explicit ProbeBot(QMdmmNetworking::Client *parent)
        : Bot(parent)
    {
    }

    using Bot::revengeScore;
    using Bot::threatScore;

protected:
    void handleRockPaperScissorsRequest(const QStringList &playerNames, int strivedOrder) override
    {
        Q_UNUSED(playerNames);
        Q_UNUSED(strivedOrder);
    }

    void handleActionOrderRequest(const QList<int> &remainedOrders, int maximumOrder, int selectionNum) override
    {
        Q_UNUSED(remainedOrders);
        Q_UNUSED(maximumOrder);
        Q_UNUSED(selectionNum);
    }

    void handleActionRequest(int currentOrder) override
    {
        Q_UNUSED(currentOrder);
    }

    void handleUpgradeRequest(int remainingTimes) override
    {
        Q_UNUSED(remainingTimes);
    }
};

class tst_QMdmmBot : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE tst_QMdmmBot() = default;

private slots:
    // A grudge is earned by a hostile action aimed at this bot, and only then.
    void revenge_recordsHostileActionsOnly();

    // A grudge fades every finished round but outlives the round it was earned
    // in, and keeps accumulating across rounds.
    void revenge_decaysEveryRound();

    // Once a grudge has faded to nothing the entry is dropped, so the table
    // stays bounded over a long match.
    void revenge_dropsNegligibleEntries();

    // The threat one opponent poses is the damage of its weapons, discounted by
    // how far away it stands.
    void threat_sumsWeaponsDiscountedByDistance();

    // A peer that cannot hurt this bot -- because it is dead, or because it is
    // not in the room at all -- is no threat, and neither is a dead bot itself.
    void threat_ignoresDeadPlayersAndStrangers();
};

void tst_QMdmmBot::revenge_recordsHostileActionsOnly()
{
    QMdmmNetworking::Client client {QMdmmNetworking::ClientConfiguration::defaults()};
    ProbeBot bot {&client};

    // The client's objectName is its playerName, i.e. the name this bot is
    // known by (see Bot::selfPlayer()).
    const QString self = client.objectName();
    const QString attacker = u"attacker"_s;
    const QString bystander = u"bystander"_s;

    // Slash and Kick aimed at us are hostile, and every hit piles up.
    client.agent()->notifyAction(attacker, QMdmmCore::Data::Slash, self, 0);
    QCOMPARE(bot.revengeScore(attacker), 1.0);
    client.agent()->notifyAction(attacker, QMdmmCore::Data::Kick, self, 0);
    QCOMPARE(bot.revengeScore(attacker), 2.0);

    // Every attacker gets its own entry.
    client.agent()->notifyAction(bystander, QMdmmCore::Data::Slash, self, 0);
    QCOMPARE(bot.revengeScore(bystander), 1.0);
    QCOMPARE(bot.revengeScore(attacker), 2.0);

    // LetMove is not hostile (it moves a player but deals no damage), and a
    // hostile action aimed at somebody else is none of our business either.
    client.agent()->notifyAction(attacker, QMdmmCore::Data::LetMove, self, 0);
    client.agent()->notifyAction(attacker, QMdmmCore::Data::Slash, bystander, 0);
    client.agent()->notifyAction(attacker, QMdmmCore::Data::DoNothing, QString(), 0);
    QCOMPARE(bot.revengeScore(attacker), 2.0);

    // A peer that never attacked this bot holds no grudge at all.
    QCOMPARE(bot.revengeScore(u"innocent"_s), 0.0);
}

void tst_QMdmmBot::revenge_decaysEveryRound()
{
    QMdmmNetworking::Client client {QMdmmNetworking::ClientConfiguration::defaults()};
    ProbeBot bot {&client};

    const QString self = client.objectName();
    const QString attacker = u"attacker"_s;

    client.agent()->notifyAction(attacker, QMdmmCore::Data::Slash, self, 0);
    client.agent()->notifyAction(attacker, QMdmmCore::Data::Slash, self, 0);
    QCOMPARE(bot.revengeScore(attacker), 2.0);

    // Every finished round multiplies the grudge by the decay factor, and one
    // round is never enough to forget an attacker.
    const double earned = bot.revengeScore(attacker);
    client.agent()->notifyRoundOver();
    QVERIFY(qFuzzyCompare(bot.revengeScore(attacker), earned * 0.8));
    QVERIFY(bot.revengeScore(attacker) > 0.0);

    const double afterFirstRound = bot.revengeScore(attacker);
    client.agent()->notifyRoundOver();
    QVERIFY(qFuzzyCompare(bot.revengeScore(attacker), afterFirstRound * 0.8));

    // A grudge outlives its round: a new hit adds to what is left over.
    client.agent()->notifyAction(attacker, QMdmmCore::Data::Slash, self, 0);
    QVERIFY(qFuzzyCompare(bot.revengeScore(attacker), afterFirstRound * 0.8 + 1.0));

    // Rounds passing do not conjure up entries for peers that never attacked.
    client.agent()->notifyRoundOver();
    QCOMPARE(bot.revengeScore(u"innocent"_s), 0.0);
}

void tst_QMdmmBot::revenge_dropsNegligibleEntries()
{
    QMdmmNetworking::Client client {QMdmmNetworking::ClientConfiguration::defaults()};
    ProbeBot bot {&client};

    const QString self = client.objectName();
    const QString attacker = u"attacker"_s;

    client.agent()->notifyAction(attacker, QMdmmCore::Data::Slash, self, 0);

    // 20 rounds leave 0.8^20 = 0.0115..., still above the drop threshold, so
    // the grudge is expected to survive them.
    for (int round = 0; round < 20; ++round)
        client.agent()->notifyRoundOver();
    QVERIFY(bot.revengeScore(attacker) > 0.0);

    // One more round pushes it to 0.8^21 = 0.0092..., at or below the
    // threshold, and the entry is dropped.
    client.agent()->notifyRoundOver();
    QCOMPARE(bot.revengeScore(attacker), 0.0);

    // A dropped entry starts over rather than leaving a residue behind.
    client.agent()->notifyAction(attacker, QMdmmCore::Data::Slash, self, 0);
    QCOMPARE(bot.revengeScore(attacker), 1.0);
}

void tst_QMdmmBot::threat_sumsWeaponsDiscountedByDistance()
{
    QMdmmNetworking::Client client {QMdmmNetworking::ClientConfiguration::defaults()};
    ProbeBot bot {&client};

    // The threat is read off the room mirror, so the room has to hold both this
    // bot (under the client's objectName, see Bot::selfPlayer()) and the peer.
    QMdmmCore::Room *room = client.room();
    QMdmmCore::Player *self = room->addPlayer(client.objectName());
    QMdmmCore::Player *enemy = room->addPlayer(u"enemy"_s);
    QVERIFY(self != nullptr);
    QVERIFY(enemy != nullptr);

    enemy->setKnifeDamage(3);
    enemy->setHorseDamage(2);

    // Unarmed, an opponent is harmless however close it stands.
    self->setPlace(1);
    enemy->setPlace(1);
    QCOMPARE(bot.threatScore(u"enemy"_s), 0.0);

    // Sharing this bot's place, an opponent brings both weapons to bear...
    enemy->setHasKnife(true);
    QCOMPARE(bot.threatScore(u"enemy"_s), 3.0);
    enemy->setHasHorse(true);
    QCOMPARE(bot.threatScore(u"enemy"_s), 5.0);

    // ...except inside the Village, where a horse cannot be used.
    self->setPlace(QMdmmCore::Data::Village);
    enemy->setPlace(QMdmmCore::Data::Village);
    QCOMPARE(bot.threatScore(u"enemy"_s), 3.0);

    // A merely adjacent opponent has to move in first, so its hit lands a round
    // later and counts for half. It lands where this bot stands: stepping into a
    // City it can still kick,
    self->setPlace(1);
    enemy->setPlace(QMdmmCore::Data::Village);
    QCOMPARE(bot.threatScore(u"enemy"_s), 2.5);

    // but stepping into the Village it cannot.
    self->setPlace(QMdmmCore::Data::Village);
    enemy->setPlace(1);
    QCOMPARE(bot.threatScore(u"enemy"_s), 1.5);

    // Two Cities are not adjacent, so an opponent standing in one of them
    // cannot reach this bot within a round.
    self->setPlace(1);
    enemy->setPlace(2);
    QCOMPARE(bot.threatScore(u"enemy"_s), 0.0);
}

void tst_QMdmmBot::threat_ignoresDeadPlayersAndStrangers()
{
    QMdmmNetworking::Client client {QMdmmNetworking::ClientConfiguration::defaults()};
    ProbeBot bot {&client};

    QMdmmCore::Room *room = client.room();
    QMdmmCore::Player *self = room->addPlayer(client.objectName());
    QMdmmCore::Player *enemy = room->addPlayer(u"enemy"_s);
    QVERIFY(self != nullptr);
    QVERIFY(enemy != nullptr);

    self->setPlace(1);
    enemy->setPlace(1);
    enemy->setHasKnife(true);
    enemy->setKnifeDamage(3);
    QCOMPARE(bot.threatScore(u"enemy"_s), 3.0);

    // A dead opponent is harmless even at arm's length.
    enemy->setHp(0);
    QCOMPARE(bot.threatScore(u"enemy"_s), 0.0);

    // So is a peer that is not in the room at all.
    QCOMPARE(bot.threatScore(u"stranger"_s), 0.0);

    // And a bot that is itself dead is threatened by nobody.
    enemy->setHp(10);
    self->setHp(0);
    QCOMPARE(bot.threatScore(u"enemy"_s), 0.0);
}

namespace {
RegisterTestObject<tst_QMdmmBot> _;
}
#include "tst_qmdmmbot.moc"

// NOLINTEND
