// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMDEBUG_P
#define QMDMMDEBUG_P

#include "qmdmmdebug.h"

#include <QMutex>
#include <QPointer>

struct QMDMMCORE_PRIVATE_EXPORT QMdmmDebugLogPrivate
{
    QMutex m;
    QPointer<QFileDevice> f;

    static QtMessageHandler qtMessageHandler;
};

QMDMMCORE_PRIVATE_EXPORT QMdmmDebugLogPrivate *qMdmmDebugLogPrivateInstance();
QMDMMCORE_PRIVATE_EXPORT void qMdmmMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

#endif
