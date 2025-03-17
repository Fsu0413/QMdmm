// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMDEBUG_P
#define QMDMMDEBUG_P

#include "qmdmmdebug.h"

#include <QMutex>
#include <QPointer>

namespace QMdmmCore {

namespace p {

struct QMDMMCORE_PRIVATE_EXPORT DebugLog
{
    QMutex m;
    QPointer<QIODevice> f;

    static QtMessageHandler qtMessageHandler;
};

QMDMMCORE_PRIVATE_EXPORT DebugLog *debugLogInstance();
QMDMMCORE_PRIVATE_EXPORT void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

} // namespace p

} // namespace QMdmmCore

#endif
