// SPDX-License-Identifier: AGPL-3.0-or-later

// qmdmmglobal.h includes this header file
#include "qmdmmcoreglobal.h"

#ifndef QMDMMDEBUG_H
#define QMDMMDEBUG_H

#include <QDebug>
#include <QFileDevice>

QMDMM_EXPORT_NAME(QMdmmDebug)

namespace QMdmmCore {
namespace v0 {
QMDMMCORE_EXPORT void qMdmmDebugSetDevice(QIODevice *f);
} // namespace v0

inline namespace v1 {
using v0::qMdmmDebugSetDevice;
} // namespace v1

} // namespace QMdmmCore

#endif
