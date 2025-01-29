// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMROOM_H
#define QMDMMROOM_H

#include "qmdmmcoreglobal.h"

#include <QJsonObject>
#include <QList>
#include <QObject>

class QMdmmPlayer;
class QMdmmLogic;

class QMDMMCORE_EXPORT QMdmmLogicConfiguration final : public QJsonObject
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
    static QMDMMCORE_EXPORT const QMdmmLogicConfiguration &defaults();
    static QMDMMCORE_EXPORT const QMdmmLogicConfiguration &v1();

    enum PunishHpRoundStrategy
    {
        RoundDown,
        RoundToNearest45,
        RoundUp,
        PlusOne,
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

struct QMdmmRoomPrivate;

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
    [[nodiscard]] int alivePlayersCount() const
    {
        return alivePlayers().size();
    }
    [[nodiscard]] bool isRoundOver() const
    {
        return alivePlayersCount() <= 1;
    }

    [[nodiscard]] bool isGameOver(QStringList *winnerPlayerNames = nullptr) const;

    void prepareForRoundStart();
    void resetUpgrades();

signals:
    void playerAdded(const QString &playerName, QPrivateSignal);
    void playerRemoved(const QString &playerName, QPrivateSignal);

private:
    const std::unique_ptr<QMdmmRoomPrivate> d;
    Q_DISABLE_COPY_MOVE(QMdmmRoom)
};

#endif // QMDMMROOM_H
