// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmdebug.h"
#include "qmdmmdebug_p.h"

#include <QMutexLocker>

/**
 * @file qmdmmdebug.h
 * @brief QMdmm Debug stuff
 */

#ifndef DOXYGEN

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
        if (l->f->inherits("QFileDevice")) {
            QFileDevice *f = qobject_cast<QFileDevice *>(l->f.data());
            if (f != nullptr)
                f->flush();
        }
    } else if (QMdmmDebugLogPrivate::qtMessageHandler != nullptr && QMdmmDebugLogPrivate::qtMessageHandler != &qMdmmMessageOutput) {
        (*QMdmmDebugLogPrivate::qtMessageHandler)(type, context, msg);
    } else {
        // Is this public API?
        // But anyway it is declared as Q_CORE_EXPORT let's use it directly since we have no other good fallback
        qt_message_output(type, context, msg);
    }
}

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

    l->f = f;
    if (QMdmmDebugLogPrivate::qtMessageHandler == nullptr)
        QMdmmDebugLogPrivate::qtMessageHandler = qInstallMessageHandler(&qMdmmMessageOutput);
}
