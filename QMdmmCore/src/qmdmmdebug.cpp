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

void qMdmmMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (qMdmmDebugLogPrivateInstance()->f != nullptr) {
        QString log = qFormatLogMessage(type, context, msg);
        QMutexLocker l(&qMdmmDebugLogPrivateInstance()->m);
        qMdmmDebugLogPrivateInstance()->f->write(log.append(QStringLiteral("\n")).toUtf8());
    } else if (QMdmmDebugLogPrivate::qtMessageHandler != nullptr) {
        (*QMdmmDebugLogPrivate::qtMessageHandler)(type, context, msg);
    } else {
        // Is this public API?
        qt_message_output(type, context, msg);
    }
}

void qMdmmDebugSetDevice(QIODevice *f)
{
    if (f != nullptr) {
        if (!f->isOpen())
            f->open(QIODevice::WriteOnly);
    }

    qMdmmDebugLogPrivateInstance()->f = f;
    if (QMdmmDebugLogPrivate::qtMessageHandler == nullptr)
        QMdmmDebugLogPrivate::qtMessageHandler = qInstallMessageHandler(&qMdmmMessageOutput);
}
