// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Headless smoke test for the networked gameplay loop used by the GUI's
// "local game" mode: an in-process Server plus N Clients (1 human + bots)
// connected over the loopback TCP socket. Verifies the full
// SSC -> action-order -> action -> upgrade -> round/game over pipeline actually
// runs to completion without a human in the loop.
//
// The match is intentionally configured to be tiny so it converges quickly:
// one hit kills (initialMaxHp = 1) and every stat needs a single upgrade to
// max out, so the first player to land three kills and spend the points wins.

#include <QCoreApplication>
#include <QTimer>
#include <QRandomGenerator>
#include <QDebug>

#include <QMdmmServer>
#include <QMdmmClient>
#include <QMdmmRoom>
#include <QMdmmPlayer>
#include <QMdmmLogicConfiguration>

using namespace QMdmmCore;
using namespace QMdmmNetworking;

namespace {
constexpr char LOCAL_HOST[] = "qmdmm://localhost:6366";

void wireBot(Client *bot)
{
    QObject::connect(bot, &Client::requestStoneScissorsCloth, bot, [bot]() {
        bot->replyStoneScissorsCloth(static_cast<Data::StoneScissorsCloth>(QRandomGenerator::global()->generate() % 3));
    });
    QObject::connect(bot, &Client::requestActionOrder, bot,
            [bot](const QList<int> &remainedOrders, int, int selectionNum) {
                QList<int> ao;
                ao.reserve(selectionNum);
                for (int i = 0; i < selectionNum && i < remainedOrders.size(); ++i)
                    ao.append(remainedOrders.at(i));
                bot->replyActionOrder(ao);
            });
    // A competent auto-player:
    //   1. Buy a knife (must be off Country).
    //   2. Slash a co-located enemy.
    //   3. Otherwise walk toward an enemy (star map: every place is adjacent
    //      only to Country, so X -> Country -> target).
    QObject::connect(bot, &Client::requestAction, bot, [bot]() {
        const QString self = bot->objectName();
        Room *room = bot->room();
        if (room == nullptr) {
            bot->replyAction(Data::DoNothing, {}, 0);
            return;
        }
        Player *me = room->player(self);
        if (me == nullptr || !me->alive()) {
            bot->replyAction(Data::DoNothing, {}, 0);
            return;
        }
        if (!me->hasKnife()) {
            if (me->canBuyKnife()) {
                bot->replyAction(Data::BuyKnife, {}, 0);
                return;
            }
            // Can't buy right now (e.g. standing in Country) -> step to any
            // non-Country place so we can buy next turn.
            for (int p = 1; p < room->logicConfiguration().playerNumPerRoom() + 1; ++p) {
                if (me->canMove(p)) {
                    bot->replyAction(Data::Move, {}, p);
                    return;
                }
            }
            bot->replyAction(Data::DoNothing, {}, 0);
            return;
        }
        // Slash a co-located enemy if any.
        for (Player *p : room->players())
            if (p->alive() && p->objectName() != self && p->place() == me->place()) {
                bot->replyAction(Data::Slash, p->objectName(), -1);
                return;
            }
        // Otherwise step toward an enemy (star graph: via Country).
        for (Player *p : room->players())
            if (p->alive() && p->objectName() != self) {
                const int dest = (me->place() == Data::Country) ? p->place() : Data::Country;
                bot->replyAction(Data::Move, {}, dest);
                return;
            }
        bot->replyAction(Data::DoNothing, {}, 0);
    });
    // Spend every earned upgrade point. The game only ends when a player has
    // maxed out knife + horse + max HP, so we must actually upgrade.
    QObject::connect(bot, &Client::requestUpgrade, bot, [bot](int remainingTimes) {
        QList<Data::UpgradeItem> ups;
        if (Room *room = bot->room()) {
            if (Player *me = room->player(bot->objectName())) {
                int budget = remainingTimes;
                while (budget-- > 0) {
                    if (me->canUpgradeKnife())
                        ups << Data::UpgradeKnife;
                    else if (me->canUpgradeHorse())
                        ups << Data::UpgradeHorse;
                    else if (me->canUpgradeMaxHp())
                        ups << Data::UpgradeMaxHp;
                    else
                        break; // already maxed everything; this player would win
                }
            }
        }
        bot->replyUpgrade(ups);
    });
}
} // namespace

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    const int playerCount = 2;
    int rounds = 0;
    int gameOvers = 0;
    bool ok = true;

    // Tiny, fast-to-converge configuration for the unattended smoke match.
    LogicConfiguration conf = LogicConfiguration::defaults();
    conf.setPlayerNumPerRoom(playerCount);
    conf.setInitialMaxHp(1);       // one hit kills -> rounds can actually end
    conf.setMaximumMaxHp(2);       // 1 upgrade to max
    conf.setInitialKnifeDamage(1);
    conf.setMaximumKnifeDamage(2); // 1 upgrade to max
    conf.setInitialHorseDamage(1);
    conf.setMaximumHorseDamage(2); // 1 upgrade to max
    conf.setPunishHpModifier(0);   // keep the attacker alive when slashing
    conf.setRequestTimeout(100);   // be lenient in the headless environment

    auto *server = new Server(ServerConfiguration::defaults(), conf, &app);
    if (!server->listen()) {
        qWarning() << "smoke: server listen failed";
        return 2;
    }

    // 1 human + (playerCount-1) bots. The human is also auto-driven here so the
    // whole match can run unattended.
    auto *human = new Client(ClientConfiguration(), &app);
    wireBot(human);
    QObject::connect(human, &Client::notifyGameStart, &app, [&]() {
        qDebug() << "smoke: game started";
    });
    QObject::connect(human, &Client::notifyRoundStart, &app, [&]() {
        ++rounds;
        qDebug() << "smoke: round" << rounds << "started";
    });
    QObject::connect(human, &Client::notifyRoundOver, &app, [&]() {
        qDebug() << "smoke: round over";
    });
    QObject::connect(human, &Client::notifyGameOver, &app, [&](const QStringList &winners) {
        ++gameOvers;
        qDebug() << "smoke: GAME OVER, winners:" << winners;
        QTimer::singleShot(0, &app, &QCoreApplication::quit);
    });
    QObject::connect(human, &Client::socketErrorDisconnected, &app, [&](const QString &err) {
        qWarning() << "smoke: socket error:" << err;
        ok = false;
    });

    human->connectToHost(QString::fromLatin1(LOCAL_HOST), Data::StateOnline);

    for (int i = 1; i < playerCount; ++i) {
        auto *bot = new Client(ClientConfiguration(), &app);
        wireBot(bot);
        bot->connectToHost(QString::fromLatin1(LOCAL_HOST), Data::StateOnlineBot);
    }

    // Safety timeout: the match must finish within 60s or something is stuck.
    QTimer::singleShot(60000, &app, [&]() {
        qWarning() << "smoke: TIMEOUT - match did not finish";
        ok = false;
        app.quit();
    });

    const int rc = app.exec();

    qDebug() << "smoke: rounds played =" << rounds
             << "gameOvers =" << gameOvers
             << "socketError =" << !ok;

    if (!ok) return 3;
    if (gameOvers == 0) {
        qWarning() << "smoke: game never reached game over";
        return 4;
    }
    qDebug() << "smoke: PASS - networked local-game loop completed";
    return rc;
}
