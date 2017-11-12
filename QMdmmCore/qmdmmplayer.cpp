#include "qmdmmplayer.h"

struct QMdmmPlayerPrivate
{
    constexpr QMdmmPlayerPrivate()
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

QMdmmPlayer::QMdmmPlayer()
    : d_ptr(new QMdmmPlayerPrivate)
{
}

QMdmmPlayer::~QMdmmPlayer()
{
    delete d_ptr;
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

bool QMdmmPlayer::buyKnife()
{
    QMDMMD(QMdmmPlayer);
    if (d->knife)
        return false;

    d->knife = true;
    return true;
}

bool QMdmmPlayer::buyHorse()
{
    QMDMMD(QMdmmPlayer);
    if (d->horse)
        return false;

    d->horse = true;
    return true;
}

bool QMdmmPlayer::slash(QMdmmPlayer *to)
{
    QMDMMD(QMdmmPlayer);
    if (!d->knife)
        return false;

    if (d->place != to->place())
        return false;

    to->damage(this, d->knifeDamage, QMdmmData::Slash);

    if (d->place != QMdmmData::Country) {
        // punish hp
        damage(to, maxHp() / 2, QMdmmData::PunishHp);
        if (dead())
            return false;
    }
    return true;
}

bool QMdmmPlayer::kick(QMdmmPlayer *to)
{
    QMDMMD(QMdmmPlayer);
    if (!d->horse)
        return false;

    if ((d->place != to->place()) || (d->place == QMdmmData::Country))
        return false;

    to->damage(this, d->horseDamage, QMdmmData::Kick);

    if (!to->dead())
        letMove(to, QMdmmData::Country);

    return true;
}

bool QMdmmPlayer::move(QMdmmData::Place toPlace)
{
    QMDMMD(QMdmmPlayer);
    if (QMdmmData::isPlaceAdjecent(d->place, toPlace)) {
        placeChange(toPlace);
        return true;
    }

    return false;
}

bool QMdmmPlayer::letMove(QMdmmPlayer *to, QMdmmData::Place toPlace)
{
    // pull, push, kick(effect)

    if (QMdmmData::isPlaceAdjecent(to->place(), toPlace)) {
        to->placeChange(toPlace);
        return true;
    }

    return false;
}

bool QMdmmPlayer::doNothing()
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
