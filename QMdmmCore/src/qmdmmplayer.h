// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMPLAYER_H
#define QMDMMPLAYER_H

#include "qmdmmcoreglobal.h"
#include <QObject>

class QMdmmRoom;

struct QMdmmPlayerPrivate;

class QMDMMCORE_EXPORT QMdmmPlayer final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasKnife READ hasKnife WRITE setHasKnife NOTIFY hasKnifeChanged DESIGNABLE false)
    Q_PROPERTY(bool hasHorse READ hasHorse WRITE setHasHorse NOTIFY hasHorseChanged DESIGNABLE false)
    Q_PROPERTY(int hp READ hp WRITE setHp NOTIFY hpChanged DESIGNABLE false)
    Q_PROPERTY(QMdmmData::Place place READ place WRITE setPlace NOTIFY placeChanged DESIGNABLE false)
    Q_PROPERTY(int knifeDamage READ knifeDamage WRITE setKnifeDamage NOTIFY knifeDamageChanged DESIGNABLE false)
    Q_PROPERTY(int horseDamage READ horseDamage WRITE setHorseDamage NOTIFY horseDamageChanged DESIGNABLE false)
    Q_PROPERTY(int maxHp READ maxHp WRITE setMaxHp NOTIFY maxHpChanged DESIGNABLE false)
    Q_PROPERTY(int upgradePoint READ upgradePoint WRITE setUpgradePoint NOTIFY upgradePointChanged DESIGNABLE false)

public:
    explicit QMdmmPlayer(const QString &name, QMdmmRoom *room /* or parent */);
    ~QMdmmPlayer() override;

    // property setters/getters
    [[nodiscard]] QMdmmRoom *room();
    [[nodiscard]] const QMdmmRoom *room() const;

    // current property
    [[nodiscard]] bool hasKnife() const;
    void setHasKnife(bool k);
    [[nodiscard]] bool hasHorse() const;
    void setHasHorse(bool h);
    [[nodiscard]] int hp() const;
    void setHp(int h, bool *kills = nullptr);
    [[nodiscard]] QMdmmData::Place place() const;
    void setPlace(QMdmmData::Place toPlace);

    // upgradeable property
    [[nodiscard]] int knifeDamage() const;
    void setKnifeDamage(int k);
    [[nodiscard]] int horseDamage() const;
    void setHorseDamage(int h);
    [[nodiscard]] int maxHp() const;
    void setMaxHp(int m);
    [[nodiscard]] int upgradePoint() const;
    void setUpgradePoint(int u);

    // calculated property
    [[nodiscard]] bool dead() const;
    [[nodiscard]] bool alive() const;

    // action checks
    [[nodiscard]] bool canBuyKnife() const;
    [[nodiscard]] bool canBuyHorse() const;
    [[nodiscard]] bool canSlash(const QMdmmPlayer *to) const;
    [[nodiscard]] bool canKick(const QMdmmPlayer *to) const;
    [[nodiscard]] bool canMove(QMdmmData::Place toPlace) const;
    [[nodiscard]] bool canLetMove(const QMdmmPlayer *to, QMdmmData::Place toPlace) const;

public slots: // NOLINT(readability-redundant-access-specifiers)
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
    void upgradePointChanged(int, QPrivateSignal);

    void die(QPrivateSignal);

private:
    QMdmmPlayerPrivate *const d;
    Q_DISABLE_COPY_MOVE(QMdmmPlayer)
};

#endif // QMDMMPLAYER_H
