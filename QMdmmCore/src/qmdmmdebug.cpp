// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmdebug.h"
#include "qmdmmdebug_p.h"

#include <QMutexLocker>

QtMessageHandler QMdmmDebugLogPrivate::qtMessageHandler = nullptr;

QMdmmDebugLogPrivate *qMdmmDebugLogPrivateInstance()
{
    static QMdmmDebugLogPrivate i;
    return &i;
}

#define l (qMdmmDebugLogPrivateInstance())

void qMdmmMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (l->f != nullptr) {
        QString log = qFormatLogMessage(type, context, msg);
        QMutexLocker lock(&l->m);
        l->f->write(log.append(QStringLiteral("\n")).toUtf8());
        l->f->flush();
    } else if (QMdmmDebugLogPrivate::qtMessageHandler != nullptr && QMdmmDebugLogPrivate::qtMessageHandler != &qMdmmMessageOutput) {
        (*QMdmmDebugLogPrivate::qtMessageHandler)(type, context, msg);
    } else {
        // Is this public API?
        qt_message_output(type, context, msg);
    }
}

void qMdmmDebugSetDevice(QFileDevice *f)
{
    if (f != nullptr) {
        if (!f->isOpen())
            f->open(QIODevice::WriteOnly);
    }

    l->f = f;
    if (QMdmmDebugLogPrivate::qtMessageHandler == nullptr)
        QMdmmDebugLogPrivate::qtMessageHandler = qInstallMessageHandler(&qMdmmMessageOutput);
}
