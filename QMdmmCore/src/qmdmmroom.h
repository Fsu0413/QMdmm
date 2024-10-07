// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMROOM_H
#define QMDMMROOM_H

#include "qmdmmcoreglobal.h"

#include <QJsonObject>
#include <QList>
#include <QObject>

class QMdmmPlayer;
class QMdmmLogic;

struct QMdmmRoomPrivate;

struct QMDMMCORE_EXPORT QMdmmLogicConfiguration final : public QJsonObject
{
    Q_GADGET
    Q_PROPERTY(int playerNumPerRoom READ playerNumPerRoom WRITE setPlayerNumPerRoom DESIGNABLE false FINAL)
    Q_PROPERTY(int requestTimeout READ requestTimeout WRITE setRequestTimeout DESIGNABLE false FINAL)
    Q_PROPERTY(int initialKnifeDamage READ initialKnifeDamage WRITE setInitialKnifeDamage DESIGNABLE false FINAL)
    Q_PROPERTY(int maximumKnifeDamage READ maximumKnifeDamage WRITE setMaximumKnifeDamage DESIGNABLE false FINAL)
    Q_PROPERTY(int initialHorseDamage READ initialHorseDamage WRITE setInitialHorseDamage DESIGNABLE false FINAL)
    Q_PROPERTY(int maximumHorseDamage READ maximumHorseDamage WRITE setMaximumHorseDamage DESIGNABLE false FINAL)
    Q_PROPERTY(int initialMaxHp READ initialMaxHp WRITE setInitialMaxHp DESIGNABLE false FINAL)
    Q_PROPERTY(int maximumMaxHp READ maximumMaxHp WRITE setMaximumMaxHp DESIGNABLE false FINAL)
    Q_PROPERTY(int punishHpModifier READ punishHpModifier WRITE setPunishHpModifier DESIGNABLE false FINAL)
    Q_PROPERTY(QMdmmLogicConfiguration::PunishHpRoundStrategy punishHpRoundStrategy READ punishHpRoundStrategy WRITE setPunishHpRoundStrategy DESIGNABLE false FINAL)
    Q_PROPERTY(bool zeroHpAsDead READ zeroHpAsDead WRITE setZeroHpAsDead DESIGNABLE false FINAL)
    Q_PROPERTY(bool enableLetMove READ enableLetMove WRITE setEnableLetMove DESIGNABLE false FINAL)
    Q_PROPERTY(bool canBuyOnlyInInitialCity READ canBuyOnlyInInitialCity WRITE setCanBuyOnlyInInitialCity DESIGNABLE false FINAL)

public:
    static const QMdmmLogicConfiguration &defaults();

    enum PunishHpRoundStrategy
    {
        RoundDown, // 1.1 -> 1, 1.4 -> 1, 1.5 -> 1, 1.9 -> 1, 2.0 -> 2
        RoundToNearest45, // 1.1 -> 1, 1.4 -> 1, 1.5 -> 2, 1.9 -> 2, 2.0 -> 2
        RoundUp, // 1.1 -> 2, 1.4 -> 2, 1.5 -> 2, 1.9 -> 2, 2.0 -> 2
        PlusOne, // 1.1 -> 2, 1.4 -> 2, 1.5 -> 2, 1.9 -> 2, 2.0 -> 3
    };
    Q_ENUM(PunishHpRoundStrategy);

#ifdef Q_MOC_RUN
    Q_INVOKABLE QMdmmLogicConfiguration();
    Q_INVOKABLE QMdmmLogicConfiguration(const QMdmmLogicConfiguration &);
#else
    using QJsonObject::QJsonObject;
    using QJsonObject::operator=;
#endif

#define DEFINE_CONFIGURATION(type, valueName, ValueName) \
    [[nodiscard]] type valueName() const;                \
    void set##ValueName(type valueName);

    // TODO: move to ServerConfiguration
    DEFINE_CONFIGURATION(int, playerNumPerRoom, PlayerNumPerRoom)
    DEFINE_CONFIGURATION(int, requestTimeout, RequestTimeout)

    // I had experienced 2 versions of MDMM, they are mostly same but with minor differences.
    // In both versions one can only buy k/h in city.
    // In both versions one can slash only if knife is bought and kick only if horse is bought.
    // In both versions one can only slash / kick other one when they are at same place.
    // In both versions one can kick other one in city, and the kicked one will be force moved to country.
    // In both versions one can only move to adjecent place at a time. Different cities are not adjecent, while country is adjecent to each city.
    // Version 1 (Legacy): initial 7/1/3 mh/kd/hd, maximum 7/3/5 mh/kd/hd (Yeah, no maxHp upgrade). One with zero HP still alive. No "Let move"s, no punish HP.
    // This is the version I had experienced in primary school.
    // This version knife upgrades are more valuable. Horse is of no value since there is no HP punish.
    // Version 2: initial 10/1/2 mh/kd/hd, maximum 20/10/10 mh/kd/hd. One with zero HP dies. With "Let move"s (pull sb. in / push sb. out of city stuff), with punish HP.
    // This is the version I had experienced in junior high school.
    // This version horse can be effectly used in multi-player. pull - kick loop is fun!

    // Default configurations matches rules of Version 2 and can be tweaked.

    // standard configuration
    DEFINE_CONFIGURATION(int, initialKnifeDamage, InitialKnifeDamage)
    DEFINE_CONFIGURATION(int, maximumKnifeDamage, MaximumKnifeDamage)
    DEFINE_CONFIGURATION(int, initialHorseDamage, InitialHorseDamage)
    DEFINE_CONFIGURATION(int, maximumHorseDamage, MaximumHorseDamage)
    DEFINE_CONFIGURATION(int, initialMaxHp, InitialMaxHp)
    DEFINE_CONFIGURATION(int, maximumMaxHp, MaximumMaxHp)

    // legacy experience
    DEFINE_CONFIGURATION(int, punishHpModifier, PunishHpModifier)
    DEFINE_CONFIGURATION(QMdmmLogicConfiguration::PunishHpRoundStrategy, punishHpRoundStrategy, PunishHpRoundStrategy)
    DEFINE_CONFIGURATION(bool, zeroHpAsDead, ZeroHpAsDead)
    DEFINE_CONFIGURATION(bool, enableLetMove, EnableLetMove)
    DEFINE_CONFIGURATION(bool, canBuyOnlyInInitialCity, CanBuyOnlyInInitialCity)

#undef DEFINE_CONFIGURATION

    bool deserialize(const QJsonValue &value);
};

class QMDMMCORE_EXPORT QMdmmRoom final : public QObject
{
    Q_OBJECT

public:
    explicit QMdmmRoom(QMdmmLogicConfiguration logicConfiguration, QObject *parent = nullptr);
    ~QMdmmRoom() override;

    [[nodiscard]] const QMdmmLogicConfiguration &logicConfiguration() const;
    void setLogicConfiguration(const QMdmmLogicConfiguration &logicConfiguration);

    QMdmmPlayer *addPlayer(const QString &playerName);
    bool removePlayer(const QString &playerName);

    [[nodiscard]] QMdmmPlayer *player(const QString &playerName);
    [[nodiscard]] const QMdmmPlayer *player(const QString &playerName) const;

    [[nodiscard]] QList<QMdmmPlayer *> players();
    [[nodiscard]] QList<const QMdmmPlayer *> players() const;
    [[nodiscard]] QStringList playerNames() const;

    [[nodiscard]] QList<QMdmmPlayer *> alivePlayers();
    [[nodiscard]] QList<const QMdmmPlayer *> alivePlayers() const;
    [[nodiscard]] QStringList alivePlayerNames() const;
    [[nodiscard]] int alivePlayersCount() const;
    [[nodiscard]] bool isRoundOver() const;
    [[nodiscard]] bool isGameOver(QStringList *winnerPlayerNames = nullptr) const;

    void prepareForRoundStart();
    void resetUpgrades();

signals:
    void playerAdded(const QString &playerName, QPrivateSignal);
    void playerRemoved(const QString &playerName, QPrivateSignal);

private:
    QMdmmRoomPrivate *const d;
    Q_DISABLE_COPY_MOVE(QMdmmRoom)
};

#endif // QMDMMROOM_H
