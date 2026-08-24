// SPDX-License-Identifier: AGPL-3.0-or-later

// qmdmmglobal.h includes this header file
#include "qmdmmcoreglobal.h"

#ifndef QMDMMDEBUG_H
#define QMDMMDEBUG_H

#include <QDebug>
#include <QFileDevice>

QMDMM_EXPORT_NAME(QMdmmDebug)

/**
 * @file qmdmmdebug.h
 * @brief QMdmm Debug stuff
 */

namespace QMdmmCore {
#ifndef DOXYGEN
namespace v0 {
#endif

/**
 * @brief Set the device where QDebug outputs log to.
 * @param f The target output device
 *
 * By default Qt outputs log to a Qt defined buffer. This changes the buffer to our one, for collecting the log we generates
 */
QMDMMCORE_EXPORT void qMdmmDebugSetDevice(QIODevice *f);

#ifndef DOXYGEN
} // namespace v0
inline namespace v1 {
using v0::qMdmmDebugSetDevice;
} // namespace v1
#endif

} // namespace QMdmmCore

#endif
