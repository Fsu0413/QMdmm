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
    Q_PROPERTY(bool hasKnife READ hasKnife WRITE setHasKnife NOTIFY hasKnifeChanged DESIGNABLE false FINAL)
    Q_PROPERTY(bool hasHorse READ hasHorse WRITE setHasHorse NOTIFY hasHorseChanged DESIGNABLE false FINAL)
    Q_PROPERTY(int hp READ hp WRITE setHp NOTIFY hpChanged DESIGNABLE false FINAL)
    Q_PROPERTY(int place READ place WRITE setPlace NOTIFY placeChanged DESIGNABLE false FINAL)
    Q_PROPERTY(int knifeDamage READ knifeDamage WRITE setKnifeDamage NOTIFY knifeDamageChanged DESIGNABLE false FINAL)
    Q_PROPERTY(int horseDamage READ horseDamage WRITE setHorseDamage NOTIFY horseDamageChanged DESIGNABLE false FINAL)
    Q_PROPERTY(int maxHp READ maxHp WRITE setMaxHp NOTIFY maxHpChanged DESIGNABLE false FINAL)
    Q_PROPERTY(int upgradePoint READ upgradePoint WRITE setUpgradePoint NOTIFY upgradePointChanged DESIGNABLE false FINAL)

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
    [[nodiscard]] int place() const;
    void setPlace(int toPlace);

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
    [[nodiscard]] bool canMove(int toPlace) const;
    [[nodiscard]] bool canLetMove(const QMdmmPlayer *to, int toPlace) const;

    // upgrade checks
    [[nodiscard]] int upgradeKnifeRemainingTimes() const;
    [[nodiscard]] int upgradeHorseRemainingTimes() const;
    [[nodiscard]] int upgradeMaxHpRemainingTimes() const;

    [[nodiscard]] constexpr bool canUpdateKnife() const
    {
        return upgradeKnifeRemainingTimes() > 0;
    }
    [[nodiscard]] constexpr bool canUpdateHorse() const
    {
        return upgradeHorseRemainingTimes() > 0;
    }
    [[nodiscard]] constexpr bool canUpdateMaxHp() const
    {
        return upgradeMaxHpRemainingTimes() > 0;
    }

public slots: // NOLINT(readability-redundant-access-specifiers)
    // actions
    bool buyKnife();
    bool buyHorse();

    bool slash(QMdmmPlayer *to);
    bool kick(QMdmmPlayer *to);

    bool move(int toPlace);
    bool letMove(QMdmmPlayer *to, int toPlace);

    bool doNothing();

    // action runs
    void applyDamage(QMdmmPlayer *from, int damagePoint, QMdmmData::DamageReason reason);

    // upgrades
    bool upgradeKnife();
    bool upgradeHorse();
    bool upgradeMaxHp();

    void prepareForRoundStart(int playerNum);
    void resetUpgrades();

signals:
    void hasKnifeChanged(bool hasKnife, QPrivateSignal);
    void hasHorseChanged(bool hasHorse, QPrivateSignal);
    void hpChanged(int hp, QPrivateSignal);
    void placeChanged(int place, QPrivateSignal);
    void knifeDamageChanged(int knifeDamage, QPrivateSignal);
    void horseDamageChanged(int horseDamage, QPrivateSignal);
    void maxHpChanged(int maxHp, QPrivateSignal);
    void upgradePointChanged(int upgradePoint, QPrivateSignal);

    // it is to be decided which of following 2 declarations is more easy to use so...
    // Temporarily keep these 2.
    void damaged(const QString &from, int damagePoint, QMdmmData::DamageReason reason, QPrivateSignal);
    void damaged(const QMdmmPlayer *from, int damagePoint, QMdmmData::DamageReason reason, QPrivateSignal);
    void die(QPrivateSignal);

private:
    QMdmmPlayerPrivate *const d;
    Q_DISABLE_COPY_MOVE(QMdmmPlayer)
};

#endif // QMDMMPLAYER_H
