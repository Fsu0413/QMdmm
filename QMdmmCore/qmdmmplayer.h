#ifndef QMDMMPLAYER_H
#define QMDMMPLAYER_H

#include "qmdmmcoreglobal.h"

struct QMdmmPlayerPrivate;

class QMDMMCORE_EXPORT QMdmmPlayer
{
public:
    QMdmmPlayer();
    virtual ~QMdmmPlayer();

    // property getters
    // current property
    bool hasKnife() const;
    bool hasHorse() const;
    int hp() const;
    QMdmmData::Place place() const;

    // upgradeable property
    int knifeDamage() const;
    int horseDamage() const;
    int maxHp() const;

    // calculated property
    bool dead() const;
    bool alive() const;

    int upgradePoint() const;

    // actions
    bool buyKnife();
    bool buyHorse();

    bool slash(QMdmmPlayer *to);
    bool kick(QMdmmPlayer *to);

    bool move(QMdmmData::Place toPlace);
    bool letMove(QMdmmPlayer *to, QMdmmData::Place toPlace);

    bool doNothing();

    void damage(QMdmmPlayer *from, int damagePoint, QMdmmData::DamageReason reason);
    void placeChange(QMdmmData::Place toPlace);

    // updates
    bool upgradeKnife();
    bool upgradeHorse();
    bool upgradeMaxHp();

    void prepareForGameStart(int playerNum);

private:
    QMDMM_D(QMdmmPlayer)
    QMDMM_DISABLE_COPY(QMdmmPlayer)
};

#endif // QMDMMPLAYER_H
