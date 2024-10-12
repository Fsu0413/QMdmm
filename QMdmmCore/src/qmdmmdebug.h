// SPDX-License-Identifier: AGPL-3.0-or-later

// qmdmmglobal.h includes this header file
#include "qmdmmcoreglobal.h"

#ifndef QMDMMDEBUG_H
#define QMDMMDEBUG_H

#if 0
class QMDMMCORE_EXPORT QMdmmDebug
#endif

#include <QDebug>
#include <QFileDevice>

QMDMMCORE_EXPORT void qMdmmDebugSetDevice(QIODevice *f);

struct QMDMMCORE_EXPORT QMdmmDebugLogOutputOnDestruct final
{
    explicit constexpr QMdmmDebugLogOutputOnDestruct(const char *outputContent)
        : outputContent(outputContent)
    {
    }
    ~QMdmmDebugLogOutputOnDestruct()
    {
        qDebug("%s", outputContent);
    }
    constexpr void setOutputContent(const char *outputContent)
    {
        this->outputContent = outputContent;
    }

    QMdmmDebugLogOutputOnDestruct() = delete;
    Q_DISABLE_COPY_MOVE(QMdmmDebugLogOutputOnDestruct);

private:
    const char *outputContent;
};

#endif
