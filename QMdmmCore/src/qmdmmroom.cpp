// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmroom.h"
#include "qmdmmroom_p.h"

#include "qmdmmlogic.h"
#include "qmdmmplayer.h"

#include <QString>

#include <utility>

/**
 * @file qmdmmroom.h
 * @brief Contains definitions of room.
 */

/**
 * @class QMdmmLogicConfiguration
 * @brief Contains configurations of logic
 */

/**
 * @def DEFINE_CONFIGURATION(type, valueName, ValueName)
 * @brief helper macro to define functions
 */

/**
 * @property QMdmmLogicConfiguration::playerNumPerRoom
 * @brief Player number per room, default 3
 * @todo move this configuration to server side
 */

/**
 * @enum QMdmmLogicConfiguration::PunishHpRoundStrategy
 * @brief The round strategy of punishing HP
 *
 * Punish HP is taking place when a player slashes others in city. By default a half of maximum HP is lost. <br />
 * Since HP is integer value one can not have a ".5" stuff or something, so rounding of the calculated half is needed.
 * The modifier can be configured individually, so "a third" or "a quarter" etc. are also configurable
 *
 * Currently there is 4 different rounding strategy supported. Following table is sort by understandability.
 *
 * | Strategy name | Description | Examples |
 * |-|-|-|
 * | @c QMdmmLogicConfiguration::RoundDown | Round all number down, i.e. digits after the dot are stripped. | 1.1 -> 1, 1.4 -> 1, 1.5 -> 1, 1.9 -> 1, 2.0 -> 2 |
 * | @c QMdmmLogicConfiguration::PlusOne | Apply +1 after round all number down. | 1.1 -> 2, 1.4 -> 2, 1.5 -> 2, 1.9 -> 2, 2.0 -> 3 |
 * | @c QMdmmLogicConfiguration::RoundUp | Round all number up, i.e. plus one if there are digits after the dot, then digits after the dot are stripped. | 1.1 -> 2, 1.4 -> 2, 1.5 -> 2, 1.9 -> 2, 2.0 -> 2 |
 * | @c QMdmmLogicConfiguration::RoundToNearest45 | Round all number to nearest integer. Round down if the first digit after dot is <= 4, round up else. | 1.1 -> 1, 1.4 -> 1, 1.5 -> 2, 1.9 -> 2, 2.0 -> 2 |
 */

/**
 * @var QMdmmLogicConfiguration::PunishHpRoundStrategy QMdmmLogicConfiguration::RoundDown
 * Round all number down, i.e. digits after the dot are stripped. See table above.
 *
 * @var QMdmmLogicConfiguration::PunishHpRoundStrategy QMdmmLogicConfiguration::PlusOne
 * Apply +1 after round all number down. See table above.
 *
 * @var QMdmmLogicConfiguration::PunishHpRoundStrategy QMdmmLogicConfiguration::RoundUp
 * Round all number up, i.e. plus one if there are digits after the dot, then digits after the dot are stripped. See table above.
 *
 * @var QMdmmLogicConfiguration::PunishHpRoundStrategy QMdmmLogicConfiguration::RoundToNearest45
 * Round all number to nearest integer. Round down if the first digit after dot is <= 4, round up else. See table above.
 */

/**
 * @fn QMdmmLogicConfiguration::playerNumPerRoom() const
 * @brief getter of @c QMdmmLogicConfiguration::playerNumPerRoom
 * @return @c QMdmmLogicConfiguration::playerNumPerRoom
 */

/**
 * @fn QMdmmLogicConfiguration::setPlayerNumPerRoom(int playerNumPerRoom)
 * @brief setter of @c QMdmmLogicConfiguration::playerNumPerRoom
 * @param playerNumPerRoom @c QMdmmLogicConfiguration::playerNumPerRoom
 */

/**
 * @property QMdmmLogicConfiguration::requestTimeout
 * @brief Request timeout, default 20
 * @todo move this configuration to server side
 */

/**
 * @fn QMdmmLogicConfiguration::requestTimeout() const
 * @brief getter of @c QMdmmLogicConfiguration::requestTimeout
 * @return @c QMdmmLogicConfiguration::requestTimeout
 */

/**
 * @fn QMdmmLogicConfiguration::setRequestTimeout(int requestTimeout)
 * @brief setter of @c QMdmmLogicConfiguration::requestTimeout
 * @param requestTimeout @c QMdmmLogicConfiguration::requestTimeout
 */

/**
 * @property QMdmmLogicConfiguration::initialKnifeDamage
 * @brief the "knife damage" when game starts, default 1
 */

/**
 * @fn QMdmmLogicConfiguration::initialKnifeDamage() const
 * @brief getter of @c QMdmmLogicConfiguration::initialKnifeDamage
 * @return @c QMdmmLogicConfiguration::initialKnifeDamage
 */

/**
 * @fn QMdmmLogicConfiguration::setInitialKnifeDamage(int initialKnifeDamage)
 * @brief setter of @c QMdmmLogicConfiguration::initialKnifeDamage
 * @param initialKnifeDamage @c QMdmmLogicConfiguration::initialKnifeDamage
 */

/**
 * @property QMdmmLogicConfiguration::maximumKnifeDamage
 * @brief the maximum "knife damage" one can upgrade to, default 10
 */

/**
 * @fn QMdmmLogicConfiguration::maximumKnifeDamage() const
 * @brief getter of @c QMdmmLogicConfiguration::maximumKnifeDamage
 * @return @c QMdmmLogicConfiguration::maximumKnifeDamage
 */

/**
 * @fn QMdmmLogicConfiguration::setMaximumKnifeDamage(int maximumKnifeDamage)
 * @brief setter of @c QMdmmLogicConfiguration::maximumKnifeDamage
 * @param maximumKnifeDamage @c QMdmmLogicConfiguration::maximumKnifeDamage
 */

/**
 * @property QMdmmLogicConfiguration::initialHorseDamage
 * @brief the "horse damage" when game starts, default 2
 */

/**
 * @fn QMdmmLogicConfiguration::initialHorseDamage() const
 * @brief getter of @c QMdmmLogicConfiguration::initialHorseDamage
 * @return @c QMdmmLogicConfiguration::initialHorseDamage
 */

/**
 * @fn QMdmmLogicConfiguration::setInitialHorseDamage(int initialHorseDamage)
 * @brief setter of @c QMdmmLogicConfiguration::initialHorseDamage
 * @param initialHorseDamage @c QMdmmLogicConfiguration::initialHorseDamage
 */

/**
 * @property QMdmmLogicConfiguration::maximumHorseDamage
 * @brief the maximum "horse damage" one can upgrade to, default 10
 */

/**
 * @fn QMdmmLogicConfiguration::maximumHorseDamage() const
 * @brief getter of @c QMdmmLogicConfiguration::maximumHorseDamage
 * @return @c QMdmmLogicConfiguration::maximumHorseDamage
 */

/**
 * @fn QMdmmLogicConfiguration::setMaximumHorseDamage(int maximumHorseDamage)
 * @brief setter of @c QMdmmLogicConfiguration::maximumHorseDamage
 * @param maximumHorseDamage @c QMdmmLogicConfiguration::maximumHorseDamage
 */

/**
 * @property QMdmmLogicConfiguration::initialMaxHp
 * @brief the "maximum HP" when game starts, default 10
 */

/**
 * @fn QMdmmLogicConfiguration::initialMaxHp() const
 * @brief getter of @c QMdmmLogicConfiguration::initialMaxHp
 * @return @c QMdmmLogicConfiguration::initialMaxHp
 */

/**
 * @fn QMdmmLogicConfiguration::setInitialMaxHp(int initialMaxHp)
 * @brief setter of @c QMdmmLogicConfiguration::initialMaxHp
 * @param initialMaxHp @c QMdmmLogicConfiguration::initialMaxHp
 */

/**
 * @property QMdmmLogicConfiguration::maximumMaxHp
 * @brief the maximum "maximum HP" one can upgrade to, default 20
 * @note This seems hard to understand. HP is reset to "maximum HP" when round starts and this "maximum HP" is upgradable. Any upgradable data have a maximum value so there is maximum "maximum HP".
 */

/**
 * @fn QMdmmLogicConfiguration::maximumMaxHp() const
 * @brief getter of @c QMdmmLogicConfiguration::maximumMaxHp
 * @return @c QMdmmLogicConfiguration::maximumMaxHp
 */

/**
 * @fn QMdmmLogicConfiguration::setMaximumMaxHp(int maximumMaxHp)
 * @brief setter of @c QMdmmLogicConfiguration::maximumMaxHp
 * @param maximumMaxHp @c QMdmmLogicConfiguration::maximumMaxHp
 */

/**
 * @property QMdmmLogicConfiguration::punishHpModifier
 * @brief The modifier of punish HP. Set it to 0 to disable punish HP, default 2
 */

/**
 * @fn QMdmmLogicConfiguration::punishHpModifier() const
 * @brief getter of @c QMdmmLogicConfiguration::punishHpModifier
 * @return @c QMdmmLogicConfiguration::punishHpModifier
 */

/**
 * @fn QMdmmLogicConfiguration::setPunishHpModifier(int punishHpModifier)
 * @brief setter of @c QMdmmLogicConfiguration::punishHpModifier
 * @param punishHpModifier @c QMdmmLogicConfiguration::punishHpModifier
 */

/**
 * @property QMdmmLogicConfiguration::punishHpRoundStrategy
 * @brief Punish HP round strategy, use with @c QMdmmLogicConfiguration::punishHpModifier, default @c QMdmmLogicConfiguration::RoundToNearest45
 * @sa @c QMdmmLogicConfiguration::PunishHpRoundStrategy
 */

/**
 * @fn QMdmmLogicConfiguration::punishHpRoundStrategy() const
 * @brief getter of @c QMdmmLogicConfiguration::punishHpRoundStrategy
 * @return @c QMdmmLogicConfiguration::punishHpRoundStrategy
 */

/**
 * @fn QMdmmLogicConfiguration::setPunishHpRoundStrategy(QMdmmLogicConfiguration::PunishHpRoundStrategy punishHpRoundStrategy)
 * @brief setter of @c QMdmmLogicConfiguration::punishHpRoundStrategy
 * @param punishHpRoundStrategy @c QMdmmLogicConfiguration::punishHpRoundStrategy
 */

/**
 * @property QMdmmLogicConfiguration::zeroHpAsDead
 * @brief Treat one with 0 hp as dead. default true
 */

/**
 * @fn QMdmmLogicConfiguration::zeroHpAsDead() const
 * @brief getter of @c QMdmmLogicConfiguration::zeroHpAsDead
 * @return @c QMdmmLogicConfiguration::zeroHpAsDead
 */

/**
 * @fn QMdmmLogicConfiguration::setZeroHpAsDead(bool zeroHpAsDead)
 * @brief setter of @c QMdmmLogicConfiguration::zeroHpAsDead
 * @param zeroHpAsDead @c QMdmmLogicConfiguration::zeroHpAsDead
 */

/**
 * @property QMdmmLogicConfiguration::enableLetMove
 * @brief Enables "let move" game mechanism. default true
 */

/**
 * @fn QMdmmLogicConfiguration::enableLetMove() const
 * @brief getter of @c QMdmmLogicConfiguration::enableLetMove
 * @return @c QMdmmLogicConfiguration::enableLetMove
 */

/**
 * @fn QMdmmLogicConfiguration::setEnableLetMove(bool enableLetMove)
 * @brief setter of @c QMdmmLogicConfiguration::enableLetMove
 * @param enableLetMove @c QMdmmLogicConfiguration::enableLetMove
 */

/**
 * @property QMdmmLogicConfiguration::canBuyOnlyInInitialCity
 * @brief Can buy knife / horse only in initial city. default false
 * @note even if this is false, one cannot buy things in country.
 */

/**
 * @fn QMdmmLogicConfiguration::canBuyOnlyInInitialCity() const
 * @brief getter of @c QMdmmLogicConfiguration::canBuyOnlyInInitialCity
 * @return @c QMdmmLogicConfiguration::canBuyOnlyInInitialCity
 */

/**
 * @fn QMdmmLogicConfiguration::setCanBuyOnlyInInitialCity(bool canBuyOnlyInInitialCity)
 * @brief setter of @c QMdmmLogicConfiguration::canBuyOnlyInInitialCity
 * @param canBuyOnlyInInitialCity @c QMdmmLogicConfiguration::canBuyOnlyInInitialCity
 */

/**
 * @brief Get default values of configuration
 * @return default configuration
 *
 * I had experienced 2 versions of MDMM, they are mostly same but with minor differences. <br />
 * In both versions one can only buy k/h in city. <br />
 * In both versions one can slash only if knife is bought and kick only if horse is bought. <br />
 * In both versions one can only slash / kick other one when they are at same place. <br />
 * In both versions one can kick other one in city, and the kicked one will be force moved to country. <br />
 * In both versions one can only move to adjacent place at a time. Different cities are not adjacent, while country is adjacent to every city. <br />
 *
 * Version 1 (Legacy): initial 7/1/3 mh/kd/hd, maximum 7/3/5 mh/kd/hd (Yeah, no maxHp upgrade). One with zero HP is still alive. No "Let move"s, no punish HP. <br />
 * This is the version I had experienced in primary school. <br />
 * This version knife upgrades are more valuable. Horse is of less value since there is no HP punish.
 *
 * Version 2: initial 10/1/2 mh/kd/hd, maximum 20/10/10 mh/kd/hd. One with zero HP dies. With "Let move"s (pull sb. in / push sb. out of city stuff), with punish HP. <br />
 * This is the version I had experienced in junior high school. <br />
 * This version horse can be effectively used in multi-player. pull - kick loop is fun!
 *
 * Default configurations matches rules of Version 2 and can be tweaked.
 */
const QMdmmLogicConfiguration &QMdmmLogicConfiguration::defaults()
{
    // clang-format off
    static const QMdmmLogicConfiguration defaultInstance {
        qMakePair(QStringLiteral("playerNumPerRoom"), 3),
        qMakePair(QStringLiteral("requestTimeout"), 20),
        qMakePair(QStringLiteral("initialKnifeDamage"), 1),
        qMakePair(QStringLiteral("maximumKnifeDamage"), 10),
        qMakePair(QStringLiteral("initialHorseDamage"), 2),
        qMakePair(QStringLiteral("maximumHorseDamage"), 10),
        qMakePair(QStringLiteral("initialMaxHp"), 10),
        qMakePair(QStringLiteral("maximumMaxHp"), 20),
        qMakePair(QStringLiteral("punishHpModifier"), 2),
        qMakePair(QStringLiteral("punishHpRoundStrategy"), static_cast<int>(RoundToNearest45)),
        qMakePair(QStringLiteral("zeroHpAsDead"), true),
        qMakePair(QStringLiteral("enableLetMove"), true),
        qMakePair(QStringLiteral("canBuyOnlyInInitialCity"), false),
    };
    // clang-format on

    return defaultInstance;
}

/**
 * @brief Get Version 1 configuration
 * @return Version 1 configuration
 *
 * @sa @c QMdmmLogicConfiguration::defaults()
 */
const QMdmmLogicConfiguration &QMdmmLogicConfiguration::v1()
{
    // clang-format off
    static const QMdmmLogicConfiguration defaultInstance {
        qMakePair(QStringLiteral("playerNumPerRoom"), 3),
        qMakePair(QStringLiteral("requestTimeout"), 20),
        qMakePair(QStringLiteral("initialKnifeDamage"), 1),
        qMakePair(QStringLiteral("maximumKnifeDamage"), 3),
        qMakePair(QStringLiteral("initialHorseDamage"), 3),
        qMakePair(QStringLiteral("maximumHorseDamage"), 5),
        qMakePair(QStringLiteral("initialMaxHp"), 7),
        qMakePair(QStringLiteral("maximumMaxHp"), 7),
        qMakePair(QStringLiteral("punishHpModifier"), 0),
        qMakePair(QStringLiteral("punishHpRoundStrategy"), static_cast<int>(RoundToNearest45)),
        qMakePair(QStringLiteral("zeroHpAsDead"), false),
        qMakePair(QStringLiteral("enableLetMove"), false),
        qMakePair(QStringLiteral("canBuyOnlyInInitialCity"), false),
    };
    // clang-format on

    return defaultInstance;
}

// NOLINTBEGIN(bugprone-macro-parentheses)

#define CONVERTTOTYPEBOOL(v) v.toBool()
#define CONVERTTOTYPEINT(v) v.toInt()
#define CONVERTTOTYPEPUNISHHPROUNDSTRATEGY(v) static_cast<QMdmmLogicConfiguration::PunishHpRoundStrategy>(v.toInt())
#define IMPLEMENTATION_CONFIGURATION(type, valueName, ValueName, convertToType, convertToJsonValue) \
    type QMdmmLogicConfiguration::valueName() const                                                 \
    {                                                                                               \
        if (contains(QStringLiteral(#valueName)))                                                   \
            return convertToType(value(QStringLiteral(#valueName)));                                \
        return convertToType(defaults().value(QStringLiteral(#valueName)));                         \
    }                                                                                               \
    void QMdmmLogicConfiguration::set##ValueName(type value)                                        \
    {                                                                                               \
        insert(QStringLiteral(#valueName), convertToJsonValue(value));                              \
    }

// NOLINTEND(bugprone-macro-parentheses)

IMPLEMENTATION_CONFIGURATION(int, playerNumPerRoom, PlayerNumPerRoom, CONVERTTOTYPEINT, )
IMPLEMENTATION_CONFIGURATION(int, requestTimeout, RequestTimeout, CONVERTTOTYPEINT, )
IMPLEMENTATION_CONFIGURATION(int, initialKnifeDamage, InitialKnifeDamage, CONVERTTOTYPEINT, )
IMPLEMENTATION_CONFIGURATION(int, maximumKnifeDamage, MaximumKnifeDamage, CONVERTTOTYPEINT, )
IMPLEMENTATION_CONFIGURATION(int, initialHorseDamage, InitialHorseDamage, CONVERTTOTYPEINT, )
IMPLEMENTATION_CONFIGURATION(int, maximumHorseDamage, MaximumHorseDamage, CONVERTTOTYPEINT, )
IMPLEMENTATION_CONFIGURATION(int, initialMaxHp, InitialMaxHp, CONVERTTOTYPEINT, )
IMPLEMENTATION_CONFIGURATION(int, maximumMaxHp, MaximumMaxHp, CONVERTTOTYPEINT, )
IMPLEMENTATION_CONFIGURATION(int, punishHpModifier, PunishHpModifier, CONVERTTOTYPEINT, )
IMPLEMENTATION_CONFIGURATION(QMdmmLogicConfiguration::PunishHpRoundStrategy, punishHpRoundStrategy, PunishHpRoundStrategy, CONVERTTOTYPEPUNISHHPROUNDSTRATEGY, static_cast<int>)
IMPLEMENTATION_CONFIGURATION(bool, zeroHpAsDead, ZeroHpAsDead, CONVERTTOTYPEBOOL, )
IMPLEMENTATION_CONFIGURATION(bool, enableLetMove, EnableLetMove, CONVERTTOTYPEBOOL, )
IMPLEMENTATION_CONFIGURATION(bool, canBuyOnlyInInitialCity, CanBuyOnlyInInitialCity, CONVERTTOTYPEBOOL, )

#undef IMPLEMENTATION_CONFIGURATION
#undef CONVERTTOTYPEPUNISHHPROUNDSTRATEGY
#undef CONVERTTOTYPEINT
#undef CONVERTTOTYPEBOOL

/**
 * @brief deserialize @c QJsonValue to @c QMdmmLogicConfiguration
 * @param value the value to be deserialized
 * @return if the deserialize succeeded.
 * @note It is possible to convert the value to @c QJsonObject and directly assign the value, since this class inherits @c QJsonObject, but the value check in this function will be nonexistent then.
 */
bool QMdmmLogicConfiguration::deserialize(const QJsonValue &value) // NOLINT(readability-function-cognitive-complexity)
{
    if (!value.isObject())
        return false;

    QJsonObject ob = value.toObject();
    QJsonObject result;

#define CONF(member, check)                                   \
    {                                                         \
        if (ob.contains(QStringLiteral(#member))) {           \
            QJsonValue v = ob.value(QStringLiteral(#member)); \
            if (v.check())                                    \
                result.insert(QStringLiteral(#member), v);    \
            else                                              \
                return false;                                 \
        } else {                                              \
            return false;                                     \
        }                                                     \
    }

    CONF(playerNumPerRoom, isDouble);
    CONF(requestTimeout, isDouble);
    CONF(initialKnifeDamage, isDouble);
    CONF(maximumKnifeDamage, isDouble);
    CONF(initialHorseDamage, isDouble);
    CONF(maximumHorseDamage, isDouble);
    CONF(initialMaxHp, isDouble);
    CONF(maximumMaxHp, isDouble);
    CONF(punishHpModifier, isDouble);
    CONF(punishHpRoundStrategy, isDouble);
    CONF(zeroHpAsDead, isBool);
    CONF(enableLetMove, isBool);
    CONF(canBuyOnlyInInitialCity, isBool);

#undef CONF

    *this = result;
    return true;
}

/**
 * @class QMdmmRoom
 * @brief The room which MDMM game is played in.
 *
 * It maintains all the players as well as global data.
 */

/**
 * @brief ctor.
 * @param logicConfiguration The configuration of the logic
 * @param parent The parent (QObject parent hierarchy)
 */
QMdmmRoom::QMdmmRoom(QMdmmLogicConfiguration logicConfiguration, QObject *parent)
    : QObject(parent)
    , d(std::make_unique<QMdmmRoomPrivate>())
{
    d->logicConfiguration = std::move(logicConfiguration);
}

/**
 * @brief dtor.
 */
QMdmmRoom::~QMdmmRoom() = default;

/**
 * @brief Get the configuration of current logic
 * @return current logic configuration
 */
const QMdmmLogicConfiguration &QMdmmRoom::logicConfiguration() const
{
    return d->logicConfiguration;
}

/**
 * @brief Set the configuration of current logic
 * @param logicConfiguration updated logic configuration
 */
void QMdmmRoom::setLogicConfiguration(const QMdmmLogicConfiguration &logicConfiguration)
{
    d->logicConfiguration = logicConfiguration;
}

/**
 * @brief Add a player to game
 * @param playerName the internal name of the newly added player
 * @return the newly added player
 */
QMdmmPlayer *QMdmmRoom::addPlayer(const QString &playerName)
{
    if (d->players.contains(playerName))
        return nullptr;

    QMdmmPlayer *ret = new QMdmmPlayer(playerName, this);
    d->players.insert(playerName, ret);

    emit playerAdded(playerName, QPrivateSignal());

    return ret;
}

/**
 * @brief Remove a player from game
 * @param playerName the internal name of the removed player
 * @return if the player is successfully removed
 */
bool QMdmmRoom::removePlayer(const QString &playerName)
{
    if (QMap<QString, QMdmmPlayer *>::iterator it = d->players.find(playerName); it != d->players.end()) {
        emit playerRemoved(playerName, QPrivateSignal());

        delete it.value();
        d->players.erase(it);
        return true;
    }

    return false;
}

/**
 * @brief get the player of specific internal name
 * @param playerName the internal name of the searched player
 * @return the player of the internal name, or @c nullptr if not found
 */
QMdmmPlayer *QMdmmRoom::player(const QString &playerName)
{
    return d->players.value(playerName, nullptr);
}

/**
 * @brief get the player of specific internal name (const version)
 * @param playerName the internal name of the searched player
 * @return the player of the internal name, or @c nullptr if not found
 */
const QMdmmPlayer *QMdmmRoom::player(const QString &playerName) const
{
    return d->players.value(playerName, nullptr);
}

/**
 * @brief get a list of players
 * @return the list of players
 */
QList<QMdmmPlayer *> QMdmmRoom::players()
{
    return d->players.values();
}

/**
 * @brief get a list of players (const version)
 * @return the list of players
 */
QList<const QMdmmPlayer *> QMdmmRoom::players() const
{
    QList<const QMdmmPlayer *> res;
    foreach (const QMdmmPlayer *player, d->players)
        res << player;

    return res;
}

/**
 * @brief get a list of player interna names
 * @return the list of player internal names
 */
QStringList QMdmmRoom::playerNames() const
{
    return d->players.keys();
}

/**
 * @brief get a list of alive players
 * @return the list of alive players
 */
QList<QMdmmPlayer *> QMdmmRoom::alivePlayers()
{
    QList<QMdmmPlayer *> res;
    foreach (QMdmmPlayer *player, d->players) {
        if (player->alive())
            res << player;
    }

    return res;
}

/**
 * @brief get a list of alive players (const version)
 * @return the list of alive players
 */
QList<const QMdmmPlayer *> QMdmmRoom::alivePlayers() const
{
    QList<const QMdmmPlayer *> res;
    foreach (const QMdmmPlayer *player, d->players) {
        if (player->alive())
            res << player;
    }

    return res;
}

/**
 * @brief get a list of internal names of alive players
 * @return the list of names of alive players
 */
QStringList QMdmmRoom::alivePlayerNames() const
{
    QStringList res;
    for (QMap<QString, QMdmmPlayer *>::const_iterator it = d->players.constBegin(); it != d->players.constEnd(); ++it) {
        if (it.value()->alive())
            res.push_back(it.key());
    }

    return res;
}

/**
 * @fn QMdmmRoom::alivePlayersCount() const
 * @brief get the count of alive players
 * @return count of alive players
 *
 * equals to @verbatim alivePlayers().size() @endverbatim
 */

/**
 * @fn QMdmmRoom::isRoundOver() const
 * @brief judge if current game is round over
 * @return if current game is round over
 *
 * equals to @verbatim alivePlayersCount() <= 1 @endverbatim
 */

/**
 * @brief judge if current game is game over
 * @param winnerPlayerNames (out) winner player names
 * @return if current game is game over
 */
bool QMdmmRoom::isGameOver(QStringList *winnerPlayerNames) const
{
    bool ret = false;
    if (winnerPlayerNames != nullptr)
        winnerPlayerNames->clear();

    foreach (const QMdmmPlayer *player, d->players) {
        if (!player->canUpgradeHorse() && !player->canUpgradeKnife() && !player->canUpgradeMaxHp()) {
            ret = true;
            if (winnerPlayerNames != nullptr)
                *winnerPlayerNames << player->objectName();
        }
    }

    return ret;
}

/**
 * @brief prepare every players for round start
 */
void QMdmmRoom::prepareForRoundStart()
{
    int i = 0;
    foreach (QMdmmPlayer *player, d->players)
        player->prepareForRoundStart(++i);
}

/**
 * @brief reset upgrades of each player, for game start
 */
void QMdmmRoom::resetUpgrades()
{
    foreach (QMdmmPlayer *player, d->players)
        player->resetUpgrades();
}

/**
 * @fn QMdmmRoom::playerAdded(const QString &playerName, QPrivateSignal)
 * @brief emitted when a player is added
 * @param playerName the internal name of the added player
 */

/**
 * @fn QMdmmRoom::playerRemoved(const QString &playerName, QPrivateSignal);
 * @brief emitted when a player is removed
 * @param playerName the internal name of the removed player
 */
