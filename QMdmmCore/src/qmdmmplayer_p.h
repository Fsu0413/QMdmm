// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMPLAYER_P
#define QMDMMPLAYER_P

#include "qmdmmplayer.h"

struct QMDMMCORE_PRIVATE_EXPORT QMdmmPlayerPrivate final
{
    QMdmmPlayerPrivate(QMdmmRoom *room);

    bool knife;
    bool horse;
    int hp;
    int place;
    int initialPlace;

    int knifeDamage;
    int horseDamage;
    int maxHp;

    int upgradePoint;
};

#endif
