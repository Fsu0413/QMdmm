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

constexpr const int Country = 0;

enum DamageReason
{
    Unknown,
    Slash,
    Kick,
    PunishHp
};
Q_FLAG_NS(DamageReason)

enum StoneScissorsCloth
{
    Stone,
    Scissors,
    Cloth
};
Q_FLAG_NS(StoneScissorsCloth)

QMDMMCORE_EXPORT bool isPlaceAdjecent(int p1, int p2);
QMDMMCORE_EXPORT QStringList stoneScissorsClothWinners(const QHash<QString, StoneScissorsCloth> &judgers);
} // namespace QMdmmData

Q_DECLARE_METATYPE(QMdmmData::DamageReason)
Q_DECLARE_METATYPE(QMdmmData::StoneScissorsCloth)

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

#endif // QMDMMLOGIC_GLOBAL_H
