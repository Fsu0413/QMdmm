// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmdebug.h"
#include "qmdmmdebug_p.h"

#include <QDebug>
#include <QIODevice>

/**
 * @file qmdmmdebug.h
 * @brief QMdmm Debug stuff
 */

using namespace QMdmmCore::p;

namespace QMdmmCore {
#ifndef DOXYGEN
namespace v0 {
#else
inline namespace v1 {
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

    debugLogInstance()->f = f;
    if (DebugLog::qtMessageHandler == nullptr)
        DebugLog::qtMessageHandler = qInstallMessageHandler(&messageOutput);
}

} // namespace v0
} // namespace QMdmmCore
