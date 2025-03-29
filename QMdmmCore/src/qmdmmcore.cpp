// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmcoreglobal.h"

#include <QMap>

/**
 * @file qmdmmcoreglobal.h
 * @brief Global definition of QMdmmCore library
 */

/**
 * @namespace QMdmmCore
 * @brief All APIs are in this namespace.
 */

namespace QMdmmCore {

/**
 * @namespace v0
 * @brief Marks the version 0 API
 *
 * Initial set of API.
 */

/**
 * @namespace v1
 * @brief Marks the version 1 API
 *
 * API which will exist in 1.x.x version.
 */

#ifndef DOXYGEN
namespace v0 {
#else
inline namespace v1 {
#endif

/**
 * @def QMDMMCORE_EXPORT
 * @brief Indicates this function is public and is exported from QMdmmCore library.
 */

/**
 * @def QMDMMCORE_PRIVATE_EXPORT
 * @brief Indicates this function is private but will be exported from QMdmmCore library if specified during build.
 */

/**
 * @def QMDMM_EXPORT_NAME
 * @brief Specify a file name for automatic header generation. Expand to nothing.
 * @param QMdmmCoreGlobal dummy paramater.
 */

/**
 * @namespace Data
 * @headerfile <QMdmmData>
 * @brief Various data definition of MDMM game
 */

/**
 * @enum Data::Place
 * @brief Enumeration values for places
 *
 * This is a dummy placeholder enum since all place related variable now returns Integer values.
 * Preserved for QMetaObject generation.
 */

/**
 * @var Data::Place Data::Country
 * @brief For use with @c QMdmmPlayer::place() , if it equals to @c Data::Country then this player is in Country.
 *
 * This enumeration variable equals to zero. Provided for readability.
 */

/**
 * @enum Data::DamageReason
 * @brief The reason for a damage.
 */

/**
 * @var Data::DamageReason Data::DamageReasonUnknown
 * @brief Unknown / erroneous damage reason.
 */

/**
 * @var Data::DamageReason Data::Slashed
 * @brief Damage is caused by a slash.
 */

/**
 * @var Data::DamageReason Data::Kicked
 * @brief Damage is caused by a kick.
 */

/**
 * @var Data::DamageReason Data::HpPunished
 * @brief Damage is caused by HP punish.
 *
 * There is a mechanism in more modern version of MDMM game, where punishment is applied for slash in city.
 * By default the punished HP is half of the maximum HP, rounded to nearest integer
 *
 * @sa @c QMdmmLogicConfiguration::PunishHpRoundStrategy
 */

/**
 * @enum Data::StoneScissorsCloth
 * @brief Stone-Scissors-Cloth variables.
 */

/**
 * @var Data::StoneScissorsCloth Data::Stone
 * @brief Stone
 */

/**
 * @var Data::StoneScissorsCloth Data::Scissors
 * @brief Scissors
 */

/**
 * @var Data::StoneScissorsCloth Data::Cloth
 * @brief Cloth
 */

/**
 * @enum Data::Action
 * @brief Action taken each time a player is acting.
 */

/**
 * @var Data::Action Data::DoNothing
 * @brief Do Nothing
 */

/**
 * @var Data::Action Data::BuyKnife
 * @brief Buy Knife
 */

/**
 * @var Data::Action Data::BuyHorse
 * @brief Buy Horse
 */

/**
 * @var Data::Action Data::Slash
 * @brief Slash (toPlayer: the target player)
 */

/**
 * @var Data::Action Data::Kick
 * @brief Kick (toPlayer: the target player)
 */

/**
 * @var Data::Action Data::Move
 * @brief Move (toPlace: the target place)
 */

/**
 * @var Data::Action Data::LetMove
 * @brief Let Move (toPlayer: the target player, toPlace: the target place)
 */

/**
 * @enum Data::UpgradeItem
 * @brief Upgradeable items when a player wins a game.
 */

/**
 * @var Data::UpgradeItem Data::UpgradeKnife
 * @brief Upgrade knife
 */

/**
 * @var Data::UpgradeItem Data::UpgradeHorse
 * @brief Upgrade horse
 */

/**
 * @var Data::UpgradeItem Data::UpgradeMaxHp
 * @brief Upgrade maximum hp
 */

/**
 * @enum Data::AgentStateEnum
 * @brief State used for Agents.
 */

/**
 * @var Data::AgentStateEnum Data::StateMaskOnline
 * @brief Mask of online
 */

/**
 * @var Data::AgentStateEnum Data::StateMaskBot
 * @brief Mask of bot
 */

/**
 * @var Data::AgentStateEnum Data::StateMaskTrust
 * @brief Mask of trust
 */

/**
 * @var Data::AgentStateEnum Data::StateOffline
 * @brief State of offline
 */

/**
 * @var Data::AgentStateEnum Data::StateOfflineBot
 * @brief State of offline bot
 */

/**
 * @var Data::AgentStateEnum Data::StateOnline
 * @brief State of online
 */

/**
 * @var Data::AgentStateEnum Data::StateOnlineBot
 * @brief State of online bot
 */

/**
 * @var Data::AgentStateEnum Data::StateOnlineTrust
 * @brief State of online trusted
 */

/**
 * @fn Data::isPlaceAdjacent(int p1, int p2)
 * @brief Judges if the 2 places are adjacent, for judgement like make-move ability or other things.
 * @param p1 Place 1
 * @param p2 Place 2
 * @return If the 2 places are adjacent.
 *
 * It is actually simplified to "only one of p1 and p2 is Country"
 */

namespace {
constexpr bool sscGreater(Data::StoneScissorsCloth op1, Data::StoneScissorsCloth op2) noexcept
{
    return (op1 == Data::Stone && op2 == Data::Scissors) || (op1 == Data::Scissors && op2 == Data::Cloth) || (op1 == Data::Cloth && op2 == Data::Stone);
}
} // namespace

/**
 * @brief Judges winners of a Stone-Scissors-Cloth output
 * @param judgers A kv-pair of the value a player outputs
 * @return A list of winners, with each value duplicates multiple times. The duplication times is equal to the number of players loses.
 *
 * This is the core logic of which Stone-Scissors-Cloth.
 * Our rule is that winners can do actions on determined sequence by times that equals to the number of players loses.
 */
QStringList Data::stoneScissorsClothWinners(const QHash<QString, Data::StoneScissorsCloth> &judgers)
{
    QMap<Data::StoneScissorsCloth, QStringList> judgersMap;

    for (QHash<QString, Data::StoneScissorsCloth>::const_iterator it = judgers.constBegin(); it != judgers.constEnd(); ++it)
        judgersMap[it.value()] << it.key();

    if (judgersMap.count() == 2) {
        QMap<Data::StoneScissorsCloth, QStringList>::const_iterator it1 = judgersMap.constBegin();
        QMap<Data::StoneScissorsCloth, QStringList>::const_iterator it2 = judgersMap.constBegin();
        ++it2;

        Data::StoneScissorsCloth type1 = it1.key();
        Data::StoneScissorsCloth type2 = it2.key();

        if (!sscGreater(type1, type2))
            std::swap(it1, it2);

        // now it1.value is winner, it2.value is loser
        // we'd make every winners repeat N times (N is loser.count), for the real judgement use
        QStringList d;
        for (int i = 0; i < it2.value().length(); ++i)
            d.append(it1.value());

        return d;
    }

    return {};
}

/**
 * @namespace Global
 * @headerfile <QMdmmGlobal>
 * @brief Global functions of QMdmm
 */

/**
 * @brief Returns the version number QMdmmCore is built with
 * @return the version number
 */
QVersionNumber Global::version()
{
    return QVersionNumber::fromString(QStringLiteral(QMDMM_VERSION));
}

/**
 * @namespace Utilities
 * @headerfile <QMdmmUtilities>
 * @brief Convenience functions and types for working with QMdmm library
 */

/**
 * @fn Utilities::list2Set(const QList<T> &l)
 * @brief Convenience function of converting a QList to QSet
 *
 * Qt deprecates QList::toSet since Qt 5.15 and instead suggests using the QSet iterator ctor. This function does call the ctor.
 */

/**
 * @fn Utilities::enumList2VariantList(const QList<T> &list)
 * @brief Convenience function of converting QList<enum> to QVariantList
 */

/**
 * @fn Utilities::enumList2VariantList(const QList<QFlags<T> > &list)
 * @brief Convenience function of converting QList<QFlags> to QVariantList
 */

/**
 * @brief Convenience function of converting QList<int> to QVariantList
 */
QVariantList Utilities::intList2VariantList(const QList<int> &list)
{
    QVariantList ret;
    ret.reserve(list.length());
    foreach (int i, list)
        ret << i;
    return ret;
}

/**
 * @brief Convenience function of converting QVariantList to QList<int>
 */
QList<int> Utilities::variantList2IntList(const QVariantList &list)
{
    QList<int> ret;
    ret.reserve(list.length());
    foreach (const QVariant &i, list)
        ret << i.toInt();
    return ret;
}

/**
 * @brief Convenience function of converting QStringList to QVariantList
 * @note Qt 5 QStringList is not QList<QString> but Qt 6 is. Use parameter type QList<QString> for compatible with Qt 5
 */
QVariantList Utilities::stringList2VariantList(const QList<QString> &list)
{
    QVariantList ret;
    ret.reserve(list.length());
    foreach (const QString &i, list)
        ret << i;
    return ret;
}

/**
 * @brief Convenience function of converting QList<int> to QStringList
 */
QStringList Utilities::variantList2StringList(const QVariantList &list)
{
    QStringList ret;
    ret.reserve(list.length());
    foreach (const QVariant &i, list)
        ret << i.toString();
    return ret;
}
} // namespace v0
} // namespace QMdmmCore
