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

#undef CONF

    return ob;
}

bool QMdmmLogicConfiguration::deserialize(const QJsonValue &value) // NOLINT(readability-function-cognitive-complexity)
{
    if (!value.isObject())
        return false;

    QJsonObject ob = value.toObject();
    static const QMdmmLogicConfiguration defaults;

#define CONVERTPUNISHHPROUNDSTRATEGY(defaultValue) static_cast<PunishHpRoundStrategy>(v.toInt(defaultValue))
#define CONF(member, check, convert)                      \
    do {                                                  \
        QJsonValue v = ob.value(QStringLiteral(#member)); \
        if (!v.check())                                   \
            return false;                                 \
        member = convert(defaults.member);                \
    } while (false)

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

#undef CONF
#undef CONVERTPUNISHHPROUNDSTRATEGY

    return true;
}

QMdmmLogicPrivate::QMdmmLogicPrivate(QMdmmLogic *q, const QMdmmLogicConfiguration &logicConfiguration)
    : q(q)
    , conf(logicConfiguration)
    , room(new QMdmmRoom(q))
    , state(QMdmmLogic::BeforeGameStart)
    , currentStrivingOperationOrder(0)
    , currentOperationOrder(0)
{
}

bool QMdmmLogicPrivate::operationFeasible(const QString &fromPlayer, QMdmmData::Operation operation, const QString &toPlayer, int toPlace) const
{
    const QMdmmPlayer *from = room->player(fromPlayer);
    switch (operation) {
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

// NOLINTNEXTLINE(readability-make-member-function-const): Operation is ought not to be const
bool QMdmmLogicPrivate::applyOperation(const QString &fromPlayer, QMdmmData::Operation operation, const QString &toPlayer, int toPlace)
{
    emit q->operationResult(fromPlayer, operation, toPlayer, toPlace, QMdmmLogic::QPrivateSignal());

    QMdmmPlayer *from = room->player(fromPlayer);
    switch (operation) {
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

void QMdmmLogicPrivate::startSscForOperation()
{
    sscForOperationReplies.clear();
    sscForOperationWinners.clear();
    state = QMdmmLogic::SscForOperation;
    emit q->requestSscForOperation(room->alivePlayerNames(), QMdmmLogic::QPrivateSignal());
}

void QMdmmLogicPrivate::sscForOperation()
{
    if (sscForOperationReplies.count() == room->alivePlayersCount()) {
        emit q->sscResult(sscForOperationReplies, QMdmmLogic::QPrivateSignal());
        sscForOperationWinners = QMdmmData::stoneScissorsClothWinners(sscForOperationReplies);
        if (sscForOperationWinners.isEmpty()) {
            // restart due to tie
            startSscForOperation();
        } else {
            confirmedOperationOrders.clear();
            startOperationOrder();
        }
    }
}

void QMdmmLogicPrivate::startOperationOrder()
{
    QHash<QString, int> remainingOperationCount;
    QList<int> remainingOperationOrders;

    int i = 1;
    foreach (const QString &player, sscForOperationWinners) {
        remainingOperationCount[player] = remainingOperationCount.value(player, 0) + 1;
        remainingOperationOrders << (i++);
    }
    for (QHash<int, QString>::const_iterator it = confirmedOperationOrders.constBegin(); it != confirmedOperationOrders.constEnd(); ++it) {
        --remainingOperationCount[it.value()];
        remainingOperationOrders.removeAll(it.key());
    }

    for (QHash<QString, int>::iterator it = remainingOperationCount.begin(); it != remainingOperationCount.end();) {
        if (it.value() == 0)
            it = remainingOperationCount.erase(it);
        else
            ++it;
    }

    Q_ASSERT(remainingOperationCount.count() >= 0);

    if (remainingOperationCount.count() == 1) {
        foreach (int n, remainingOperationOrders)
            confirmedOperationOrders[n] = remainingOperationCount.constBegin().key();
        remainingOperationCount.clear();
    }

    if (remainingOperationCount.isEmpty()) {
        emit q->operationOrderResult(confirmedOperationOrders, QMdmmLogic::QPrivateSignal());
        currentOperationOrder = 0;
        startOperation();
    } else {
        desiredOperationOrders.clear();
        state = QMdmmLogic::OperationOrder;
        for (QHash<QString, int>::const_iterator it = remainingOperationCount.constBegin(); it != remainingOperationCount.constEnd(); ++it)
            emit q->requestOperationOrder(it.key(), remainingOperationOrders, sscForOperationWinners.length(), it.value(), QMdmmLogic::QPrivateSignal());
    }
}

void QMdmmLogicPrivate::operationOrder()
{
    // TODO: support 0 as yielding selection / accepting arbitrary order
    if (desiredOperationOrders.size() + confirmedOperationOrders.size() == sscForOperationWinners.length())
        startSscForOperationOrder();
}

void QMdmmLogicPrivate::startSscForOperationOrder()
{
    QList<int> orders = desiredOperationOrders.uniqueKeys();
    foreach (int order, orders) {
        if (desiredOperationOrders.count(order) == 1) {
            confirmedOperationOrders[order] = desiredOperationOrders.value(order);
            desiredOperationOrders.remove(order);
        }
    }

    if (desiredOperationOrders.isEmpty()) {
        startOperationOrder();
    } else {
        currentStrivingOperationOrder = 0;
        for (int i = 1; i <= sscForOperationWinners.length(); ++i) {
            if (desiredOperationOrders.constFind(i) != desiredOperationOrders.constEnd()) {
                currentStrivingOperationOrder = i;
                break;
            }
        }

        Q_ASSERT(currentStrivingOperationOrder != 0);
        QStringList striving = desiredOperationOrders.values(currentStrivingOperationOrder);
        sscForOperationOrderReplies.clear();
        state = QMdmmLogic::SscForOperationOrder;
        emit q->requestSscForOperationOrder(striving, currentStrivingOperationOrder, QMdmmLogic::QPrivateSignal());
    }
}

void QMdmmLogicPrivate::sscForOperationOrder()
{
    QStringList striving = desiredOperationOrders.values(currentStrivingOperationOrder);
    if (sscForOperationOrderReplies.count() == striving.count()) {
        emit q->sscResult(sscForOperationOrderReplies, QMdmmLogic::QPrivateSignal());
        QStringList sscForOperationOrderWinners = QMdmmData::stoneScissorsClothWinners(sscForOperationOrderReplies);
        if (!sscForOperationOrderWinners.isEmpty()) {
            foreach (const QString &winner, sscForOperationOrderWinners)
                striving.removeAll(winner);
            foreach (const QString &loser, striving)
                desiredOperationOrders.remove(currentStrivingOperationOrder, loser);
        }
        startSscForOperationOrder();
    }
}

void QMdmmLogicPrivate::startOperation()
{
    if (!room->isGameOver()) {
        if (++currentOperationOrder <= sscForOperationWinners.length()) {
            state = QMdmmLogic::Operation;
            emit q->requestOperation(confirmedOperationOrders[currentOperationOrder], currentOperationOrder, QMdmmLogic::QPrivateSignal());
        } else {
            startSscForOperation();
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
        d->startSscForOperation();
    }
}

void QMdmmLogic::sscReply(const QString &playerName, QMdmmData::StoneScissorsCloth ssc)
{
    if (d->state == SscForOperation) {
        d->sscForOperationReplies.insert(playerName, ssc);
        d->sscForOperation();
    } else if (d->state == SscForOperationOrder) {
        d->sscForOperationOrderReplies.insert(playerName, ssc);
        d->sscForOperationOrder();
    }
}

void QMdmmLogic::operationOrderReply(const QString &playerName, const QList<int> &desiredOrder)
{
    if (d->state == OperationOrder) {
        foreach (int order, desiredOrder)
            d->desiredOperationOrders.insert(order, playerName);
        d->operationOrder();
    }
}

void QMdmmLogic::operationReply(const QString &playerName, QMdmmData::Operation operation, const QString &toPlayer, int toPosition)
{
    if (d->state == Operation) {
        if (d->operationFeasible(playerName, operation, toPlayer, toPosition)) {
            d->applyOperation(playerName, operation, toPlayer, toPosition);
            d->startOperation();
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
