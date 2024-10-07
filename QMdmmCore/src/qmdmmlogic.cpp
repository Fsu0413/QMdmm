// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmlogic.h"
#include "qmdmmlogic_p.h"

#include "qmdmmplayer.h"
#include "qmdmmroom.h"

#include <QHash>
#include <QJsonObject>
#include <QJsonValue>
#include <QMultiHash>

#include <utility>

const QMdmmLogicConfiguration &QMdmmLogicConfiguration::defaults()
{
    // clang-format off
    static const QMdmmLogicConfiguration defaultInstance {
        qMakePair(QStringLiteral("playerNumPerRoom"), 3),
        qMakePair(QStringLiteral("requestTimeout"), 20),
        qMakePair(QStringLiteral("initialKnifeDamage"), 1),
        qMakePair(QStringLiteral("maximumKnifeDamage"), 10),
        qMakePair(QStringLiteral("initialHorseDamage"), 2),
        qMakePair(QStringLiteral("maximumHorseDamage"), 10),
        qMakePair(QStringLiteral("initialMaxHp"), 10),
        qMakePair(QStringLiteral("maximumMaxHp"), 20),
        qMakePair(QStringLiteral("punishHpModifier"), 2),
        qMakePair(QStringLiteral("punishHpRoundStrategy"), static_cast<int>(RoundToNearest45)),
        qMakePair(QStringLiteral("zeroHpAsDead"), true),
        qMakePair(QStringLiteral("enableLetMove"), true),
        qMakePair(QStringLiteral("canBuyOnlyInInitialCity"), false),
    };
    // clang-format on

    return defaultInstance;
}

#define CONVERTTOTYPEBOOL(v) v.toBool()
#define CONVERTTOTYPEINT(v) v.toInt()
#define CONVERTTOTYPEPUNISHHPROUNDSTRATEGY(v) static_cast<QMdmmLogicConfiguration::PunishHpRoundStrategy>(v.toInt())
#define REALIZE_CONFIGURATION(type, valueName, ValueName, convertToType, convertToJsonValue) \
    type QMdmmLogicConfiguration::valueName() const                                          \
    {                                                                                        \
        if (contains(QStringLiteral(#valueName)))                                            \
            return convertToType(value(QStringLiteral(#valueName)));                         \
        return convertToType(defaults().value(QStringLiteral(#valueName)));                  \
    }                                                                                        \
    void QMdmmLogicConfiguration::set##ValueName(type valueName)                             \
    {                                                                                        \
        insert(QStringLiteral(#valueName), convertToJsonValue(valueName));                   \
    }

REALIZE_CONFIGURATION(int, playerNumPerRoom, PlayerNumPerRoom, CONVERTTOTYPEINT, )
REALIZE_CONFIGURATION(int, requestTimeout, RequestTimeout, CONVERTTOTYPEINT, )
REALIZE_CONFIGURATION(int, initialKnifeDamage, InitialKnifeDamage, CONVERTTOTYPEINT, )
REALIZE_CONFIGURATION(int, maximumKnifeDamage, MaximumKnifeDamage, CONVERTTOTYPEINT, )
REALIZE_CONFIGURATION(int, initialHorseDamage, InitialHorseDamage, CONVERTTOTYPEINT, )
REALIZE_CONFIGURATION(int, maximumHorseDamage, MaximumHorseDamage, CONVERTTOTYPEINT, )
REALIZE_CONFIGURATION(int, initialMaxHp, InitialMaxHp, CONVERTTOTYPEINT, )
REALIZE_CONFIGURATION(int, maximumMaxHp, MaximumMaxHp, CONVERTTOTYPEINT, )
REALIZE_CONFIGURATION(int, punishHpModifier, PunishHpModifier, CONVERTTOTYPEINT, )
REALIZE_CONFIGURATION(QMdmmLogicConfiguration::PunishHpRoundStrategy, punishHpRoundStrategy, PunishHpRoundStrategy, CONVERTTOTYPEPUNISHHPROUNDSTRATEGY, static_cast<int>)
REALIZE_CONFIGURATION(bool, zeroHpAsDead, ZeroHpAsDead, CONVERTTOTYPEBOOL, )
REALIZE_CONFIGURATION(bool, enableLetMove, EnableLetMove, CONVERTTOTYPEBOOL, )
REALIZE_CONFIGURATION(bool, canBuyOnlyInInitialCity, CanBuyOnlyInInitialCity, CONVERTTOTYPEBOOL, )

#undef REALIZE_CONFIGURATION
#undef CONVERTTOTYPEPUNISHHPROUNDSTRATEGY
#undef CONVERTTOTYPEINT
#undef CONVERTTOTYPEBOOL

bool QMdmmLogicConfiguration::deserialize(const QJsonValue &value) // NOLINT(readability-function-cognitive-complexity)
{
    if (!value.isObject())
        return false;

    QJsonObject ob = value.toObject();
    QJsonObject result;

#define CONVERTPUNISHHPROUNDSTRATEGY() static_cast<PunishHpRoundStrategy>(v.toInt())
#define CONF(member, check)                                                                    \
    {                                                                                          \
        if (ob.contains(QStringLiteral(#member))) {                                            \
            QJsonValue v = ob.value(QStringLiteral(#member));                                  \
            if (v.check())                                                                     \
                result.insert(QStringLiteral(#member), v);                                     \
            else                                                                               \
                return false;                                                                  \
        } else {                                                                               \
            result.insert(QStringLiteral(#member), defaults().value(QStringLiteral(#member))); \
        }                                                                                      \
    }

    CONF(playerNumPerRoom, isDouble);
    CONF(requestTimeout, isDouble);
    CONF(initialKnifeDamage, isDouble);
    CONF(maximumKnifeDamage, isDouble);
    CONF(initialHorseDamage, isDouble);
    CONF(maximumHorseDamage, isDouble);
    CONF(initialMaxHp, isDouble);
    CONF(maximumMaxHp, isDouble);
    CONF(punishHpModifier, isDouble);
    CONF(punishHpRoundStrategy, isDouble);
    CONF(zeroHpAsDead, isBool);
    CONF(enableLetMove, isBool);

#undef CONF
#undef CONVERTPUNISHHPROUNDSTRATEGY

    *this = result;
    return true;
}

QMdmmLogicPrivate::QMdmmLogicPrivate(QMdmmLogic *q, QMdmmLogicConfiguration logicConfiguration)
    : q(q)
    , conf(std::move(logicConfiguration))
    , room(new QMdmmRoom(q))
    , state(QMdmmLogic::BeforeRoundStart)
    , currentStrivingActionOrder(0)
    , currentActionOrder(0)
{
}

bool QMdmmLogicPrivate::actionFeasible(const QString &fromPlayer, QMdmmData::Action action, const QString &toPlayer, int toPlace) const
{
    const QMdmmPlayer *from = room->player(fromPlayer);
    switch (action) {
    case QMdmmData::DoNothing: {
        return true;
    }
    case QMdmmData::BuyKnife: {
        return from->canBuyKnife();
    }
    case QMdmmData::BuyHorse: {
        return from->canBuyHorse();
    }
    case QMdmmData::Slash: {
        const QMdmmPlayer *to = room->player(toPlayer);
        return from->canSlash(to);
    }
    case QMdmmData::Kick: {
        const QMdmmPlayer *to = room->player(toPlayer);
        return from->canKick(to);
    }
    case QMdmmData::Move: {
        return from->canMove(toPlace);
    }
    case QMdmmData::LetMove: {
        const QMdmmPlayer *to = room->player(toPlayer);
        return from->canLetMove(to, toPlace);
    }
    default:
        break;
    }

    return false;
}

// NOLINTNEXTLINE(readability-make-member-function-const): Action is ought not to be const
bool QMdmmLogicPrivate::applyAction(const QString &fromPlayer, QMdmmData::Action action, const QString &toPlayer, int toPlace)
{
    emit q->actionResult(fromPlayer, action, toPlayer, toPlace, QMdmmLogic::QPrivateSignal());

    QMdmmPlayer *from = room->player(fromPlayer);
    switch (action) {
    case QMdmmData::DoNothing: {
        return from->doNothing();
    }
    case QMdmmData::BuyKnife: {
        return from->buyKnife();
    }
    case QMdmmData::BuyHorse: {
        return from->buyHorse();
    }
    case QMdmmData::Slash: {
        QMdmmPlayer *to = room->player(toPlayer);
        return from->slash(to);
    }
    case QMdmmData::Kick: {
        QMdmmPlayer *to = room->player(toPlayer);
        return from->kick(to);
    }
    case QMdmmData::Move: {
        return from->move(toPlace);
    }
    case QMdmmData::LetMove: {
        QMdmmPlayer *to = room->player(toPlayer);
        return from->letMove(to, toPlace);
    }
    default:
        break;
    }

    return false;
}

void QMdmmLogicPrivate::startSscForAction()
{
    sscForActionReplies.clear();
    sscForActionWinners.clear();
    state = QMdmmLogic::SscForAction;
    emit q->requestSscForAction(room->alivePlayerNames(), QMdmmLogic::QPrivateSignal());
}

void QMdmmLogicPrivate::sscForAction()
{
    if (sscForActionReplies.count() == room->alivePlayersCount()) {
        emit q->sscResult(sscForActionReplies, QMdmmLogic::QPrivateSignal());
        sscForActionWinners = QMdmmData::stoneScissorsClothWinners(sscForActionReplies);
        if (sscForActionWinners.isEmpty()) {
            // restart due to tie
            startSscForAction();
        } else {
            confirmedActionOrders.clear();
            startActionOrder();
        }
    }
}

void QMdmmLogicPrivate::startActionOrder()
{
    QHash<QString, int> remainingActionCount;
    QList<int> remainingActionOrders;

    int i = 1;
    foreach (const QString &player, sscForActionWinners) {
        remainingActionCount[player] = remainingActionCount.value(player, 0) + 1;
        remainingActionOrders << (i++);
    }
    for (QHash<int, QString>::const_iterator it = confirmedActionOrders.constBegin(); it != confirmedActionOrders.constEnd(); ++it) {
        --remainingActionCount[it.value()];
        remainingActionOrders.removeAll(it.key());
    }

    for (QHash<QString, int>::iterator it = remainingActionCount.begin(); it != remainingActionCount.end();) {
        if (it.value() == 0)
            it = remainingActionCount.erase(it);
        else
            ++it;
    }

    Q_ASSERT(remainingActionCount.count() >= 0);

    if (remainingActionCount.count() == 1) {
        foreach (int n, remainingActionOrders)
            confirmedActionOrders[n] = remainingActionCount.constBegin().key();
        remainingActionCount.clear();
    }

    if (remainingActionCount.isEmpty()) {
        emit q->actionOrderResult(confirmedActionOrders, QMdmmLogic::QPrivateSignal());
        currentActionOrder = 0;
        startAction();
    } else {
        desiredActionOrders.clear();
        state = QMdmmLogic::ActionOrder;
        for (QHash<QString, int>::const_iterator it = remainingActionCount.constBegin(); it != remainingActionCount.constEnd(); ++it)
            emit q->requestActionOrder(it.key(), remainingActionOrders, sscForActionWinners.length(), it.value(), QMdmmLogic::QPrivateSignal());
    }
}

void QMdmmLogicPrivate::actionOrder()
{
    // TODO: support 0 as yielding selection / accepting arbitrary order
    if (desiredActionOrders.size() + confirmedActionOrders.size() == sscForActionWinners.length())
        startSscForActionOrder();
}

void QMdmmLogicPrivate::startSscForActionOrder()
{
    QList<int> orders = desiredActionOrders.uniqueKeys();
    foreach (int order, orders) {
        if (desiredActionOrders.count(order) == 1) {
            confirmedActionOrders[order] = desiredActionOrders.value(order);
            desiredActionOrders.remove(order);
        }
    }

    if (desiredActionOrders.isEmpty()) {
        startActionOrder();
    } else {
        currentStrivingActionOrder = 0;
        for (int i = 1; i <= sscForActionWinners.length(); ++i) {
            if (desiredActionOrders.constFind(i) != desiredActionOrders.constEnd()) {
                currentStrivingActionOrder = i;
                break;
            }
        }

        Q_ASSERT(currentStrivingActionOrder != 0);
        QStringList striving = desiredActionOrders.values(currentStrivingActionOrder);
        sscForActionOrderReplies.clear();
        state = QMdmmLogic::SscForActionOrder;
        emit q->requestSscForActionOrder(striving, currentStrivingActionOrder, QMdmmLogic::QPrivateSignal());
    }
}

void QMdmmLogicPrivate::sscForActionOrder()
{
    if (QStringList striving = desiredActionOrders.values(currentStrivingActionOrder); sscForActionOrderReplies.count() == striving.count()) {
        emit q->sscResult(sscForActionOrderReplies, QMdmmLogic::QPrivateSignal());
        if (QStringList sscForActionOrderWinners = QMdmmData::stoneScissorsClothWinners(sscForActionOrderReplies); !sscForActionOrderWinners.isEmpty()) {
            foreach (const QString &winner, sscForActionOrderWinners)
                striving.removeAll(winner);
            foreach (const QString &loser, striving)
                desiredActionOrders.remove(currentStrivingActionOrder, loser);
        }
        startSscForActionOrder();
    }
}

void QMdmmLogicPrivate::startAction()
{
    if (!room->isRoundOver()) {
        ++currentActionOrder;
        while (++currentActionOrder <= sscForActionWinners.length()) {
            // no copy since C++17 - QHash<T>::value returns const T
            // If C++11 or C++14 is needed, we need const QString && as type of currentPlayer (somewhat uncomfortable)
            QString currentPlayer = confirmedActionOrders.value(currentActionOrder);
            QMdmmPlayer *p = room->player(currentPlayer);
            if (p->alive()) {
                state = QMdmmLogic::Action;
                emit q->requestAction(currentPlayer, currentActionOrder, QMdmmLogic::QPrivateSignal());
                return;
            }
        }

        startSscForAction();
    } else {
        emit q->roundOver(QMdmmLogic::QPrivateSignal());
        startUpgrade();
    }
}

void QMdmmLogicPrivate::startUpgrade()
{
    if (room->isRoundOver()) {
        upgrades.clear();
        foreach (const QMdmmPlayer *p, room->players()) {
            if (p->upgradePoint() > 0) {
                state = QMdmmLogic::Upgrade;
                emit q->requestUpgrade(p->objectName(), p->upgradePoint(), QMdmmLogic::QPrivateSignal());
            }
        }
    } else {
        // ??
    }
}

// NOLINTNEXTLINE(readability-make-member-function-const, readability-function-cognitive-complexity)
void QMdmmLogicPrivate::upgrade()
{
    if (room->isRoundOver()) {
        int n = 0;
        foreach (const QMdmmPlayer *p, room->players()) {
            if (p->upgradePoint() > 0)
                ++n;
        }
        if (upgrades.count() == n) {
            for (QHash<QString, QList<QMdmmData::UpgradeItem>>::const_iterator it = upgrades.constBegin(); it != upgrades.constEnd(); ++it) {
                QMdmmPlayer *up = room->player(it.key());
                const QList<QMdmmData::UpgradeItem> &items = it.value();
                foreach (QMdmmData::UpgradeItem item, items) {
                    bool success = false;
                    switch (item) {
                    case QMdmmData::UpgradeKnife:
                        success = up->upgradeKnife();
                        break;
                    case QMdmmData::UpgradeHorse:
                        success = up->upgradeHorse();
                        break;
                    case QMdmmData::UpgradeMaxHp:
                        success = up->upgradeMaxHp();
                        break;
                    default:
                        break;
                    }
                    Q_ASSERT(success);
                    Q_UNUSED(success);
                }
            }

            state = QMdmmLogic::BeforeRoundStart;

            if (QStringList winners; room->isGameOver(&winners)) {
                room->resetUpgrades();
                emit q->gameOver(winners, QMdmmLogic::QPrivateSignal());
            } else {
                emit q->upgradeResult(upgrades, QMdmmLogic::QPrivateSignal());
            }
        }
    }
}

QMdmmLogic::QMdmmLogic(const QMdmmLogicConfiguration &logicConfiguration, QObject *parent)
    : QObject(parent)
    , d(new QMdmmLogicPrivate(this, logicConfiguration))
{
}

QMdmmLogic::~QMdmmLogic()
{
    delete d;
}

const QMdmmLogicConfiguration &QMdmmLogic::configuration() const
{
    return d->conf;
}

QMdmmLogic::State QMdmmLogic::state() const
{
    return d->state;
}

void QMdmmLogic::addPlayer(const QString &playerName)
{
    if (d->state == BeforeRoundStart) {
        if (!d->players.contains(playerName)) {
            if (d->room->addPlayer(playerName) != nullptr) {
                d->players << playerName;
                d->room->resetUpgrades();
            }
        }
    }
}

void QMdmmLogic::removePlayer(const QString &playerName)
{
    if (d->state == BeforeRoundStart) {
        if (d->players.contains(playerName)) {
            if (d->room->removePlayer(playerName)) {
                d->players.removeAll(playerName);
                d->room->resetUpgrades();
            }
        }
    }
}

void QMdmmLogic::roundStart()
{
    if (d->state == BeforeRoundStart) {
        d->room->prepareForRoundStart();
        d->startSscForAction();
    }
}

void QMdmmLogic::sscReply(const QString &playerName, QMdmmData::StoneScissorsCloth ssc)
{
    if (d->state == SscForAction) {
        d->sscForActionReplies.insert(playerName, ssc);
        d->sscForAction();
    } else if (d->state == SscForActionOrder) {
        d->sscForActionOrderReplies.insert(playerName, ssc);
        d->sscForActionOrder();
    }
}

void QMdmmLogic::actionOrderReply(const QString &playerName, const QList<int> &desiredOrder)
{
    if (d->state == ActionOrder) {
        foreach (int order, desiredOrder)
            d->desiredActionOrders.insert(order, playerName);
        d->actionOrder();
    }
}

void QMdmmLogic::actionReply(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPlace)
{
    if (d->state == Action) {
        if (d->actionFeasible(playerName, action, toPlayer, toPlace)) {
            d->applyAction(playerName, action, toPlayer, toPlace);
            d->startAction();
        }
    }
}

void QMdmmLogic::upgradeReply(const QString &playerName, const QList<QMdmmData::UpgradeItem> &items)
{
    if (d->state == Upgrade) {
        d->upgrades.insert(playerName, items);
        d->upgrade();
    }
}
