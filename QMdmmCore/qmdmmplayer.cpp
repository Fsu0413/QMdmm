#include "qmdmmplayer.h"

struct QMdmmPlayerPrivate
{
    constexpr QMdmmPlayerPrivate()
        : name()
        , knife(false)
        , horse(false)
        , hp(10)
        , place(QMdmmData::Country)
        , knifeDamage(1)
        , horseDamage(2)
        , maxHp(10)
        , upgradePoint(0)
    {
    }
    std::string name;

    bool knife;
    bool horse;
    int hp;
    QMdmmData::Place place;

    int knifeDamage;
    int horseDamage;
    int maxHp;

    int upgradePoint;
};

QMdmmPlayer::QMdmmPlayer()
    : d_ptr(new QMdmmPlayerPrivate)
{
}

QMdmmPlayer::~QMdmmPlayer()
{
    delete d_ptr;
}

void QMdmmPlayer::setName(const std::string &name)
{
    QMDMMD(QMdmmPlayer);
    d->name = name;
}

std::string QMdmmPlayer::name() const
{
    QMDMMD(const QMdmmPlayer);
    return d->name;
}

bool QMdmmPlayer::hasKnife() const
{
    QMDMMD(const QMdmmPlayer);
    return d->knife;
}

bool QMdmmPlayer::hasHorse() const
{
    QMDMMD(const QMdmmPlayer);
    return d->horse;
}

int QMdmmPlayer::hp() const
{
    QMDMMD(const QMdmmPlayer);
    return d->hp;
}

QMdmmData::Place QMdmmPlayer::place() const
{
    QMDMMD(const QMdmmPlayer);
    return d->place;
}

int QMdmmPlayer::knifeDamage() const
{
    QMDMMD(const QMdmmPlayer);
    return d->knifeDamage;
}

int QMdmmPlayer::horseDamage() const
{
    QMDMMD(const QMdmmPlayer);
    return d->horseDamage;
}

int QMdmmPlayer::maxHp() const
{
    QMDMMD(const QMdmmPlayer);
    return d->maxHp;
}

bool QMdmmPlayer::dead() const
{
    QMDMMD(const QMdmmPlayer);
    return d->hp <= 0;
}

bool QMdmmPlayer::alive() const
{
    return !dead();
}

int QMdmmPlayer::upgradePoint() const
{
    QMDMMD(const QMdmmPlayer);
    return d->upgradePoint;
}

bool QMdmmPlayer::canBuyKnife() const
{
    QMDMMD(const QMdmmPlayer);
    if (d->knife)
        return false;

    return true;
}

bool QMdmmPlayer::canBuyHorse() const
{
    QMDMMD(const QMdmmPlayer);
    if (d->horse)
        return false;

    return true;
}

bool QMdmmPlayer::canSlash(const QMdmmPlayer *to) const
{
    QMDMMD(const QMdmmPlayer);
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
    QMDMMD(const QMdmmPlayer);
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
    QMDMMD(const QMdmmPlayer);
    return QMdmmData::isPlaceAdjecent(d->place, toPlace);
}

bool QMdmmPlayer::canLetMove(const QMdmmPlayer *to, QMdmmData::Place toPlace) const
{
    if (to->dead())
        return false;

    QMDMMD(const QMdmmPlayer);
    return QMdmmData::isPlaceAdjecent(to->place(), toPlace) && ((QMdmmData::isPlaceAdjecent(d->place, to->place()) && (toPlace == d->place)) || (d->place == to->place()));
}

bool QMdmmPlayer::buyKnife()
{
    if (!canBuyKnife())
        return false;

    QMDMMD(QMdmmPlayer);
    d->knife = true;
    return true;
}

bool QMdmmPlayer::buyHorse()
{
    if (!canBuyHorse())
        return false;

    QMDMMD(QMdmmPlayer);
    d->horse = true;
    return true;
}

bool QMdmmPlayer::slash(QMdmmPlayer *to)
{
    if (!canSlash(to))
        return false;

    QMDMMD(QMdmmPlayer);
    to->damage(this, d->knifeDamage, QMdmmData::Slash);

    if (place() != QMdmmData::Country) {
        // life punishment: damage point is maxHp / 2
        damage(to, maxHp() / 2, QMdmmData::PunishHp);
    }

    return true;
}

bool QMdmmPlayer::kick(QMdmmPlayer *to)
{
    if (!canKick(to))
        return false;

    QMDMMD(QMdmmPlayer);
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

bool QMdmmPlayer::letMove(QMdmmPlayer *to, QMdmmData::Place toPlace)
{
    // pull, push, kick(effect)

    if (canLetMove(to, toPlace)) {
        to->placeChange(toPlace);
        return true;
    }

    return false;
}

bool QMdmmPlayer::doNothing(const std::string &)
{
    return true;
}

void QMdmmPlayer::damage(QMdmmPlayer *from, int damagePoint, QMdmmData::DamageReason reason)
{
    QMDMM_UNUSED(reason);
    QMDMMD(QMdmmPlayer);
    d->hp -= damagePoint;

    if (dead())
        ++(from->d_func()->upgradePoint);
}

void QMdmmPlayer::placeChange(QMdmmData::Place toPlace)
{
    QMDMMD(QMdmmPlayer);
    d->place = toPlace;
}

bool QMdmmPlayer::upgradeKnife()
{
    QMDMMD(QMdmmPlayer);

    if (d->knifeDamage >= 5)
        return false;

    ++d->knifeDamage;
    return true;
}

bool QMdmmPlayer::upgradeHorse()
{
    QMDMMD(QMdmmPlayer);

    if (d->horseDamage >= 10)
        return false;

    ++d->horseDamage;
    return true;
}

bool QMdmmPlayer::upgradeMaxHp()
{
    QMDMMD(QMdmmPlayer);

    if (d->maxHp >= 20)
        return false;

    ++d->maxHp;
    return true;
}

void QMdmmPlayer::prepareForGameStart(int playerNum)
{
    QMDMMD(QMdmmPlayer);
    d->hp = d->maxHp;
    d->place = static_cast<QMdmmData::Place>(playerNum);
    d->knife = false;
    d->horse = false;
    d->upgradePoint = 0;
}
