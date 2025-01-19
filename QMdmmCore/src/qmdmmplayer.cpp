// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmplayer.h"
#include "qmdmmplayer_p.h"

#include "qmdmmlogic.h"
#include "qmdmmroom.h"

#ifndef DOXYGEN

QMdmmPlayerPrivate::QMdmmPlayerPrivate(QMdmmRoom *room)
    : knife(false)
    , horse(false)
    , hp(room->logicConfiguration().initialMaxHp())
    , place(QMdmmData::Country)
    , initialPlace(QMdmmData::Country)
    , knifeDamage(room->logicConfiguration().initialKnifeDamage())
    , horseDamage(room->logicConfiguration().initialHorseDamage())
    , maxHp(room->logicConfiguration().initialMaxHp())
    , upgradePoint(0)
{
}

void QMdmmPlayerPrivate::applyDamage(QMdmmPlayer *from, QMdmmPlayer *to, int damagePoint, QMdmmData::DamageReason reason)
{
    Q_ASSERT(from->room() == to->room());

    bool kills = false;

    to->setHp(to->hp() - damagePoint, &kills);
    emit to->damaged(from, damagePoint, reason, QMdmmPlayer::QPrivateSignal());
    emit to->damaged(from->objectName(), damagePoint, reason, QMdmmPlayer::QPrivateSignal());

    if (kills)
        from->setUpgradePoint(from->upgradePoint() + 1);
}

#endif

/**
 * @file qmdmmplayer.h
 * @brief This is the file where MDMM player is defined.
 */

/**
 * @class QMdmmPlayer
 * @brief The player playing MDMM Game
 *
 * This is the place where the data of each player is saved, and is the object where the data changed signal is emitted.
 */

/**
 * @property QMdmmPlayer::hasKnife
 * @brief if the player has bought knife
 */

/**
 * @property QMdmmPlayer::hasHorse
 * @brief if the player has bought horse
 */

/**
 * @property QMdmmPlayer::hp
 * @brief the current HP of the player
 */

/**
 * @property QMdmmPlayer::place
 * @brief the place of the player.
 *
 * @note The return value is not of type @c QMdmmData::Place . The reason is that the number of Cities should be same as the number of players in a room.
 * Making a enumeration value of each City is not feasible. Since even the enumeration is created the value is still @c static_cast-ed and the enumeration will be of no use.
 */

/**
 * @property QMdmmPlayer::initialPlace
 * @brief the initial place of the player.
 *
 * @note This is also known as "seat", since one should be in "City No. N" where N is the seat number when game starts.
 */

/**
 * @property QMdmmPlayer::knifeDamage
 * @brief the knife damage of the player.
 */

/**
 * @property QMdmmPlayer::horseDamage
 * @brief the horse damage of the player.
 */

/**
 * @property QMdmmPlayer::maxHp
 * @brief the maximum HP of the player.
 */

/**
 * @property QMdmmPlayer::upgradePoint
 * @brief the upgrade point of the player.
 *
 * @note This is also known as "killed player per round"
 */

/**
 * @property QMdmmPlayer::dead
 * @brief if the player is dead
 *
 * @note This property is not stored and is calculated from @c hp property.
 */

/**
 * @property QMdmmPlayer::alive
 * @brief if the player is alive
 *
 * @note This property is not stored and is calculated from @c hp property.
 */

/**
 * @brief ctor.
 * @param name The internal name of the player
 * @param room The room the player is in (or parent as QObject hierarchy)
 *
 * Note: This ctor is meant to be called from Room
 */
QMdmmPlayer::QMdmmPlayer(const QString &name, QMdmmRoom *room)
    : QObject(room)
    , d(std::make_unique<QMdmmPlayerPrivate>(room))
{
    setObjectName(name);
}

/**
 * @brief dtor
 *
 * Note: This dtor is meant to be called from Room
 */
QMdmmPlayer::~QMdmmPlayer() = default;

/**
 * @brief Get the room the player is in
 * @return the room the player is in
 */
QMdmmRoom *QMdmmPlayer::room()
{
    // We don't need extra cost for qobject_cast here, since every Player is created with Room as parent.
    return static_cast<QMdmmRoom *>(parent());
}

/**
 * @brief Get the room the player is in (const version)
 * @return the room the player is in
 */
const QMdmmRoom *QMdmmPlayer::room() const
{
    // same as above
    return static_cast<const QMdmmRoom *>(parent());
}

/**
 * @brief getter of property @c hasKnife
 * @return @c hasKnife
 */
bool QMdmmPlayer::hasKnife() const noexcept
{
    return d->knife;
}

/**
 * @brief setter of property @c hasKnife
 * @param k @c hasKnife
 */
void QMdmmPlayer::setHasKnife(bool k)
{
    if (d->knife != k) {
        d->knife = k;
        emit hasKnifeChanged(k, QPrivateSignal());
    }
}

/**
 * @brief getter of property @c hasHorse
 * @return @c hasHorse
 */
bool QMdmmPlayer::hasHorse() const noexcept
{
    return d->horse;
}

/**
 * @brief setter of property @c hasHorse
 * @param h @c hasHorse
 */
void QMdmmPlayer::setHasHorse(bool h)
{
    if (d->horse != h) {
        d->horse = h;
        emit hasHorseChanged(h, QPrivateSignal());
    }
}

/**
 * @brief getter of property @c hp
 * @return @c hp
 */
int QMdmmPlayer::hp() const noexcept
{
    return d->hp;
}

/**
 * @brief setter of property @c hp
 * @param h @c hp
 * @param kills (OUT) if this HP kills the player
 *
 * This is not only a setter, but also the one triggers @c die() signal if this HP kills.
 * A value of HP kills means before setting this HP the player is alive, and after setting this HP the player is dead.
 *
 * A player is alive if @c hp is positive and dead if @c hp is negative. It depends on the configuration whether @c hp is 0 is treated as dead or alive.
 *
 * @sa QMdmmLogicConfiguration::zeroHpAsDead
 */
void QMdmmPlayer::setHp(int h, bool *kills)
{
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

/**
 * @brief getter of property @c place
 * @return @c place
 */
int QMdmmPlayer::place() const noexcept
{
    return d->place;
}

/**
 * @brief setter of property @c place
 * @param toPlace @c place
 */
void QMdmmPlayer::setPlace(int toPlace)
{
    if (d->place != toPlace) {
        d->place = toPlace;
        emit placeChanged(d->place, QPrivateSignal());
    }
}

/**
 * @brief getter of property @c initialPlace
 * @return @c initialPlace
 */
int QMdmmPlayer::initialPlace() const noexcept
{
    return d->initialPlace;
}

/**
 * @brief setter of property @c initialPlace
 * @param initialPlace @c initialPlace
 */
void QMdmmPlayer::setInitialPlace(int initialPlace)
{
    if (d->initialPlace != initialPlace) {
        d->initialPlace = initialPlace;
        emit initialPlaceChanged(initialPlace, QPrivateSignal {});
    }
}

/**
 * @brief getter of property @c knifeDamage
 * @return @c knifeDamage
 */
int QMdmmPlayer::knifeDamage() const noexcept
{
    return d->knifeDamage;
}

/**
 * @brief setter of property @c knifeDamage
 * @param k @c knifeDamage
 */
void QMdmmPlayer::setKnifeDamage(int k)
{
    if (d->knifeDamage != k) {
        d->knifeDamage = k;
        emit knifeDamageChanged(k, QPrivateSignal());
    }
}

/**
 * @brief getter of property @c horseDamage
 * @return @c horseDamage
 */
int QMdmmPlayer::horseDamage() const noexcept
{
    return d->horseDamage;
}

/**
 * @brief setter of property @c horseDamage
 * @param h @c horseDamage
 */
void QMdmmPlayer::setHorseDamage(int h)
{
    if (d->horseDamage != h) {
        d->horseDamage = h;
        emit horseDamageChanged(h, QPrivateSignal());
    }
}

/**
 * @brief getter of property @c maxHp
 * @return @c maxHp
 */
int QMdmmPlayer::maxHp() const noexcept
{
    return d->maxHp;
}

/**
 * @brief setter of property @c maxHp
 * @param m @c maxHp
 */
void QMdmmPlayer::setMaxHp(int m)
{
    if (d->maxHp != m) {
        d->maxHp = m;
        emit maxHpChanged(m, QPrivateSignal());
    }
}

/**
 * @brief getter of property @c upgradePoint
 * @return @c upgradePoint
 */
int QMdmmPlayer::upgradePoint() const noexcept
{
    return d->upgradePoint;
}

/**
 * @brief setter of property @c upgradePoint
 * @param u @c upgradePoint
 */
void QMdmmPlayer::setUpgradePoint(int u)
{
    if (d->upgradePoint != u) {
        d->upgradePoint = u;
        emit upgradePointChanged(u, QPrivateSignal());
    }
}

/**
 * @brief getter of property @c dead
 * @return @c dead
 */
bool QMdmmPlayer::dead() const
{
    bool zeroHpAsDead = room()->logicConfiguration().zeroHpAsDead();
    return zeroHpAsDead ? (d->hp <= 0) : (d->hp < 0);
}

/**
 * @fn QMdmmPlayer::alive() const
 * @brief getter of property @c alive
 * @return @c alive
 */

/**
 * @brief If the player can buy knife
 * @return @c true if able, @c false if not
 *
 * @sa QMdmmLogicConfiguration::canBuyOnlyInInitialCity
 */
bool QMdmmPlayer::canBuyKnife() const
{
    return alive() && !hasKnife() && (room()->logicConfiguration().canBuyOnlyInInitialCity() ? (place() == initialPlace()) : (place() != QMdmmData::Country));
}

/**
 * @brief If the player can buy horse
 * @return @c true if able, @c false if not
 *
 * @sa QMdmmLogicConfiguration::canBuyOnlyInInitialCity
 */
bool QMdmmPlayer::canBuyHorse() const
{
    return alive() && !hasHorse() && (room()->logicConfiguration().canBuyOnlyInInitialCity() ? (place() == initialPlace()) : (place() != QMdmmData::Country));
}

/**
 * @brief If the player can slash a specific player
 * @param to the target player
 * @return @c true if able, @c false if not
 *
 * @note A player cannot slash himself / herself.
 */
bool QMdmmPlayer::canSlash(const QMdmmPlayer *to) const
{
    Q_ASSERT(room() == to->room());

    if (dead() || to->dead())
        return false;

    if (!hasKnife())
        return false;

    if (this == to)
        return false;

    if (place() != to->place())
        return false;

    return true;
}

/**
 * @brief If the player can kick a specific player
 * @param to the target player
 * @return @c true if able, @c false if not
 *
 * @note A player cannot kick himself / herself.
 */
bool QMdmmPlayer::canKick(const QMdmmPlayer *to) const
{
    Q_ASSERT(room() == to->room());

    if (dead() || to->dead())
        return false;

    if (!hasHorse())
        return false;

    if (this == to)
        return false;

    if ((place() != to->place()) || (place() == QMdmmData::Country))
        return false;

    return true;
}

/**
 * @brief If the player can move to a specific place
 * @param toPlace the target place
 * @return @c true if able, @c false if not
 */
bool QMdmmPlayer::canMove(int toPlace) const
{
    return alive() && QMdmmData::isPlaceAdjacent(place(), toPlace);
}

/**
 * @brief If the player can make a specific player move
 * @param to the target player
 * @param toPlace the target place
 * @return @c true if able, @c false if not
 *
 * @note if @c this == @c to then this function is same as @c canMove()
 *
 * @sa QMdmmLogicConfiguration::enableLetMove
 */
bool QMdmmPlayer::canLetMove(const QMdmmPlayer *to, int toPlace) const
{
    Q_ASSERT(room() == to->room());

    if (this == to)
        return canMove(toPlace);

    if (!room()->logicConfiguration().enableLetMove())
        return false;

    if (dead() || to->dead())
        return false;

    // one movement should move player to adjacent place only
    if (!QMdmmData::isPlaceAdjacent(to->place(), toPlace))
        return false;

    // case 1: pull a player in adjacent place to self's place
    if (QMdmmData::isPlaceAdjacent(place(), to->place()) && toPlace == place())
        return true;

    // case 2: push a player in same place to adjacent place
    if (place() == to->place())
        return true;

    return false;
}

/**
 * @brief get the remained times a player can upgrade knife damage
 * @return the remained times a player can upgrade knife damage
 *
 * @sa QMdmmLogicConfiguration::maximumKnifeDamage
 */
int QMdmmPlayer::upgradeKnifeRemainingTimes() const
{
    return room()->logicConfiguration().maximumKnifeDamage() - knifeDamage();
}

/**
 * @brief get the remained times a player can upgrade horse damage
 * @return the remained times a player can upgrade horse damage
 *
 * @sa QMdmmLogicConfiguration::maximumHorseDamage
 */
int QMdmmPlayer::upgradeHorseRemainingTimes() const
{
    return room()->logicConfiguration().maximumHorseDamage() - horseDamage();
}

/**
 * @brief get the remained times a player can upgrade maximum HP
 * @return the remained times a player can upgrade maximum HP
 *
 * @sa QMdmmLogicConfiguration::maximumMaxHp
 */
int QMdmmPlayer::upgradeMaxHpRemainingTimes() const
{
    return room()->logicConfiguration().maximumMaxHp() - maxHp();
}

/**
 * @fn QMdmmPlayer::canUpgradeKnife() const
 * @brief if a player can upgrade knife damage
 * @return @c true if able, @c false if not
 */

/**
 * @fn QMdmmPlayer::canUpgradeHorse() const
 * @brief if a player can upgrade horse damage
 * @return @c true if able, @c false if not
 */

/**
 * @fn QMdmmPlayer::canUpgradeMaxHp() const
 * @brief if a player can upgrade maximum HP
 * @return @c true if able, @c false if not
 */

/**
 * @brief Action: Buy knife
 * @return @c true if succeed, @c false if not
 *
 * @sa @c canBuyKnife()
 */
bool QMdmmPlayer::buyKnife()
{
    if (!canBuyKnife())
        return false;

    setHasKnife(true);
    return true;
}

/**
 * @brief Action: Buy horse
 * @return @c true if succeed, @c false if not
 *
 * @sa @c canBuyHorse()
 */
bool QMdmmPlayer::buyHorse()
{
    if (!canBuyHorse())
        return false;

    setHasHorse(true);
    return true;
}

/**
 * @brief Action: Slash a specific player
 * @param to the target player
 * @return @c true if succeed, @c false if not
 *
 * If punish HP is enabled (@c QMdmmLogicConfiguration::punishHpModifier > 0) and this slash is occurred in city, the punish HP will take into account.
 *
 * @sa @c canSlash()
 */
bool QMdmmPlayer::slash(QMdmmPlayer *to)
{
    Q_ASSERT(room() == to->room());

    if (!canSlash(to))
        return false;

    QMdmmPlayerPrivate::applyDamage(this, to, knifeDamage(), QMdmmData::Slashed);

    if (place() != QMdmmData::Country) {
        int punishHpModifier = room()->logicConfiguration().punishHpModifier();
        if (punishHpModifier > 0) {
            QMdmmLogicConfiguration::PunishHpRoundStrategy punishHpRoundStrategy = room()->logicConfiguration().punishHpRoundStrategy();

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
                QMdmmPlayerPrivate::applyDamage(to, this, punishedHp, QMdmmData::HpPunished);
        }
    }

    return true;
}

/**
 * @brief Action: Kick a specific player
 * @param to the target player
 * @return @c true if succeed, @c false if not
 *
 * @sa @c canKick()
 */
bool QMdmmPlayer::kick(QMdmmPlayer *to)
{
    Q_ASSERT(room() == to->room());

    if (!canKick(to))
        return false;

    QMdmmPlayerPrivate::applyDamage(this, to, horseDamage(), QMdmmData::Kicked);

    if (!to->dead())
        to->setPlace(QMdmmData::Country);

    return true;
}

/**
 * @brief Action: Move to a specific place
 * @param toPlace the target place
 * @return @c true if succeed, @c false if not
 *
 * Not to confuse with @c setPlace() .
 *
 * @sa @c canMove()
 */
bool QMdmmPlayer::move(int toPlace)
{
    if (canMove(toPlace)) {
        setPlace(toPlace);
        return true;
    }

    return false;
}

/**
 * @brief Action: Let a specific player move to a specific place
 * @param to the target player
 * @param toPlace the target place
 * @return @c true if succeed, @c false if not
 *
 * @sa @c canLetMove()
 */
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

/**
 * @brief Action: Do nothing
 * @return @c true (always succeed)
 */
bool QMdmmPlayer::doNothing() // NOLINT(readability-make-member-function-const): Operation is ought to be not const
{
    return true;
}
/**
 * @brief upgrade knife damage by one point
 * @return @c true if succeed, @c false if not
 */
bool QMdmmPlayer::upgradeKnife()
{
    if (!canUpgradeKnife())
        return false;

    setKnifeDamage(knifeDamage() + 1);
    return true;
}

/**
 * @brief upgrade horse damage by one point
 * @return @c true if succeed, @c false if not
 */
bool QMdmmPlayer::upgradeHorse()
{
    if (!canUpgradeHorse())
        return false;

    setHorseDamage(horseDamage() + 1);
    return true;
}

/**
 * @brief upgrade maximum HP by one point
 * @return @c true if succeed, @c false if not
 */
bool QMdmmPlayer::upgradeMaxHp()
{
    if (!canUpgradeMaxHp())
        return false;

    setMaxHp(maxHp() + 1);
    return true;
}

/**
 * @brief reset all status to initial state of a round
 * @param seat the seat number (a.k.a. initial place)
 */
void QMdmmPlayer::prepareForRoundStart(int seat)
{
    setHasKnife(false);
    setHasHorse(false);
    setHp(maxHp());
    setInitialPlace(seat);
    setPlace(seat);
    setUpgradePoint(0);
}

/**
 * @brief reset all upgrades.
 *
 * Useful when a player leaves or joins a room between rounds, which causes a game over
 */
void QMdmmPlayer::resetUpgrades()
{
    setMaxHp(room()->logicConfiguration().initialMaxHp());
    setKnifeDamage(room()->logicConfiguration().initialKnifeDamage());
    setHorseDamage(room()->logicConfiguration().initialHorseDamage());
    setUpgradePoint(0);
}

/**
 * @fn QMdmmPlayer::hasKnifeChanged(bool hasKnife, QPrivateSignal)
 * @brief notify signal for property @c hasKnife
 */

/**
 * @fn QMdmmPlayer::hasHorseChanged(bool hasHorse, QPrivateSignal)
 * @brief notify signal for property @c hasHorse
 */

/**
 * @fn QMdmmPlayer::hpChanged(int hp, QPrivateSignal)
 * @brief notify signal for property @c hp
 */

/**
 * @fn QMdmmPlayer::placeChanged(int place, QPrivateSignal)
 * @brief notify signal for property @c place
 */

/**
 * @fn QMdmmPlayer::initialPlaceChanged(int initialPlace, QPrivateSignal)
 * @brief notify signal for property @c initialPlace
 */

/**
 * @fn QMdmmPlayer::knifeDamageChanged(int knifeDamage, QPrivateSignal)
 * @brief notify signal for property @c knifeDamage
 */

/**
 * @fn QMdmmPlayer::horseDamageChanged(int horseDamage, QPrivateSignal)
 * @brief notify signal for property @c horseDamage
 */

/**
 * @fn QMdmmPlayer::maxHpChanged(int maxHp, QPrivateSignal)
 * @brief notify signal for property @c maxHp
 */

/**
 * @fn QMdmmPlayer::upgradePointChanged(int upgradePoint, QPrivateSignal)
 * @brief notify signal for property @c upgradePoint
 */

/**
 * @fn QMdmmPlayer::damaged(const QString &from, int damagePoint, QMdmmData::DamageReason reason, QPrivateSignal)
 * @brief emitted when this player is damaged
 *
 * @note This is emitted later of the 2 overloads.
 * Since it is not determined which is the better overload, temporarily keeping these 2.
 */

/**
 * @fn QMdmmPlayer::damaged(const QMdmmPlayer *from, int damagePoint, QMdmmData::DamageReason reason, QPrivateSignal)
 * @brief emitted when this player is damaged
 *
 * @note This is emitted earlier of the 2 overloads.
 * Since it is not determined which is the better overload, temporarily keeping these 2.
 */

/**
 * @fn QMdmmPlayer::die(QPrivateSignal)
 * @brief emitted when the player dies
 *
 * Acts as the notify signal for property @c alive and @c dead
 */
