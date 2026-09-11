// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmdebug.h"
#include "qmdmmdebug_p.h"

#include <QDebug>
#include <QIODevice>

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

void qMdmmDebugSetDevice(QIODevice *f)
{
    if (f != nullptr) {
        if (!f->isOpen())
            f->open(QIODevice::WriteOnly);
    }

    p::debugLogInstance()->f = f;
    if (p::DebugLogP::qtMessageHandler == nullptr)
        p::DebugLogP::qtMessageHandler = qInstallMessageHandler(&p::messageOutput);
}

#ifndef DOXYGEN
} // namespace v0
#endif

} // namespace QMdmmCore
