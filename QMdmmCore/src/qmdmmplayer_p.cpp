// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmplayer_p.h"
#include "qmdmmplayer.h"

#include "qmdmmroom.h"

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
    emit to->damaged(from->objectName(), damagePoint, reason, Player::QPrivateSignal());

    if (kills)
        from->setUpgradePoint(from->upgradePoint() + 1);
}

} // namespace p

} // namespace QMdmmCore
