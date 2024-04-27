// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmplayer.h"

struct QMdmmPlayerPrivate
{
    QMdmmPlayerPrivate()
        : knife(false)
        , horse(false)
        , hp(10)
        , place(QMdmmData::Country)
        , knifeDamage(1)
        , horseDamage(2)
        , maxHp(10)
        , upgradePoint(0)
    {
    }

    bool knife;
    bool horse;
    int hp;
    QMdmmData::Place place;

    int knifeDamage;
    int horseDamage;
    int maxHp;

    int upgradePoint;
};

QMdmmPlayer::QMdmmPlayer(const QString &name, QObject *parent)
    : QObject(parent)
    , d(new QMdmmPlayerPrivate)
{
    setObjectName(name);
}

QMdmmPlayer::~QMdmmPlayer()
{
    delete d;
}

bool QMdmmPlayer::hasKnife() const
{
    return d->knife;
}

bool QMdmmPlayer::hasHorse() const
{
    return d->horse;
}

int QMdmmPlayer::hp() const
{
    return d->hp;
}

QMdmmData::Place QMdmmPlayer::place() const
{
    return d->place;
}

int QMdmmPlayer::knifeDamage() const
{
    return d->knifeDamage;
}

int QMdmmPlayer::horseDamage() const
{
    return d->horseDamage;
}

int QMdmmPlayer::maxHp() const
{
    return d->maxHp;
}

bool QMdmmPlayer::dead() const
{
    // TODO: setting - 0 HP alive / dead
    return d->hp <= 0;
}

bool QMdmmPlayer::alive() const
{
    return !dead();
}

int QMdmmPlayer::upgradePoint() const
{
    return d->upgradePoint;
}

bool QMdmmPlayer::canBuyKnife() const
{
    return !d->knife;
}

bool QMdmmPlayer::canBuyHorse() const
{
    return !d->horse;
}

bool QMdmmPlayer::canSlash(const QMdmmPlayer *to) const
{
    if (!d->knife)
        return false;

    if (to->dead())
        return false;

    if (d->place != to->place())
        return false;

    return true;
}

bool QMdmmPlayer::canKick(const QMdmmPlayer *to) const
{
    if (!d->horse)
        return false;

    if (to->dead())
        return false;

    if ((d->place != to->place()) || (d->place == QMdmmData::Country))
        return false;

    return true;
}

bool QMdmmPlayer::canMove(QMdmmData::Place toPlace) const
{
    return QMdmmData::isPlaceAdjecent(d->place, toPlace);
}

bool QMdmmPlayer::canLetMove(const QMdmmPlayer *to, QMdmmData::Place toPlace) const
{
    if (to->dead())
        return false;

    return QMdmmData::isPlaceAdjecent(to->place(), toPlace) && ((QMdmmData::isPlaceAdjecent(d->place, to->place()) && (toPlace == d->place)) || (d->place == to->place()));
}

bool QMdmmPlayer::buyKnife()
{
    if (!canBuyKnife())
        return false;

    d->knife = true;
    emit hasKnifeChanged(d->knife, QPrivateSignal());
    return true;
}

bool QMdmmPlayer::buyHorse()
{
    if (!canBuyHorse())
        return false;

    d->horse = true;
    emit hasHorseChanged(d->horse, QPrivateSignal());
    return true;
}

bool QMdmmPlayer::slash(QMdmmPlayer *to)
{
    if (!canSlash(to))
        return false;

    to->damage(this, d->knifeDamage, QMdmmData::Slash);

    // life punishment: damage point is maxHp / 2
    // TODO: configure item for amount / switch of punish HP
    if (place() != QMdmmData::Country)
        damage(to, maxHp() / 2, QMdmmData::PunishHp);

    return true;
}

bool QMdmmPlayer::kick(QMdmmPlayer *to)
{
    if (!canKick(to))
        return false;

    to->damage(this, d->horseDamage, QMdmmData::Kick);

    if (!to->dead())
        letMove(to, QMdmmData::Country);

    return true;
}

bool QMdmmPlayer::move(QMdmmData::Place toPlace)
{
    if (canMove(toPlace)) {
        placeChange(toPlace);
        return true;
    }

    return false;
}

bool QMdmmPlayer::letMove(QMdmmPlayer *to, QMdmmData::Place toPlace) // NOLINT
{
    // pull, push, kick(effect)

    if (canLetMove(to, toPlace)) {
        to->placeChange(toPlace);
        return true;
    }

    return false;
}

bool QMdmmPlayer::doNothing(const QString & /*unused*/)
{
    return true;
}

void QMdmmPlayer::damage(QMdmmPlayer *from, int damagePoint, QMdmmData::DamageReason reason)
{
    Q_UNUSED(reason);
    bool wasDead = dead();

    d->hp -= damagePoint;
    emit hpChanged(d->hp, QPrivateSignal());

    if (!wasDead && dead()) {
        ++(from->d->upgradePoint);
        emit aliveChanged(alive(), QPrivateSignal());
        emit deadChanged(dead(), QPrivateSignal());
        emit from->upgradePointChanged(from->d->upgradePoint, QPrivateSignal());
    }
}

void QMdmmPlayer::placeChange(QMdmmData::Place toPlace)
{
    d->place = toPlace;
    emit placeChanged(d->place, QPrivateSignal());
}

// TODO: configure item for maximum value of upgrade
bool QMdmmPlayer::upgradeKnife()
{
    if (d->knifeDamage >= 5)
        return false;

    ++d->knifeDamage;
    emit knifeDamageChanged(d->knifeDamage, QPrivateSignal());
    return true;
}

bool QMdmmPlayer::upgradeHorse()
{
    if (d->horseDamage >= 10)
        return false;

    ++d->horseDamage;
    emit horseDamageChanged(d->horseDamage, QPrivateSignal());
    return true;
}

bool QMdmmPlayer::upgradeMaxHp()
{
    if (d->maxHp >= 20)
        return false;

    ++d->maxHp;
    emit maxHpChanged(d->maxHp, QPrivateSignal());
    return true;
}

void QMdmmPlayer::prepareForGameStart(int playerNum)
{
    d->hp = d->maxHp;
    emit hpChanged(d->hp, QPrivateSignal());
    d->place = static_cast<QMdmmData::Place>(playerNum);
    emit placeChanged(d->place, QPrivateSignal());
    d->knife = false;
    emit hasKnifeChanged(d->knife, QPrivateSignal());
    d->horse = false;
    emit hasHorseChanged(d->horse, QPrivateSignal());
    d->upgradePoint = 0;
    emit upgradePointChanged(d->upgradePoint, QPrivateSignal());
}
