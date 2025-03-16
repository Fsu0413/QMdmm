// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMPLAYER_P
#define QMDMMPLAYER_P

#include "qmdmmplayer.h"

// NOLINTBEGIN(misc-non-private-member-variables-in-classes): This is private header

namespace QMdmmCore {

struct QMDMMCORE_PRIVATE_EXPORT QMdmmPlayerPrivate final
{
    QMdmmPlayerPrivate(v0::QMdmmRoom *room);

    bool knife;
    bool horse;
    int hp;
    int place;
    int initialPlace;

    int knifeDamage;
    int horseDamage;
    int maxHp;

    int upgradePoint;

    static void applyDamage(QMdmmPlayer *from, QMdmmPlayer *to, int damagePoint, QMdmmData::DamageReason reason);
};

} // namespace QMdmmCore

// NOLINTEND(misc-non-private-member-variables-in-classes): This is private header

#endif
