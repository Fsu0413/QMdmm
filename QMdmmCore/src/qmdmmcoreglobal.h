// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMCOREGLOBAL_H
#define QMDMMCOREGLOBAL_H

#include <QHash>
#include <QList>
#include <QMetaType>
#include <QSet>
#include <QStringList>
#include <QVersionNumber>
#include <QtGlobal>

#if 0
class QMDMMCORE_EXPORT QMdmmCoreGlobal
class QMDMMCORE_EXPORT QMdmmData
class QMDMMCORE_EXPORT QMdmmGlobal
class QMDMMCORE_EXPORT QMdmmUtilities
#endif

#ifndef QMDMM_STATIC
#ifdef QMDMMCORE_LIBRARY
#define QMDMMCORE_EXPORT Q_DECL_EXPORT
#else
#define QMDMMCORE_EXPORT Q_DECL_IMPORT
#endif
#else
#define QMDMMCORE_EXPORT
#endif

#define QMDMMCORE_EXPORT_NOHEADER QMDMMCORE_EXPORT

namespace QMdmmData {
Q_NAMESPACE_EXPORT(QMDMMCORE_EXPORT)

// Originally there was enum Place, but removed due to overdesign.
// It is revealed that number of places should be same of number of players
// So if we need City1, City2 and etc. we need as many Cities as the maximum supported player numbers
// In theory the player numbers should be unlimitted since there is no difference between players
enum
{
    Country = 0,
};

enum DamageReason
{
    Unknown,
    Slashed,
    Kicked,
    HpPunished
};
Q_FLAG_NS(DamageReason)

enum StoneScissorsCloth
{
    Stone,
    Scissors,
    Cloth
};
Q_FLAG_NS(StoneScissorsCloth)

enum Operation
{
    DoNothing,
    BuyKnife,
    BuyHorse,
    Slash,
    Kick,
    Move,
    LetMove,
};
Q_FLAG_NS(Operation)

enum UpgradeItem
{
    UpgradeKnife,
    UpgradeHorse,
    UpgradeMaxHp,
};
Q_FLAG_NS(UpgradeItem)

QMDMMCORE_EXPORT bool isPlaceAdjecent(int p1, int p2);
QMDMMCORE_EXPORT QStringList stoneScissorsClothWinners(const QHash<QString, StoneScissorsCloth> &judgers);
} // namespace QMdmmData

Q_DECLARE_METATYPE(QMdmmData::DamageReason)
Q_DECLARE_METATYPE(QMdmmData::StoneScissorsCloth)
Q_DECLARE_METATYPE(QMdmmData::Operation)
Q_DECLARE_METATYPE(QMdmmData::UpgradeItem)

namespace QMdmmGlobal {
QMDMMCORE_EXPORT QVersionNumber version();
}

namespace QMdmmUtilities {
template<typename T>
QSet<T> qList2QSet(const QList<T> &l)
{
    return QSet<T>(l.constBegin(), l.constEnd());
}
} // namespace QMdmmUtilities

#endif // QMDMMCOREGLOBAL_H
