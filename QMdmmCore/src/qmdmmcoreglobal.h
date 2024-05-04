// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMCOREGLOBAL_H
#define QMDMMCOREGLOBAL_H

#include <QMetaType>
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
enum Place
{
    Country,
    City1,
    City2,
    City3
};

enum DamageReason
{
    Unknown,
    Slash,
    Kick,
    PunishHp
};

enum StoneScissorsCloth
{
    Stone,
    Scissors,
    Cloth
};

QMDMMCORE_EXPORT bool isPlaceAdjecent(Place p1, Place p2);
} // namespace QMdmmData

Q_DECLARE_METATYPE(QMdmmData::Place)
Q_DECLARE_METATYPE(QMdmmData::DamageReason)
Q_DECLARE_METATYPE(QMdmmData::StoneScissorsCloth)

namespace QMdmmGlobal {
QMDMMCORE_EXPORT QVersionNumber version();
}

#endif // QMDMMLOGIC_GLOBAL_H
