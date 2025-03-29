// SPDX-License-Identifier: AGPL-3.0-or-later

// qmdmmglobal.h includes this header file
#include "qmdmmcoreglobal.h"

#ifndef QMDMMDEBUG_H
#define QMDMMDEBUG_H

#include <QDebug>
#include <QFileDevice>

QMDMM_EXPORT_NAME(QMdmmDebug)

namespace QMdmmCore {
#ifndef DOXYGEN
namespace v0 {
#else
inline namespace v1 {
#endif
QMDMMCORE_EXPORT void qMdmmDebugSetDevice(QIODevice *f);
} // namespace v0

#ifndef DOXYGEN
inline namespace v1 {
using v0::qMdmmDebugSetDevice;
} // namespace v1
#endif

} // namespace QMdmmCore

#endif
