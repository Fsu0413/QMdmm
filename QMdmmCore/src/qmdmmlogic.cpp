// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmlogic.h"
#include "qmdmmlogic_p.h"

#include "qmdmmplayer.h"
#include "qmdmmroom.h"

#include <QHash>
#include <QJsonObject>
#include <QJsonValue>
#include <QMultiHash>

QJsonValue QMdmmLogicConfiguration::serialize() const
{
    QJsonObject ob;

#define CONF(member, type) ob.insert(QStringLiteral(#member), static_cast<type>(this->member))

    CONF(playerNumPerRoom, int);
    CONF(initialKnifeDamage, int);
    CONF(maximumKnifeDamage, int);
    CONF(initialHorseDamage, int);
    CONF(maximumHorseDamage, int);
    CONF(initialMaxHp, int);
    CONF(maximumMaxHp, int);
    CONF(punishHpModifier, int);
    CONF(punishHpRoundStrategy, int);
    CONF(zeroHpAsDead, bool);
    CONF(enableLetMove, bool);

#undef CONF

    return ob;
}

bool QMdmmLogicConfiguration::deserialize(const QJsonValue &value) // NOLINT(readability-function-cognitive-complexity)
{
    if (!value.isObject())
        return false;

    QJsonObject ob = value.toObject();
    static const QMdmmLogicConfiguration defaults;

#define CONVERTPUNISHHPROUNDSTRATEGY() static_cast<PunishHpRoundStrategy>(v.toInt())
#define CONF(member, check, convert)                          \
    {                                                         \
        if (ob.contains(QStringLiteral(#member))) {           \
            QJsonValue v = ob.value(QStringLiteral(#member)); \
            if (!v.check())                                   \
                return false;                                 \
            member = convert();                               \
        } else {                                              \
            member = defaults.member;                         \
        }                                                     \
    }

    CONF(playerNumPerRoom, isDouble, v.toInt);
    CONF(initialKnifeDamage, isDouble, v.toInt);
    CONF(maximumKnifeDamage, isDouble, v.toInt);
    CONF(initialHorseDamage, isDouble, v.toInt);
    CONF(maximumHorseDamage, isDouble, v.toInt);
    CONF(initialMaxHp, isDouble, v.toInt);
    CONF(maximumMaxHp, isDouble, v.toInt);
    CONF(punishHpModifier, isDouble, v.toInt);
    CONF(punishHpRoundStrategy, isDouble, CONVERTPUNISHHPROUNDSTRATEGY);
    CONF(zeroHpAsDead, isBool, v.toBool);
    CONF(enableLetMove, isBool, v.toBool);

#undef CONF
#undef CONVERTPUNISHHPROUNDSTRATEGY

    return true;
}

QMdmmLogicPrivate::QMdmmLogicPrivate(QMdmmLogic *q, const QMdmmLogicConfiguration &logicConfiguration)
    : q(q)
    , conf(logicConfiguration)
    , room(new QMdmmRoom(q))
    , state(QMdmmLogic::BeforeGameStart)
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

void QMdmmLogicPrivate::startBeforeGameStart()
{
    room->resetUpgrades();
    state = QMdmmLogic::BeforeGameStart;
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
    QStringList striving = desiredActionOrders.values(currentStrivingActionOrder);
    if (sscForActionOrderReplies.count() == striving.count()) {
        emit q->sscResult(sscForActionOrderReplies, QMdmmLogic::QPrivateSignal());
        QStringList sscForActionOrderWinners = QMdmmData::stoneScissorsClothWinners(sscForActionOrderReplies);
        if (!sscForActionOrderWinners.isEmpty()) {
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
    if (!room->isGameOver()) {
        if (++currentActionOrder <= sscForActionWinners.length()) {
            state = QMdmmLogic::Action;
            emit q->requestAction(confirmedActionOrders[currentActionOrder], currentActionOrder, QMdmmLogic::QPrivateSignal());
        } else {
            startSscForAction();
        }
    } else {
        startUpgrade();
    }
}

void QMdmmLogicPrivate::startUpgrade()
{
    if (room->isGameOver()) {
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
    if (room->isGameOver()) {
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
            state = QMdmmLogic::GameFinish;
            emit q->upgradeResult(upgrades, QMdmmLogic::QPrivateSignal());
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
    if (d->state == BeforeGameStart || d->state == GameFinish) {
        if (!d->players.contains(playerName)) {
            QMdmmPlayer *p = d->room->addPlayer(playerName);
            if (p != nullptr) {
                d->players << playerName;
                if (d->state == GameFinish)
                    d->startBeforeGameStart();
            }
        }
    }
}

void QMdmmLogic::removePlayer(const QString &playerName)
{
    if (d->state == BeforeGameStart || d->state == GameFinish) {
        if (d->players.contains(playerName)) {
            bool r = d->room->removePlayer(playerName);
            if (r) {
                d->players.removeAll(playerName);
                if (d->state == GameFinish)
                    d->startBeforeGameStart();
            }
        }
    }
}

void QMdmmLogic::gameStart()
{
    if (d->state == BeforeGameStart || d->state == GameFinish) {
        d->room->prepareForGameStart();
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

void QMdmmLogic::actionReply(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPosition)
{
    if (d->state == Action) {
        if (d->actionFeasible(playerName, action, toPlayer, toPosition)) {
            d->applyAction(playerName, action, toPlayer, toPosition);
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
