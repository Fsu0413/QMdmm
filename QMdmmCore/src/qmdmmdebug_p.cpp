// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmdebug_p.h"
#include "qmdmmdebug.h"

#include <QMutexLocker>

namespace QMdmmCore {

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
        QMutexLocker lock(&qMdmmDebugLogPrivateInstance()->m);
        qMdmmDebugLogPrivateInstance()->f->write(log.append(QStringLiteral("\n")).toUtf8());
        if (qMdmmDebugLogPrivateInstance()->f->inherits("QFileDevice")) {
            QFileDevice *f = qobject_cast<QFileDevice *>(qMdmmDebugLogPrivateInstance()->f.data());
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

} // namespace QMdmmCore
