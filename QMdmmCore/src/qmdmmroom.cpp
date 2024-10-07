// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmroom.h"
#include "qmdmmroom_p.h"

#include "qmdmmlogic.h"
#include "qmdmmplayer.h"

#include <QString>

#include <utility>

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

bool QMdmmLogicConfiguration::deserialize(const QJsonValue &value) // NOLINT(readability-function-cognitive-complexity)
{
    if (!value.isObject())
        return false;

    QJsonObject ob = value.toObject();
    QJsonObject result;

#define CONVERTPUNISHHPROUNDSTRATEGY() static_cast<PunishHpRoundStrategy>(v.toInt())
#define CONF(member, check)                                                                    \
    {                                                                                          \
        if (ob.contains(QStringLiteral(#member))) {                                            \
            QJsonValue v = ob.value(QStringLiteral(#member));                                  \
            if (v.check())                                                                     \
                result.insert(QStringLiteral(#member), v);                                     \
            else                                                                               \
                return false;                                                                  \
        } else {                                                                               \
            result.insert(QStringLiteral(#member), defaults().value(QStringLiteral(#member))); \
        }                                                                                      \
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

#undef CONF
#undef CONVERTPUNISHHPROUNDSTRATEGY

    *this = result;
    return true;
}

QMdmmRoom::QMdmmRoom(QMdmmLogicConfiguration logicConfiguration, QObject *parent)
    : QObject(parent)
    , d(new QMdmmRoomPrivate)
{
    d->logicConfiguration = std::move(logicConfiguration);
}

QMdmmRoom::~QMdmmRoom()
{
    delete d;
}

const QMdmmLogicConfiguration &QMdmmRoom::logicConfiguration() const
{
    return d->logicConfiguration;
}

void QMdmmRoom::setLogicConfiguration(const QMdmmLogicConfiguration &logicConfiguration)
{
    d->logicConfiguration = logicConfiguration;
}

QMdmmPlayer *QMdmmRoom::addPlayer(const QString &playerName)
{
    if (d->players.contains(playerName))
        return nullptr;

    QMdmmPlayer *ret = new QMdmmPlayer(playerName, this);
    d->players.insert(playerName, ret);

    emit playerAdded(playerName, QPrivateSignal());

    return ret;
}

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

QMdmmPlayer *QMdmmRoom::player(const QString &playerName)
{
    return d->players.value(playerName, nullptr);
}

const QMdmmPlayer *QMdmmRoom::player(const QString &playerName) const
{
    return d->players.value(playerName, nullptr);
}

QList<QMdmmPlayer *> QMdmmRoom::players()
{
    return d->players.values();
}

QList<const QMdmmPlayer *> QMdmmRoom::players() const
{
    QList<const QMdmmPlayer *> res;
    foreach (const QMdmmPlayer *player, d->players)
        res << player;

    return res;
}

QStringList QMdmmRoom::playerNames() const
{
    return d->players.keys();
}

QList<QMdmmPlayer *> QMdmmRoom::alivePlayers()
{
    QList<QMdmmPlayer *> res;
    foreach (QMdmmPlayer *player, d->players) {
        if (player->alive())
            res << player;
    }

    return res;
}

QList<const QMdmmPlayer *> QMdmmRoom::alivePlayers() const
{
    QList<const QMdmmPlayer *> res;
    foreach (const QMdmmPlayer *player, d->players) {
        if (player->alive())
            res << player;
    }

    return res;
}

QStringList QMdmmRoom::alivePlayerNames() const
{
    QStringList res;
    for (QMap<QString, QMdmmPlayer *>::const_iterator it = d->players.constBegin(); it != d->players.constEnd(); ++it) {
        if (it.value()->alive())
            res.push_back(it.key());
    }

    return res;
}

int QMdmmRoom::alivePlayersCount() const
{
    return alivePlayers().size();
}

bool QMdmmRoom::isRoundOver() const
{
    return alivePlayersCount() <= 1;
}

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

void QMdmmRoom::prepareForRoundStart()
{
    int i = 0;
    foreach (QMdmmPlayer *player, d->players)
        player->prepareForRoundStart(++i);
}

void QMdmmRoom::resetUpgrades()
{
    foreach (QMdmmPlayer *player, d->players)
        player->resetUpgrades();
}
