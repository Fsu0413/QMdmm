// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmdebug.h"
#include "qmdmmdebug_p.h"

#include <QDebug>
#include <QIODevice>

using namespace QMdmmCore::p;

namespace QMdmmCore {
#ifndef DOXYGEN
namespace v0 {
#endif

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

#ifndef DOXYGEN
} // namespace v0
#endif

} // namespace QMdmmCore
