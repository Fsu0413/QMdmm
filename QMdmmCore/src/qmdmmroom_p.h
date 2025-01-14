// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMROOM_P
#define QMDMMROOM_P

#include "qmdmmroom.h"

#include <QMap>

struct QMDMMCORE_PRIVATE_EXPORT QMdmmRoomPrivate final
{
    QMap<QString, QMdmmPlayer *> players;
    QMdmmLogicConfiguration logicConfiguration;
};

#endif
