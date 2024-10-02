// SPDX-License-Identifier: AGPL-3.0-or-later

// qmdmmglobal.h includes this header file
#include "qmdmmcoreglobal.h"

#ifndef QMDMMDEBUG_H
#define QMDMMDEBUG_H

#if 0
class QMDMMCORE_EXPORT QMdmmDebug
#endif

#include <QDebug>
#include <QFileDevice>

QMDMMCORE_EXPORT void qMdmmDebugSetDevice(QFileDevice *f);

#endif
