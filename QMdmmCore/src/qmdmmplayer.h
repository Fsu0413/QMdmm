// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMPLAYER_H
#define QMDMMPLAYER_H

#include "qmdmmcoreglobal.h"
#include <QObject>

struct QMdmmPlayerPrivate;

class QMDMMCORE_EXPORT QMdmmPlayer final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasKnife READ hasKnife NOTIFY hasKnifeChanged DESIGNABLE false)
    Q_PROPERTY(bool hasHorse READ hasHorse NOTIFY hasHorseChanged DESIGNABLE false)
    Q_PROPERTY(int hp READ hp NOTIFY hpChanged DESIGNABLE false)
    Q_PROPERTY(QMdmmData::Place place READ place NOTIFY placeChanged DESIGNABLE false)
    Q_PROPERTY(int knifeDamage READ knifeDamage NOTIFY knifeDamageChanged DESIGNABLE false)
    Q_PROPERTY(int horseDamage READ horseDamage NOTIFY horseDamageChanged DESIGNABLE false)
    Q_PROPERTY(int maxHp READ maxHp NOTIFY maxHpChanged DESIGNABLE false)
    Q_PROPERTY(bool dead READ dead STORED false NOTIFY deadChanged DESIGNABLE false)
    Q_PROPERTY(bool alive READ alive STORED false NOTIFY aliveChanged DESIGNABLE false)
    Q_PROPERTY(int upgradePoint READ upgradePoint NOTIFY upgradePointChanged DESIGNABLE false)

public:
    explicit QMdmmPlayer(const QString &name, QObject *parent = nullptr);
    ~QMdmmPlayer() override;

    // property setters/getters

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

    // action checks
    bool canBuyKnife() const;
    bool canBuyHorse() const;
    bool canSlash(const QMdmmPlayer *to) const;
    bool canKick(const QMdmmPlayer *to) const;
    bool canMove(QMdmmData::Place toPlace) const;
    bool canLetMove(const QMdmmPlayer *to, QMdmmData::Place toPlace) const;

public slots:
    // actions
    bool buyKnife();
    bool buyHorse();

    bool slash(QMdmmPlayer *to);
    bool kick(QMdmmPlayer *to);

    bool move(QMdmmData::Place toPlace);
    bool letMove(QMdmmPlayer *to, QMdmmData::Place toPlace);

    bool doNothing(const QString &reason);

    // action runs
    void damage(QMdmmPlayer *from, int damagePoint, QMdmmData::DamageReason reason);
    void placeChange(QMdmmData::Place toPlace);

    // upgrades
    bool upgradeKnife();
    bool upgradeHorse();
    bool upgradeMaxHp();

    void prepareForGameStart(int playerNum);

signals:
    void hasKnifeChanged(bool, QPrivateSignal);
    void hasHorseChanged(bool, QPrivateSignal);
    void hpChanged(int, QPrivateSignal);
    void placeChanged(QMdmmData::Place, QPrivateSignal);
    void knifeDamageChanged(int, QPrivateSignal);
    void horseDamageChanged(int, QPrivateSignal);
    void maxHpChanged(int, QPrivateSignal);
    void deadChanged(bool, QPrivateSignal);
    void aliveChanged(bool, QPrivateSignal);
    void upgradePointChanged(int, QPrivateSignal);

private:
    QMdmmPlayerPrivate *const d;
    Q_DISABLE_COPY(QMdmmPlayer)
};

#endif // QMDMMPLAYER_H