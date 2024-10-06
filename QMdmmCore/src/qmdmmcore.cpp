// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * \file QMdmmCoreGlobal
 * \brief Alternative include of "qmdmmcoreglobal.h". See \ref qmdmmcoreglobal.h for details.
 * \ingroup QMdmmCore
 */
/**
 * \file QMdmmData
 * \brief Include all things from namespace \c QMdmmData. See \ref qmdmmcoreglobal.h for details.
 * \ingroup QMdmmCore
 */
/**
 * \file QMdmmGlobal
 * \brief Include all things from namespace \c QMdmmGlobal. See \ref qmdmmcoreglobal.h for details.
 * \ingroup QMdmmCore
 */
/**
 * \file QMdmmUtilities
 * \brief Include all things from namespace \c QMdmmUtilities. See \ref qmdmmcoreglobal.h for details.
 * \ingroup QMdmmCore
 */

#include "qmdmmcoreglobal.h"

/**
 * \file qmdmmcoreglobal.h
 * \brief Global definition of QMdmmCore library
 * \ingroup QMdmmCore
 */

/**
 * \def QMDMMCORE_EXPORT
 * \brief Indicates this function is public and is exported from QMdmmCore library.
 * \ingroup QMdmmCore
 */

/**
 * \def QMDMMCORE_PRIVATE_EXPORT
 * \brief Indicates this function is private but will be exported from QMdmmCore library if specified during build.
 * \ingroup QMdmmCore
 */

/**
 * \namespace QMdmmData
 * \brief Various data definition of MDMM game
 * \ingroup QMdmmCore
 */

/**
 * \var int QMdmmData::Country
 * \brief For use with \c QMdmmPlayer::place() , if it equals to \c QMdmmData::Country then this player is in Country.
 * \ingroup QMdmmCore
 *
 * This enumeration variable equals to zero. Provided for readability.
 */

/**
 * \enum QMdmmData::DamageReason
 * \brief The reason for a damage.
 * \ingroup QMdmmCore
 */

/**
 * \var QMdmmData::DamageReason QMdmmData::DamageReasonUnknown
 * \brief Unknown / erroneous damage reason.
 * \ingroup QMdmmCore
 */

/**
 * \var QMdmmData::DamageReason QMdmmData::Slashed
 * \brief Damage is caused by a slash.
 * \ingroup QMdmmCore
 */

/**
 * \var QMdmmData::DamageReason QMdmmData::Kicked
 * \brief Damage is caused by a kick.
 * \ingroup QMdmmCore
 */

/**
 * \var QMdmmData::DamageReason QMdmmData::HpPunished
 * \brief Damage is caused by HP punish.
 * \ingroup QMdmmCore
 *
 * There is a mechanism in more modern version of MDMM game, where punishment is applied for slash in city.
 * By default the punished HP is half of the maximum HP, rounded to nearest integer (since it can only be X.0 or X.5 so it is always rounded up)
 */

/**
 * \enum QMdmmData::Action
 * \brief Action executed at a time.
 * \ingroup QMdmmCore
 */

#include <QMap>

namespace {
constexpr bool sscGreater(QMdmmData::StoneScissorsCloth op1, QMdmmData::StoneScissorsCloth op2)
{
    return (op1 == QMdmmData::Stone && op2 == QMdmmData::Scissors) || (op1 == QMdmmData::Scissors && op2 == QMdmmData::Cloth)
        || (op1 == QMdmmData::Cloth && op2 == QMdmmData::Stone);
}
} // namespace

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

QVersionNumber QMdmmGlobal::version()
{
    return QVersionNumber::fromString(QStringLiteral(QMDMM_VERSION));
}

QVariantList QMdmmUtilities::intList2VariantList(const QList<int> &list)
{
    QVariantList ret;
    ret.reserve(list.length());
    foreach (int i, list)
        ret << i;
    return ret;
}

QList<int> QMdmmUtilities::variantList2IntList(const QVariantList &list)
{
    QList<int> ret;
    ret.reserve(list.length());
    foreach (const QVariant &i, list)
        ret << i.toInt();
    return ret;
}

QVariantList stringList2VariantList(const QList<QString> &list)
{
    QVariantList ret;
    ret.reserve(list.length());
    foreach (const QString &i, list)
        ret << i;
    return ret;
}

QStringList variantList2StrList(const QVariantList &list)
{
    QStringList ret;
    ret.reserve(list.length());
    foreach (const QVariant &i, list)
        ret << i.toString();
    return ret;
}
