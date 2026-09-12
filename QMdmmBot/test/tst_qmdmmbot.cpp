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

// The state under test (the revenge memory, the threat assessment, the target
// selection built on them) lives in the Bot base class, but its read accessors
// are protected and the four request handlers are pure virtual (a style
// subclass must implement its strategy there). This subclass only lifts the
// read accessors into public scope and supplies the minimal concrete request
// handlers; it deliberately leaves the notification handlers untouched, so the
// base class implementations are the ones under test here. The notifications are
// delivered through the public Agent API, which also pins the signal
// connections Bot makes in its constructor.
class ProbeBot final : public Bot
{
public:
    explicit ProbeBot(QMdmmNetworking::Client *parent)
        : Bot(parent)
    {
    }

    using Bot::canSlashSafely;
    using Bot::logicConfiguration;
    using Bot::revengeScore;
    using Bot::selectTarget;
    using Bot::targetScore;
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

namespace {

// What a bot answered to one action request.
struct ActionReply
{
    int count = 0;
    QMdmmCore::Data::Action action = QMdmmCore::Data::DoNothing;
    QString toPlayer;
    int toPlace = 0;
};

// Asks a bot for an action the way the match does: the server's request reaches
// the client, which hands it to the agent, which raises it as a signal -- the
// same signal the bot answered above. What comes back out is the reply signal,
// recorded here together with the player and place it names.
ActionReply askForAction(QMdmmNetworking::Client &client, int currentOrder)
{
    ActionReply reply;
    const auto record = [&reply](QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace) {
        ++reply.count;
        reply.action = action;
        reply.toPlayer = toPlayer;
        reply.toPlace = toPlace;
    };
    const QMetaObject::Connection connection = QObject::connect(client.agent(), &QMdmmNetworking::Agent::replyAction, &client, record);
    client.agent()->requestAction(currentOrder);
    QObject::disconnect(connection);
    return reply;
}

} // namespace

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

    // How attractive a peer is as a target is its grudge plus the threat it
    // poses; death zeroes the threat but not the grudge.
    void target_combinesRevengeAndThreat();

    // The target is the opponent with the highest score, on equal weight for the
    // two dimensions, and a tie goes to room order.
    void target_picksTheHighestScoringOpponent();

    // Nobody scores above zero -- or nobody alive is left to act against -- means
    // no target at all.
    void target_returnsEmptyWhenNobodyIsWorthAimingAt();

    // Where a slash happens decides what it costs: a city charges the slasher its
    // own HP, the Village charges nothing, and the rules decide whether there is a
    // punish at all.
    void punish_isChargedInCitiesAndNotInTheVillage();

    // A slash is skipped when its punish would finish the slasher off, and taken
    // when it would not.
    void slash_isSkippedWhenItsPunishWouldBeFatal();

    // The styles answer an action request with that rule in force: they pass up a
    // co-located attack that a city would punish them to death for.
    void action_skipsASlashThatTheCityPunishWouldMakeFatal();
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

void tst_QMdmmBot::target_combinesRevengeAndThreat()
{
    QMdmmNetworking::Client client {QMdmmNetworking::ClientConfiguration::defaults()};
    ProbeBot bot {&client};

    const QString self = client.objectName();
    const QString enemy = u"enemy"_s;

    QMdmmCore::Room *room = client.room();
    QMdmmCore::Player *selfPlayer = room->addPlayer(self);
    QMdmmCore::Player *enemyPlayer = room->addPlayer(enemy);
    QVERIFY(selfPlayer != nullptr);
    QVERIFY(enemyPlayer != nullptr);

    selfPlayer->setPlace(1);
    enemyPlayer->setPlace(1);

    // An unarmed peer that never attacked this bot is worth nothing as a target,
    // and so is a peer that is not in the room at all.
    QCOMPARE(bot.targetScore(enemy), 0.0);
    QCOMPARE(bot.targetScore(u"stranger"_s), 0.0);

    // The threat it poses is what it brings to bear...
    enemyPlayer->setKnifeDamage(3);
    enemyPlayer->setHasKnife(true);
    QCOMPARE(bot.targetScore(enemy), 3.0);

    // ...and a grudge adds on top of that.
    client.agent()->notifyAction(enemy, QMdmmCore::Data::Slash, self, 0);
    QCOMPARE(bot.revengeScore(enemy), 1.0);
    QCOMPARE(bot.targetScore(enemy), 4.0);

    // Death zeroes the threat but not the grudge: the bot still remembers who
    // wronged it.
    enemyPlayer->setHp(0);
    QCOMPARE(bot.threatScore(enemy), 0.0);
    QCOMPARE(bot.targetScore(enemy), 1.0);
}

void tst_QMdmmBot::target_picksTheHighestScoringOpponent()
{
    QMdmmNetworking::Client client {QMdmmNetworking::ClientConfiguration::defaults()};
    ProbeBot bot {&client};

    const QString self = client.objectName();

    QMdmmCore::Room *room = client.room();
    QMdmmCore::Player *selfPlayer = room->addPlayer(self);
    // Room order is name order, which is what decides a tie: "bbb" comes before
    // "ccc".
    QMdmmCore::Player *bbb = room->addPlayer(u"bbb"_s);
    QMdmmCore::Player *ccc = room->addPlayer(u"ccc"_s);
    QVERIFY(selfPlayer != nullptr);
    QVERIFY(bbb != nullptr);
    QVERIFY(ccc != nullptr);

    selfPlayer->setPlace(1);
    bbb->setPlace(1);
    ccc->setPlace(1);

    // Nobody has wronged this bot and nobody is armed, so there is nothing to
    // pick.
    QCOMPARE(bot.selectTarget(), QString());

    // "ccc" attacked us, which alone puts it ahead of the harmless "bbb".
    client.agent()->notifyAction(u"ccc"_s, QMdmmCore::Data::Slash, self, 0);
    QCOMPARE(bot.selectTarget(), u"ccc"_s);

    // A knife, however, makes "bbb" the bigger threat and hands it the lead.
    bbb->setKnifeDamage(3);
    bbb->setHasKnife(true);
    QCOMPARE(bot.selectTarget(), u"bbb"_s);

    // Both dimensions are summed at equal weight, so two more hits put the
    // grudge against "ccc" at exactly the threat of "bbb" -- and a tie goes to
    // room order, i.e. to "bbb".
    client.agent()->notifyAction(u"ccc"_s, QMdmmCore::Data::Slash, self, 0);
    client.agent()->notifyAction(u"ccc"_s, QMdmmCore::Data::Slash, self, 0);
    QCOMPARE(bot.targetScore(u"bbb"_s), 3.0);
    QCOMPARE(bot.targetScore(u"ccc"_s), 3.0);
    QCOMPARE(bot.selectTarget(), u"bbb"_s);

    // One more hit and "ccc" pulls ahead on its own.
    client.agent()->notifyAction(u"ccc"_s, QMdmmCore::Data::Slash, self, 0);
    QCOMPARE(bot.targetScore(u"ccc"_s), 4.0);
    QCOMPARE(bot.selectTarget(), u"ccc"_s);
}

void tst_QMdmmBot::target_returnsEmptyWhenNobodyIsWorthAimingAt()
{
    QMdmmNetworking::Client client {QMdmmNetworking::ClientConfiguration::defaults()};
    ProbeBot bot {&client};

    const QString self = client.objectName();
    const QString enemy = u"enemy"_s;

    QMdmmCore::Room *room = client.room();
    QMdmmCore::Player *selfPlayer = room->addPlayer(self);
    QMdmmCore::Player *enemyPlayer = room->addPlayer(enemy);
    QVERIFY(selfPlayer != nullptr);
    QVERIFY(enemyPlayer != nullptr);

    // A peer that is out of reach, unarmed and holds no grudge is not worth
    // aiming at.
    selfPlayer->setPlace(1);
    enemyPlayer->setPlace(2);
    QCOMPARE(bot.targetScore(enemy), 0.0);
    QCOMPARE(bot.selectTarget(), QString());

    // Once it has wronged this bot there is something to act on...
    client.agent()->notifyAction(enemy, QMdmmCore::Data::Slash, self, 0);
    QCOMPARE(bot.selectTarget(), enemy);

    // ...but a dead peer is out of the running even while the grudge survives:
    // only the living can be acted against.
    enemyPlayer->setHp(0);
    QCOMPARE(bot.targetScore(enemy), 1.0);
    QCOMPARE(bot.selectTarget(), QString());

    // A bot whose room holds nobody but itself has no target either.
    QMdmmNetworking::Client lonelyClient {QMdmmNetworking::ClientConfiguration::defaults()};
    ProbeBot lonelyBot {&lonelyClient};
    QMdmmCore::Room *lonelyRoom = lonelyClient.room();
    QVERIFY(lonelyRoom->addPlayer(lonelyClient.objectName()) != nullptr);
    QCOMPARE(lonelyBot.selectTarget(), QString());
}

void tst_QMdmmBot::punish_isChargedInCitiesAndNotInTheVillage()
{
    QMdmmNetworking::Client client {QMdmmNetworking::ClientConfiguration::defaults()};
    ProbeBot bot {&client};

    QMdmmCore::Room *room = client.room();
    QMdmmCore::Player *self = room->addPlayer(client.objectName());
    QMdmmCore::Player *enemy = room->addPlayer(u"enemy"_s);
    QVERIFY(self != nullptr);
    QVERIFY(enemy != nullptr);

    // The rules a strategy reads are the ones the room holds -- the same ones the
    // players are ruled by.
    QMdmmCore::LogicConfiguration configuration;
    configuration.setPunishHpModifier(2);
    configuration.setPunishHpRoundStrategy(QMdmmCore::LogicConfiguration::RoundDown);
    room->setLogicConfiguration(configuration);
    QCOMPARE(bot.logicConfiguration().punishHpModifier(), 2);

    self->setMaxHp(10);
    self->setHp(10);
    self->setHasKnife(true);
    self->setKnifeDamage(1);
    enemy->setMaxHp(10);
    enemy->setHp(10);

    // In a city a slash is punished with a share of the slasher's own max HP...
    self->setPlace(1);
    enemy->setPlace(1);
    QCOMPARE(self->slashPunishHp(), 5);

    // ...and the engine charges exactly that: the slash costs this player the
    // punish on top of the hit it lands.
    QVERIFY(self->slash(enemy));
    QCOMPARE(self->hp(), 5);
    QCOMPARE(enemy->hp(), 9);

    // Inside the Village the very same slash is free.
    self->setHp(10);
    self->setPlace(QMdmmCore::Data::Village);
    enemy->setPlace(QMdmmCore::Data::Village);
    QCOMPARE(self->slashPunishHp(), 0);
    QVERIFY(self->slash(enemy));
    QCOMPARE(self->hp(), 10);
    QCOMPARE(enemy->hp(), 8);

    // And with the punish rule called off, no city charges either.
    configuration.setPunishHpModifier(0);
    room->setLogicConfiguration(configuration);
    self->setPlace(1);
    enemy->setPlace(1);
    QCOMPARE(self->slashPunishHp(), 0);
}

void tst_QMdmmBot::slash_isSkippedWhenItsPunishWouldBeFatal()
{
    QMdmmNetworking::Client client {QMdmmNetworking::ClientConfiguration::defaults()};
    ProbeBot bot {&client};

    QMdmmCore::Room *room = client.room();
    QMdmmCore::Player *self = room->addPlayer(client.objectName());
    QMdmmCore::Player *enemy = room->addPlayer(u"enemy"_s);
    QVERIFY(self != nullptr);
    QVERIFY(enemy != nullptr);

    QMdmmCore::LogicConfiguration configuration;
    configuration.setPunishHpModifier(2);
    configuration.setPunishHpRoundStrategy(QMdmmCore::LogicConfiguration::RoundDown);
    room->setLogicConfiguration(configuration);

    self->setMaxHp(10);
    self->setHp(6);
    self->setHasKnife(true);
    self->setPlace(1);
    enemy->setHp(10);
    enemy->setPlace(1);

    // A slash its owner walks away from is taken...
    QCOMPARE(self->slashPunishHp(), 5);
    QVERIFY(bot.canSlashSafely(enemy));

    // ...but one that would be paid for with this bot's life is not, however
    // healthy the peer happens to be.
    self->setHp(5);
    QVERIFY(!bot.canSlashSafely(enemy));

    // Where zero HP still counts as alive, zero is survivable after all.
    configuration.setZeroHpAsDead(false);
    room->setLogicConfiguration(configuration);
    QVERIFY(bot.canSlashSafely(enemy));
    self->setHp(4);
    QVERIFY(!bot.canSlashSafely(enemy));
    configuration.setZeroHpAsDead(true);
    room->setLogicConfiguration(configuration);

    // Without a knife there is nothing to slash in the first place.
    self->setHasKnife(false);
    QVERIFY(!bot.canSlashSafely(enemy));
    self->setHasKnife(true);

    // A slash inside the Village costs nothing, so even a bot on its last HP
    // takes it.
    self->setPlace(QMdmmCore::Data::Village);
    enemy->setPlace(QMdmmCore::Data::Village);
    self->setHp(1);
    QVERIFY(bot.canSlashSafely(enemy));

    // A peer that is not in the room is nothing to slash at.
    QVERIFY(!bot.canSlashSafely(nullptr));

    // And a bot that has not signed in yet has no self player to slash with.
    QMdmmNetworking::Client otherClient {QMdmmNetworking::ClientConfiguration::defaults()};
    ProbeBot otherBot {&otherClient};
    QVERIFY(!otherBot.canSlashSafely(enemy));
}

void tst_QMdmmBot::action_skipsASlashThatTheCityPunishWouldMakeFatal()
{
    // Both styles still answer out of the same shared placeholder, so both are
    // asked to show that the place rules reach the reply they send.
    const QStringList styles = {u"knifePreferred"_s, u"horsePreferred"_s};

    for (const QString &style : styles) {
        QMdmmNetworking::Client client {QMdmmNetworking::ClientConfiguration::defaults()};
        Bot *bot = Bot::createBot(style, &client);
        QVERIFY(bot != nullptr);

        QMdmmCore::Room *room = client.room();
        QMdmmCore::Player *self = room->addPlayer(client.objectName());
        QMdmmCore::Player *enemy = room->addPlayer(u"enemy"_s);
        QVERIFY(self != nullptr);
        QVERIFY(enemy != nullptr);

        // A city that charges half of the slasher's max HP for a slash.
        QMdmmCore::LogicConfiguration configuration;
        configuration.setPunishHpModifier(2);
        configuration.setPunishHpRoundStrategy(QMdmmCore::LogicConfiguration::RoundDown);
        room->setLogicConfiguration(configuration);

        self->setMaxHp(10);
        self->setHp(6);
        self->setHasKnife(true);
        self->setKnifeDamage(1);
        self->setPlace(1);
        enemy->setHp(10);
        enemy->setPlace(1);

        // HP to spare: the bot attacks the opponent standing next to it.
        const ActionReply attack = askForAction(client, 1);
        QCOMPARE(attack.count, 1);
        QCOMPARE(attack.action, QMdmmCore::Data::Slash);
        QCOMPARE(attack.toPlayer, enemy->objectName());

        // One HP less and that same slash would be punished with this bot's life:
        // the charge is 5 and it has exactly 5 HP left. It spends the round
        // elsewhere instead -- on the horse it does not have yet -- rather than
        // trade its life for the hit.
        self->setHp(5);
        const ActionReply skips = askForAction(client, 1);
        QCOMPARE(skips.count, 1);
        QVERIFY(skips.action != QMdmmCore::Data::Slash);
        QCOMPARE(skips.action, QMdmmCore::Data::BuyHorse);

        // The same last HP is no reason to hold back inside the Village, where a
        // slash is free.
        self->setHp(1);
        self->setPlace(QMdmmCore::Data::Village);
        enemy->setPlace(QMdmmCore::Data::Village);
        const ActionReply freeSlash = askForAction(client, 1);
        QCOMPARE(freeSlash.count, 1);
        QCOMPARE(freeSlash.action, QMdmmCore::Data::Slash);
        QCOMPARE(freeSlash.toPlayer, enemy->objectName());
    }
}

namespace {
RegisterTestObject<tst_QMdmmBot> _;
}
#include "tst_qmdmmbot.moc"

// NOLINTEND
