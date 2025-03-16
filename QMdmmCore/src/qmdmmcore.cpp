// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmcoreglobal.h"

#include <QMap>

/**
 * @file qmdmmcoreglobal.h
 * @brief Global definition of QMdmmCore library
 */

namespace QMdmmCore {
namespace v0 {

/**
 * @def QMDMMCORE_EXPORT
 * @brief Indicates this function is public and is exported from QMdmmCore library.
 */

/**
 * @def QMDMMCORE_PRIVATE_EXPORT
 * @brief Indicates this function is private but will be exported from QMdmmCore library if specified during build.
 */

/**
 * @def QMDMMCORE_EXPORT_NO_GENERATE_HEADER
 * @brief Indicates this function is public and is exported from QMdmmCore library, but no automatic header file generation.
 */

/**
 * @namespace QMdmmData
 * @headerfile <QMdmmData>
 * @brief Various data definition of MDMM game
 */

/**
 * @enum QMdmmData::Place
 * @brief Enumeration values for places
 *
 * This is a dummy placeholder enum since all place related variable now returns Integer values.
 * Preserved for QMetaObject generation.
 */

/**
 * @var QMdmmData::Place QMdmmData::Country
 * @brief For use with @c QMdmmPlayer::place() , if it equals to @c QMdmmData::Country then this player is in Country.
 *
 * This enumeration variable equals to zero. Provided for readability.
 */

/**
 * @enum QMdmmData::DamageReason
 * @brief The reason for a damage.
 */

/**
 * @var QMdmmData::DamageReason QMdmmData::DamageReasonUnknown
 * @brief Unknown / erroneous damage reason.
 */

/**
 * @var QMdmmData::DamageReason QMdmmData::Slashed
 * @brief Damage is caused by a slash.
 */

/**
 * @var QMdmmData::DamageReason QMdmmData::Kicked
 * @brief Damage is caused by a kick.
 */

/**
 * @var QMdmmData::DamageReason QMdmmData::HpPunished
 * @brief Damage is caused by HP punish.
 *
 * There is a mechanism in more modern version of MDMM game, where punishment is applied for slash in city.
 * By default the punished HP is half of the maximum HP, rounded to nearest integer
 *
 * @sa @c QMdmmLogicConfiguration::PunishHpRoundStrategy
 */

/**
 * @enum QMdmmData::StoneScissorsCloth
 * @brief Stone-Scissors-Cloth variables.
 */

/**
 * @var QMdmmData::StoneScissorsCloth QMdmmData::Stone
 * @brief Stone
 */

/**
 * @var QMdmmData::StoneScissorsCloth QMdmmData::Scissors
 * @brief Scissors
 */

/**
 * @var QMdmmData::StoneScissorsCloth QMdmmData::Cloth
 * @brief Cloth
 */

/**
 * @enum QMdmmData::Action
 * @brief Action taken each time a player is acting.
 */

/**
 * @var QMdmmData::Action QMdmmData::DoNothing
 * @brief Do Nothing
 */

/**
 * @var QMdmmData::Action QMdmmData::BuyKnife
 * @brief Buy Knife
 */

/**
 * @var QMdmmData::Action QMdmmData::BuyHorse
 * @brief Buy Horse
 */

/**
 * @var QMdmmData::Action QMdmmData::Slash
 * @brief Slash (toPlayer: the target player)
 */

/**
 * @var QMdmmData::Action QMdmmData::Kick
 * @brief Kick (toPlayer: the target player)
 */

/**
 * @var QMdmmData::Action QMdmmData::Move
 * @brief Move (toPlace: the target place)
 */

/**
 * @var QMdmmData::Action QMdmmData::LetMove
 * @brief Let Move (toPlayer: the target player, toPlace: the target place)
 */

/**
 * @enum QMdmmData::UpgradeItem
 * @brief Upgradeable items when a player wins a game.
 */

/**
 * @var QMdmmData::UpgradeItem QMdmmData::UpgradeKnife
 * @brief Upgrade knife
 */

/**
 * @var QMdmmData::UpgradeItem QMdmmData::UpgradeHorse
 * @brief Upgrade horse
 */

/**
 * @var QMdmmData::UpgradeItem QMdmmData::UpgradeMaxHp
 * @brief Upgrade maximum hp
 */

/**
 * @enum QMdmmData::AgentStateEnum
 * @brief State used for Agents.
 */

/**
 * @var QMdmmData::AgentStateEnum QMdmmData::StateMaskOnline
 * @brief Mask of online
 */

/**
 * @var QMdmmData::AgentStateEnum QMdmmData::StateMaskBot
 * @brief Mask of bot
 */

/**
 * @var QMdmmData::AgentStateEnum QMdmmData::StateMaskTrust
 * @brief Mask of trust
 */

/**
 * @var QMdmmData::AgentStateEnum QMdmmData::StateOffline
 * @brief State of offline
 */

/**
 * @var QMdmmData::AgentStateEnum QMdmmData::StateOfflineBot
 * @brief State of offline bot
 */

/**
 * @var QMdmmData::AgentStateEnum QMdmmData::StateOnline
 * @brief State of online
 */

/**
 * @var QMdmmData::AgentStateEnum QMdmmData::StateOnlineBot
 * @brief State of online bot
 */

/**
 * @var QMdmmData::AgentStateEnum QMdmmData::StateOnlineTrust
 * @brief State of online trusted
 */

/**
 * @fn QMdmmData::isPlaceAdjacent(int p1, int p2)
 * @brief Judges if the 2 places are adjacent, for judgement like make-move ability or other things.
 * @param p1 Place 1
 * @param p2 Place 2
 * @return If the 2 places are adjacent.
 *
 * It is actually simplified to "only one of p1 and p2 is Country"
 */

namespace {
constexpr bool sscGreater(QMdmmData::StoneScissorsCloth op1, QMdmmData::StoneScissorsCloth op2) noexcept
{
    return (op1 == QMdmmData::Stone && op2 == QMdmmData::Scissors) || (op1 == QMdmmData::Scissors && op2 == QMdmmData::Cloth)
        || (op1 == QMdmmData::Cloth && op2 == QMdmmData::Stone);
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
QStringList QMdmmData::stoneScissorsClothWinners(const QHash<QString, QMdmmData::StoneScissorsCloth> &judgers)
{
    QMap<QMdmmData::StoneScissorsCloth, QStringList> judgersMap;

    for (QHash<QString, QMdmmData::StoneScissorsCloth>::const_iterator it = judgers.constBegin(); it != judgers.constEnd(); ++it)
        judgersMap[it.value()] << it.key();

    if (judgersMap.count() == 2) {
        QMap<QMdmmData::StoneScissorsCloth, QStringList>::const_iterator it1 = judgersMap.constBegin();
        QMap<QMdmmData::StoneScissorsCloth, QStringList>::const_iterator it2 = judgersMap.constBegin();
        ++it2;

        QMdmmData::StoneScissorsCloth type1 = it1.key();
        QMdmmData::StoneScissorsCloth type2 = it2.key();

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
 * @namespace QMdmmGlobal
 * @headerfile <QMdmmGlobal>
 * @brief Global functions of QMdmm
 */

/**
 * @brief Returns the version number QMdmmCore is built with
 * @return the version number
 */
QVersionNumber QMdmmGlobal::version()
{
    return QVersionNumber::fromString(QStringLiteral(QMDMM_VERSION));
}

/**
 * @namespace QMdmmUtilities
 * @headerfile <QMdmmUtilities>
 * @brief Convenience functions and types for working with QMdmm library
 */

/**
 * @fn QMdmmUtilities::list2Set(const QList<T> &l)
 * @brief Convenience function of converting a QList to QSet
 *
 * Qt deprecates QList::toSet since Qt 5.15 and instead suggests using the QSet iterator ctor. This function does call the ctor.
 */

/**
 * @fn QMdmmUtilities::enumList2VariantList(const QList<T> &list)
 * @brief Convenience function of converting QList<enum> to QVariantList
 */

/**
 * @fn QMdmmUtilities::enumList2VariantList(const QList<QFlags<T> > &list)
 * @brief Convenience function of converting QList<QFlags> to QVariantList
 */

/**
 * @brief Convenience function of converting QList<int> to QVariantList
 */
QVariantList QMdmmUtilities::intList2VariantList(const QList<int> &list)
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
QList<int> QMdmmUtilities::variantList2IntList(const QVariantList &list)
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
QVariantList QMdmmUtilities::stringList2VariantList(const QList<QString> &list)
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
QStringList QMdmmUtilities::variantList2StringList(const QVariantList &list)
{
    QStringList ret;
    ret.reserve(list.length());
    foreach (const QVariant &i, list)
        ret << i.toString();
    return ret;
}
} // namespace v0
} // namespace QMdmmCore
