// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmplayer_p.h"
#include "qmdmmplayer.h"

#include "qmdmmroom.h"

namespace QMdmmCore {

QMdmmPlayerPrivate::QMdmmPlayerPrivate(QMdmmRoom *room)
    : knife(false)
    , horse(false)
    , hp(room->logicConfiguration().initialMaxHp())
    , place(QMdmmData::Country)
    , initialPlace(QMdmmData::Country)
    , knifeDamage(room->logicConfiguration().initialKnifeDamage())
    , horseDamage(room->logicConfiguration().initialHorseDamage())
    , maxHp(room->logicConfiguration().initialMaxHp())
    , upgradePoint(0)
{
}

void QMdmmPlayerPrivate::applyDamage(QMdmmPlayer *from, QMdmmPlayer *to, int damagePoint, QMdmmData::DamageReason reason)
{
    Q_ASSERT(from->room() == to->room());

    bool kills = false;

    to->setHp(to->hp() - damagePoint, &kills);
    emit to->damaged(from, damagePoint, reason, QMdmmPlayer::QPrivateSignal());
    emit to->damaged(from->objectName(), damagePoint, reason, QMdmmPlayer::QPrivateSignal());

    if (kills)
        from->setUpgradePoint(from->upgradePoint() + 1);
}

} // namespace QMdmmCore
