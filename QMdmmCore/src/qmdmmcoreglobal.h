// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMCOREGLOBAL_H
#define QMDMMCOREGLOBAL_H

#ifndef DOXYGEN
#include <QFlags>
#include <QHash>
#include <QList>
#include <QMetaType>
#include <QSet>
#include <QStringList>
#include <QVariant>
#include <QVersionNumber>
#include <QtGlobal>

#include <type_traits>
#else
#define Q_NAMESPACE_EXPORT
#define Q_FLAG_NS
#define Q_DECLARE_METATYPE
#endif

#if 0
class QMDMMCORE_EXPORT QMdmmCoreGlobal
class QMDMMCORE_EXPORT QMdmmData
class QMDMMCORE_EXPORT QMdmmGlobal
class QMDMMCORE_EXPORT QMdmmUtilities
#endif

#ifndef DOXYGEN
#ifndef QMDMM_STATIC
#ifdef QMDMMCORE_LIBRARY
#define QMDMMCORE_EXPORT Q_DECL_EXPORT
#else
#define QMDMMCORE_EXPORT Q_DECL_IMPORT
#endif
#else
#define QMDMMCORE_EXPORT
#endif
#ifdef QMDMM_NEED_EXPORT_PRIVATE
#define QMDMMCORE_PRIVATE_EXPORT QMDMMCORE_EXPORT
#else
#define QMDMMCORE_PRIVATE_EXPORT
#endif
#else
#define QMDMMCORE_EXPORT dll_interface_defined
#define QMDMMCORE_PRIVATE_EXPORT dll_interface_defined
#endif

#include "qmdmmdebug.h"

namespace QMdmmData {
Q_NAMESPACE_EXPORT(QMDMMCORE_EXPORT)

// Originally there was enum Place, but removed due to overdesign.
// It is revealed that number of places should be same of number of players
// So if we need City1, City2 and etc. we need as many Cities as the maximum supported player numbers
// In theory the player numbers should be unlimitted since there is no difference between players
// Added back of Enum Place but preserve the only "Country" value, for QMetaObject generation only
enum Place
{
    Country = 0,
};
Q_ENUM_NS(Place)

enum DamageReason
{
    DamageReasonUnknown,
    Slashed,
    Kicked,
    HpPunished,
};
Q_ENUM_NS(DamageReason)

enum StoneScissorsCloth
{
    Stone,
    Scissors,
    Cloth,
};
Q_ENUM_NS(StoneScissorsCloth)

enum Action
{
    DoNothing,
    BuyKnife,
    BuyHorse,
    Slash,
    Kick,
    Move,
    LetMove,
};
Q_ENUM_NS(Action)

enum UpgradeItem
{
    UpgradeKnife,
    UpgradeHorse,
    UpgradeMaxHp,
};
Q_ENUM_NS(UpgradeItem)

enum AgentStateEnum
{
    StateMaskOnline = 0x10,
    StateMaskBot = 0x01,
    StateMaskTrust = 0x08,

    StateOffline = 0x0,
    StateOfflineBot = 0x01,
    StateOnline = 0x10,
    StateOnlineBot = 0x11,
    StateOnlineTrust = 0x18,
};
Q_DECLARE_FLAGS(AgentState, AgentStateEnum)
Q_FLAG_NS(AgentState)

constexpr bool isPlaceAdjecent(int p1, int p2)
{
    // simplifies to "only one of p1 and p2 is Country"
    // simplifies again to "p1 is Country xor p2 is Country"
    // (boolean xor == notequal)
    return (p1 == Country) != (p2 == Country);
}

QMDMMCORE_EXPORT QStringList stoneScissorsClothWinners(const QHash<QString, StoneScissorsCloth> &judgers);
} // namespace QMdmmData

namespace QMdmmGlobal {
QMDMMCORE_EXPORT QVersionNumber version();
}

namespace QMdmmUtilities {
template<typename T>
QSet<T> list2Set(const QList<T> &l)
{
    return QSet<T>(l.constBegin(), l.constEnd());
}
template<typename T>
QVariantList enumList2VariantList(const QList<T> &list)
{
    static_assert(std::is_enum_v<T>);

    QVariantList ret;
    ret.reserve(list.length());
    foreach (T i, list)
        ret << static_cast<int>(i);
    return ret;
}
template<typename T>
QVariantList enumList2VariantList(const QList<QFlags<T>> &list)
{
    QVariantList ret;
    ret.reserve(list.length());
    foreach (const QFlags<T> &i, list)
        ret << static_cast<int>(typename QFlags<T>::Int(i));
    return ret;
}
QVariantList intList2VariantList(const QList<int> &list);
QList<int> variantList2IntList(const QVariantList &list);
QVariantList stringList2VariantList(const QList<QString> &list); // !!!: Qt 5 QStringList is not QList<QString> but Qt 6 is
QStringList variantList2StrList(const QVariantList &list);
} // namespace QMdmmUtilities

#endif // QMDMMCOREGLOBAL_H
