// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmdebug_p.h"
#include "qmdmmdebug.h"

#include <QMutexLocker>

namespace QMdmmCore {

namespace p {

QtMessageHandler DebugLog::qtMessageHandler = nullptr;

DebugLog *debugLogInstance()
{
    static DebugLog i;
    return &i;
}

void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (debugLogInstance()->f != nullptr) {
        QString log = qFormatLogMessage(type, context, msg);
        QMutexLocker lock(&debugLogInstance()->m);
        debugLogInstance()->f->write(log.append(QStringLiteral("\n")).toUtf8());
        if (debugLogInstance()->f->inherits("QFileDevice")) {
            QFileDevice *f = qobject_cast<QFileDevice *>(debugLogInstance()->f.data());
            if (f != nullptr)
                f->flush();
        }
    } else if (DebugLog::qtMessageHandler != nullptr && DebugLog::qtMessageHandler != &messageOutput) {
        (*DebugLog::qtMessageHandler)(type, context, msg);
    } else {
        // Is this public API?
        // But anyway it is declared as Q_CORE_EXPORT let's use it directly since we have no other good fallback
        qt_message_output(type, context, msg);
    }
}

} // namespace p

} // namespace QMdmmCore
