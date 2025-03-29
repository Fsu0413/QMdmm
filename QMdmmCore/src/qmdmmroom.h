// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMROOM_H
#define QMDMMROOM_H

#include "qmdmmcoreglobal.h"

#include <QJsonObject>
#include <QList>
#include <QObject>

#include <cstdint>

QMDMM_EXPORT_NAME(QMdmmLogicConfiguration)
QMDMM_EXPORT_NAME(QMdmmRoom)

namespace QMdmmCore {

namespace p {
struct RoomP;
}

#ifndef DOXYGEN
namespace v0 {
#else
inline namespace v1 {
#endif

class Player;
class Logic;

class QMDMMCORE_EXPORT LogicConfiguration final : public QJsonObject
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
    Q_PROPERTY(LogicConfiguration::PunishHpRoundStrategy punishHpRoundStrategy READ punishHpRoundStrategy WRITE setPunishHpRoundStrategy DESIGNABLE false FINAL)
    Q_PROPERTY(bool zeroHpAsDead READ zeroHpAsDead WRITE setZeroHpAsDead DESIGNABLE false FINAL)
    Q_PROPERTY(bool enableLetMove READ enableLetMove WRITE setEnableLetMove DESIGNABLE false FINAL)
    Q_PROPERTY(bool canBuyOnlyInInitialCity READ canBuyOnlyInInitialCity WRITE setCanBuyOnlyInInitialCity DESIGNABLE false FINAL)

public:
    static QMDMMCORE_EXPORT const LogicConfiguration &defaults();
    static QMDMMCORE_EXPORT const LogicConfiguration &v1();

    enum PunishHpRoundStrategy : uint8_t
    {
        RoundDown,
        RoundToNearest45,
        RoundUp,
        PlusOne,
    };
    Q_ENUM(PunishHpRoundStrategy);

#ifdef Q_MOC_RUN
    Q_INVOKABLE LogicConfiguration();
    Q_INVOKABLE LogicConfiguration(const LogicConfiguration &);
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
    DEFINE_CONFIGURATION(LogicConfiguration::PunishHpRoundStrategy, punishHpRoundStrategy, PunishHpRoundStrategy)
    DEFINE_CONFIGURATION(bool, zeroHpAsDead, ZeroHpAsDead)
    DEFINE_CONFIGURATION(bool, enableLetMove, EnableLetMove)
    DEFINE_CONFIGURATION(bool, canBuyOnlyInInitialCity, CanBuyOnlyInInitialCity)

#undef DEFINE_CONFIGURATION

    bool deserialize(const QJsonValue &value);
};

class QMDMMCORE_EXPORT Room final : public QObject
{
    Q_OBJECT

public:
    explicit Room(LogicConfiguration logicConfiguration, QObject *parent = nullptr);
    ~Room() override;

    [[nodiscard]] const LogicConfiguration &logicConfiguration() const;
    void setLogicConfiguration(const LogicConfiguration &logicConfiguration);

    Player *addPlayer(const QString &playerName);
    bool removePlayer(const QString &playerName);

    [[nodiscard]] Player *player(const QString &playerName);
    [[nodiscard]] const Player *player(const QString &playerName) const;

    [[nodiscard]] QList<Player *> players();
    [[nodiscard]] QList<const Player *> players() const;
    [[nodiscard]] QStringList playerNames() const;

    [[nodiscard]] QList<Player *> alivePlayers();
    [[nodiscard]] QList<const Player *> alivePlayers() const;
    [[nodiscard]] QStringList alivePlayerNames() const;
    [[nodiscard]] int alivePlayersCount() const
    {
        return (int)(alivePlayers().size());
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
    const std::unique_ptr<p::RoomP> d;
    Q_DISABLE_COPY_MOVE(Room)
};
} // namespace v0

#ifndef DOXYGEN
inline namespace v1 {
using v0::LogicConfiguration;
using v0::Room;
} // namespace v1
#endif

} // namespace QMdmmCore

#endif // QMDMMROOM_H
