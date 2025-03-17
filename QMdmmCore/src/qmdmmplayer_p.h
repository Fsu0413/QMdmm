// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMPLAYER_P
#define QMDMMPLAYER_P

#include "qmdmmplayer.h"
#include "qmdmmroom.h"

// NOLINTBEGIN(misc-non-private-member-variables-in-classes): This is private header

namespace QMdmmCore {

namespace p {

struct QMDMMCORE_PRIVATE_EXPORT PlayerP final
{
    PlayerP(Room *room);

    bool knife;
    bool horse;
    int hp;
    int place;
    int initialPlace;

    int knifeDamage;
    int horseDamage;
    int maxHp;

    int upgradePoint;

    static void applyDamage(Player *from, Player *to, int damagePoint, Data::DamageReason reason);
};

} // namespace p

} // namespace QMdmmCore

// NOLINTEND(misc-non-private-member-variables-in-classes): This is private header

#endif
