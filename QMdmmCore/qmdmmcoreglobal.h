// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMCOREGLOBAL_H
#define QMDMMCOREGLOBAL_H

#include <cstddef>

#if 0
class QMDMMCORE_EXPORT QMdmmCoreGlobal
#endif

#ifndef QMDMM_STATIC
#ifdef _WIN32
#ifdef QMDMMCORE_LIBRARY
#define QMDMMCORE_EXPORT __declspec(dllexport)
#else
#define QMDMMCORE_EXPORT __declspec(dllimport)
#endif
#else
#ifdef QMDMMCORE_LIBRARY
#define QMDMMCORE_EXPORT __attribute__((visibility("default")))
#else
#define QMDMMCORE_EXPORT
#endif
#endif
#else
#define QMDMMCORE_EXPORT
#endif

#define QMDMMCORE_EXPORT_NOHEADER QMDMMCORE_EXPORT

#define QMDMM_DISABLE_COPY(c) \
    c(const c &) = delete;    \
    c &operator=(const c &) = delete;

#define QMDMM_UNUSED(n) ((void)n)

#define QMDMM_FOREVER for (;;)

#ifndef forever
#define forever for (;;)
#endif

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

#endif // QMDMMLOGIC_GLOBAL_H
