// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmplayer.h"
#include "qmdmmroom.h"

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

QMdmmPlayer::QMdmmPlayer(const QString &name, QMdmmRoom *room)
    : QObject(room)
    , d(new QMdmmPlayerPrivate)
{
    setObjectName(name);
}

QMdmmPlayer::~QMdmmPlayer()
{
    delete d;
}

QMdmmRoom *QMdmmPlayer::room()
{
    // We don't need extra cost for qobject_cast here, since every Player is created with Room as parent.
    return static_cast<QMdmmRoom *>(parent());
}

const QMdmmRoom *QMdmmPlayer::room() const
{
    // same as above
    return static_cast<const QMdmmRoom *>(parent());
}

bool QMdmmPlayer::hasKnife() const
{
    return d->knife;
}

void QMdmmPlayer::setHasKnife(bool k)
{
    if (d->knife != k) {
        d->knife = k;
        emit hasKnifeChanged(k, QPrivateSignal());
    }
}

bool QMdmmPlayer::hasHorse() const
{
    return d->horse;
}

void QMdmmPlayer::setHasHorse(bool h)
{
    if (d->horse != h) {
        d->horse = h;
        emit hasHorseChanged(h, QPrivateSignal());
    }
}

int QMdmmPlayer::hp() const
{
    return d->hp;
}

void QMdmmPlayer::setHp(int h, bool *kills)
{
    // This is not only a setter, but also the one triggers "die" signal.

    if (kills == nullptr) {
        static bool _kills;
        kills = &_kills;
    }

    *kills = false;

    if (d->hp != h) {
        bool wasDead = dead();
        d->hp = h;
        emit hpChanged(h, QPrivateSignal());
        if (!wasDead && dead()) {
            *kills = true;
            emit die(QPrivateSignal());
        }
    }
}

QMdmmData::Place QMdmmPlayer::place() const
{
    return d->place;
}

void QMdmmPlayer::setPlace(QMdmmData::Place toPlace)
{
    if (d->place != toPlace) {
        d->place = toPlace;
        emit placeChanged(d->place, QPrivateSignal());
    }
}

int QMdmmPlayer::knifeDamage() const
{
    return d->knifeDamage;
}

void QMdmmPlayer::setKnifeDamage(int k)
{
    if (d->knifeDamage != k) {
        d->knifeDamage = k;
        emit knifeDamageChanged(k, QPrivateSignal());
    }
}

int QMdmmPlayer::horseDamage() const
{
    return d->horseDamage;
}

void QMdmmPlayer::setHorseDamage(int h)
{
    if (d->horseDamage != h) {
        d->horseDamage = h;
        emit horseDamageChanged(h, QPrivateSignal());
    }
}

int QMdmmPlayer::maxHp() const
{
    return d->maxHp;
}

void QMdmmPlayer::setMaxHp(int m)
{
    if (d->maxHp != m) {
        d->maxHp = m;
        emit maxHpChanged(m, QPrivateSignal());
    }
}

int QMdmmPlayer::upgradePoint() const
{
    return d->upgradePoint;
}

void QMdmmPlayer::setUpgradePoint(int u)
{
    if (d->upgradePoint != u) {
        d->upgradePoint = u;
        emit upgradePointChanged(u, QPrivateSignal());
    }
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

bool QMdmmPlayer::canBuyKnife() const
{
    return alive() && !hasKnife();
}

bool QMdmmPlayer::canBuyHorse() const
{
    return alive() && !hasHorse();
}

bool QMdmmPlayer::canSlash(const QMdmmPlayer *to) const
{
    if (dead() || to->dead())
        return false;

    if (!hasKnife())
        return false;

    if (place() != to->place())
        return false;

    return true;
}

bool QMdmmPlayer::canKick(const QMdmmPlayer *to) const
{
    if (dead() || to->dead())
        return false;

    if (!hasHorse())
        return false;

    if ((place() != to->place()) || (place() == QMdmmData::Country))
        return false;

    return true;
}

bool QMdmmPlayer::canMove(QMdmmData::Place toPlace) const
{
    return alive() && QMdmmData::isPlaceAdjecent(place(), toPlace);
}

bool QMdmmPlayer::canLetMove(const QMdmmPlayer *to, QMdmmData::Place toPlace) const
{
    if (dead() || to->dead())
        return false;

    // one movement should move player to adjecent place only
    if (!QMdmmData::isPlaceAdjecent(to->place(), toPlace))
        return false;

    // case 1: pull a player in adjecent place to self's place
    if (QMdmmData::isPlaceAdjecent(place(), to->place()) && toPlace == place())
        return true;

    // case 2: push a player in same place to adjecent place
    if (place() == to->place())
        return true;

    return false;
}

bool QMdmmPlayer::buyKnife()
{
    if (!canBuyKnife())
        return false;

    setHasKnife(true);
    return true;
}

bool QMdmmPlayer::buyHorse()
{
    if (!canBuyHorse())
        return false;

    setHasHorse(true);
    return true;
}

bool QMdmmPlayer::slash(QMdmmPlayer *to)
{
    if (!canSlash(to))
        return false;

    to->damage(this, knifeDamage(), QMdmmData::Slash);

    // life punishment: damage point is maxHp / 2, with reminders ignored
    // TODO: configure item for amount / switch of punish HP
    if (place() != QMdmmData::Country)
        damage(to, maxHp() / 2, QMdmmData::PunishHp);

    return true;
}

bool QMdmmPlayer::kick(QMdmmPlayer *to)
{
    if (!canKick(to))
        return false;

    to->damage(this, horseDamage(), QMdmmData::Kick);

    if (!to->dead())
        letMove(to, QMdmmData::Country);

    return true;
}

bool QMdmmPlayer::move(QMdmmData::Place toPlace)
{
    if (canMove(toPlace)) {
        setPlace(toPlace);
        return true;
    }

    return false;
}

bool QMdmmPlayer::letMove(QMdmmPlayer *to, QMdmmData::Place toPlace) // NOLINT(readability-make-member-function-const)
{
    // pull, push, kick(effect)

    if (canLetMove(to, toPlace)) {
        to->setPlace(toPlace);
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
    bool kills = false;

    setHp(hp() - damagePoint, &kills);
    if (kills)
        from->setUpgradePoint(from->upgradePoint() + 1);
}

// TODO: configure item for maximum value of upgrade
bool QMdmmPlayer::upgradeKnife()
{
    if (knifeDamage() >= 5)
        return false;

    setKnifeDamage(knifeDamage() + 1);
    return true;
}

bool QMdmmPlayer::upgradeHorse()
{
    if (horseDamage() >= 10)
        return false;

    setHorseDamage(horseDamage() + 1);
    return true;
}

bool QMdmmPlayer::upgradeMaxHp()
{
    if (maxHp() >= 20)
        return false;

    setMaxHp(maxHp() + 1);
    return true;
}

void QMdmmPlayer::prepareForGameStart(int playerNum)
{
    setHasKnife(false);
    setHasHorse(false);
    setHp(maxHp());
    setPlace(static_cast<QMdmmData::Place>(playerNum));
    setUpgradePoint(0);
}
