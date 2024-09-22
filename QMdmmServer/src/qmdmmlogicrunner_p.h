// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMLOGICRUNNER_P
#define QMDMMLOGICRUNNER_P

#include "qmdmmlogicrunner.h"

#include <QMdmmLogic>

#include <QPointer>

struct QMDMMSERVER_EXPORT QMdmmLogicRunnerPrivate
{
    QPointer<QMdmmLogic> logic;
};

#endif
