// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmplayer_p.h"
#include "qmdmmplayer.h"

#include "qmdmmroom.h"

#include <algorithm>

namespace QMdmmCore {

namespace p {

PlayerP::PlayerP(Room *room)
    : knife(false)
    , horse(false)
    , hp(room->logicConfiguration().initialMaxHp())
    , place(Data::Country)
    , initialPlace(Data::Country)
    , knifeDamage(room->logicConfiguration().initialKnifeDamage())
    , horseDamage(room->logicConfiguration().initialHorseDamage())
    , maxHp(room->logicConfiguration().initialMaxHp())
    , upgradePoint(0)
{
}

void PlayerP::applyDamage(Player *from, Player *to, int damagePoint, Data::DamageReason reason)
{
    Q_ASSERT(from->room() == to->room());

    bool kills = false;

    to->setHp(to->hp() - damagePoint, &kills);
    emit to->damaged(from, damagePoint, reason, Player::QPrivateSignal());

    if (kills) {
        // Cap the upgrade point to the remaining upgrade times so a kill never grants
        // more points than the killer can actually spend in the upgrade phase.
        const int remaining = from->upgradeKnifeRemainingTimes() + from->upgradeHorseRemainingTimes() + from->upgradeMaxHpRemainingTimes();
        from->setUpgradePoint(std::min(from->upgradePoint() + 1, remaining));
    }
}

} // namespace p

} // namespace QMdmmCore
