// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmplayer.h"
#include "qmdmmplayer_p.h"

#include "qmdmmlogic.h"
#include "qmdmmroom.h"

QMdmmPlayerPrivate::QMdmmPlayerPrivate(QMdmmRoom *room)
    : knife(false)
    , horse(false)
    , hp(room->logic()->configuration().initialMaxHp())
    , place(QMdmmData::Country)
    , knifeDamage(room->logic()->configuration().initialKnifeDamage())
    , horseDamage(room->logic()->configuration().initialHorseDamage())
    , maxHp(room->logic()->configuration().initialMaxHp())
    , upgradePoint(0)
{
}

QMdmmPlayer::QMdmmPlayer(const QString &name, QMdmmRoom *room)
    : QObject(room)
    , d(new QMdmmPlayerPrivate(room))
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

int QMdmmPlayer::place() const
{
    return d->place;
}

void QMdmmPlayer::setPlace(int toPlace)
{
    if (d->place != toPlace) {
        d->place = toPlace;
        emit placeChanged(d->place, QPrivateSignal());
    }
}

int QMdmmPlayer::initialPlace() const
{
    return d->initialPlace;
}

void QMdmmPlayer::setInitialPlace(int initialPlace)
{
    if (d->initialPlace != initialPlace) {
        d->initialPlace = initialPlace;
        emit initialPlaceChanged(initialPlace, QPrivateSignal {});
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
    bool zeroHpAsDead = room()->logic()->configuration().zeroHpAsDead();
    return zeroHpAsDead ? (d->hp <= 0) : (d->hp < 0);
}

bool QMdmmPlayer::canBuyKnife() const
{
    return alive() && !hasKnife() && (room()->logic()->configuration().canBuyOnlyInInitialCity() ? (place() == initialPlace()) : (place() != QMdmmData::Country));
}

bool QMdmmPlayer::canBuyHorse() const
{
    return alive() && !hasHorse() && (room()->logic()->configuration().canBuyOnlyInInitialCity() ? (place() == initialPlace()) : (place() != QMdmmData::Country));
}

bool QMdmmPlayer::canSlash(const QMdmmPlayer *to) const
{
    Q_ASSERT(room() == to->room());

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
    Q_ASSERT(room() == to->room());

    if (dead() || to->dead())
        return false;

    if (!hasHorse())
        return false;

    if ((place() != to->place()) || (place() == QMdmmData::Country))
        return false;

    return true;
}

bool QMdmmPlayer::canMove(int toPlace) const
{
    return alive() && QMdmmData::isPlaceAdjecent(place(), toPlace);
}

bool QMdmmPlayer::canLetMove(const QMdmmPlayer *to, int toPlace) const
{
    Q_ASSERT(room() == to->room());

    if (!room()->logic()->configuration().enableLetMove())
        return false;

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

int QMdmmPlayer::upgradeKnifeRemainingTimes() const
{
    return room()->logic()->configuration().maximumKnifeDamage() - knifeDamage();
}

int QMdmmPlayer::upgradeHorseRemainingTimes() const
{
    return room()->logic()->configuration().maximumHorseDamage() - horseDamage();
}

int QMdmmPlayer::upgradeMaxHpRemainingTimes() const
{
    return room()->logic()->configuration().maximumMaxHp() - maxHp();
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
    Q_ASSERT(room() == to->room());

    if (!canSlash(to))
        return false;

    to->applyDamage(this, knifeDamage(), QMdmmData::Slashed);

    if (place() != QMdmmData::Country) {
        int punishHpModifier = room()->logic()->configuration().punishHpModifier();
        if (punishHpModifier > 0) {
            QMdmmLogicConfiguration::PunishHpRoundStrategy punishHpRoundStrategy = room()->logic()->configuration().punishHpRoundStrategy();

            int punishedHp;
            switch (punishHpRoundStrategy) {
            default:
                [[fallthrough]];
            case QMdmmLogicConfiguration::RoundDown:
                punishedHp = maxHp() / punishHpModifier;
                break;
            case QMdmmLogicConfiguration::RoundToNearest45:
                punishedHp = ((maxHp() * 2) / punishHpModifier + 1) / 2;
                break;
            case QMdmmLogicConfiguration::RoundUp:
                punishedHp = (maxHp() + punishHpModifier - 1) / punishHpModifier;
                break;
            case QMdmmLogicConfiguration::PlusOne:
                punishedHp = maxHp() / punishHpModifier + 1;
                break;
            }

            if (punishedHp > 0)
                applyDamage(to, punishedHp, QMdmmData::HpPunished);
        }
    }

    return true;
}

bool QMdmmPlayer::kick(QMdmmPlayer *to)
{
    Q_ASSERT(room() == to->room());

    if (!canKick(to))
        return false;

    to->applyDamage(this, horseDamage(), QMdmmData::Kicked);

    if (!to->dead())
        to->setPlace(QMdmmData::Country);

    return true;
}

bool QMdmmPlayer::move(int toPlace)
{
    if (canMove(toPlace)) {
        setPlace(toPlace);
        return true;
    }

    return false;
}

bool QMdmmPlayer::letMove(QMdmmPlayer *to, int toPlace) // NOLINT(readability-make-member-function-const): Operation is ought to be not const
{
    // for pull / push
    // kick (effect) is not implemented here since it should bypass canLetMove check.

    Q_ASSERT(room() == to->room());

    if (canLetMove(to, toPlace)) {
        to->setPlace(toPlace);
        return true;
    }

    return false;
}

bool QMdmmPlayer::doNothing() // NOLINT(readability-make-member-function-const): Operation is ought to be not const
{
    return true;
}

void QMdmmPlayer::applyDamage(QMdmmPlayer *from, int damagePoint, QMdmmData::DamageReason reason)
{
    Q_ASSERT(room() == from->room());

    bool kills = false;

    setHp(hp() - damagePoint, &kills);
    emit damaged(from, damagePoint, reason, QPrivateSignal());
    emit damaged(from->objectName(), damagePoint, reason, QPrivateSignal());

    if (kills)
        from->setUpgradePoint(from->upgradePoint() + 1);
}

bool QMdmmPlayer::upgradeKnife()
{
    if (!canUpdateKnife())
        return false;

    setKnifeDamage(knifeDamage() + 1);
    return true;
}

bool QMdmmPlayer::upgradeHorse()
{
    if (!canUpdateHorse())
        return false;

    setHorseDamage(horseDamage() + 1);
    return true;
}

bool QMdmmPlayer::upgradeMaxHp()
{
    if (!canUpdateMaxHp())
        return false;

    setMaxHp(maxHp() + 1);
    return true;
}

void QMdmmPlayer::prepareForRoundStart(int playerNum)
{
    setHasKnife(false);
    setHasHorse(false);
    setHp(maxHp());
    setInitialPlace(playerNum);
    setPlace(playerNum);
    setUpgradePoint(0);
}

void QMdmmPlayer::resetUpgrades()
{
    setMaxHp(room()->logic()->configuration().initialMaxHp());
    setKnifeDamage(room()->logic()->configuration().initialKnifeDamage());
    setHorseDamage(room()->logic()->configuration().initialHorseDamage());
    setUpgradePoint(0);
}
