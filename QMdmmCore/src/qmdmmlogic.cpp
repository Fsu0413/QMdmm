// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmlogic.h"
#include "qmdmmlogic_p.h"

#include "qmdmmplayer.h"
#include "qmdmmroom.h"

#include <QHash>
#include <QJsonObject>
#include <QJsonValue>
#include <QMultiHash>
#include <QPointer>

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
{
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
    // TODO
    Q_UNIMPLEMENTED();
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

void QMdmmLogic::operationReply(const QString &playerName, const QString &operation, const QString &toPlayer, int toPosition)
{
}

void QMdmmLogic::updateReply(const QString &playerName, const QList<int> &items)
{
}
