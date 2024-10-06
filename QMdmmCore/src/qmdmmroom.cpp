// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmroom.h"
#include "qmdmmroom_p.h"

#include "qmdmmlogic.h"
#include "qmdmmplayer.h"

#include <QString>

QMdmmRoom::QMdmmRoom(QMdmmLogic *logic)
    : QObject(logic)
    , d(new QMdmmRoomPrivate)
{
}

QMdmmRoom::~QMdmmRoom()
{
    delete d;
}

QMdmmLogic *QMdmmRoom::logic()
{
    // We don't need extra cost for qobject_cast here, since every Room is created with Logic as parent.
    return static_cast<QMdmmLogic *>(parent());
}

const QMdmmLogic *QMdmmRoom::logic() const
{
    // same as above
    return static_cast<const QMdmmLogic *>(parent());
}

QMdmmPlayer *QMdmmRoom::addPlayer(const QString &playerName)
{
    if (d->players.contains(playerName))
        return nullptr;

    QMdmmPlayer *ret = new QMdmmPlayer(playerName, this);
    d->players.insert(playerName, ret);

    emit playerAdded(playerName, QPrivateSignal());

    return ret;
}

bool QMdmmRoom::removePlayer(const QString &playerName)
{
    if (QMap<QString, QMdmmPlayer *>::iterator it = d->players.find(playerName); it != d->players.end()) {
        emit playerRemoved(playerName, QPrivateSignal());

        delete it.value();
        d->players.erase(it);
        return true;
    }

    return false;
}

bool QMdmmRoom::full() const
{
    return d->players.size() >= logic()->configuration().playerNumPerRoom;
}

QMdmmPlayer *QMdmmRoom::player(const QString &playerName)
{
    return d->players.value(playerName, nullptr);
}

const QMdmmPlayer *QMdmmRoom::player(const QString &playerName) const
{
    return d->players.value(playerName, nullptr);
}

QList<QMdmmPlayer *> QMdmmRoom::players()
{
    return d->players.values();
}

QList<const QMdmmPlayer *> QMdmmRoom::players() const
{
    QList<const QMdmmPlayer *> res;
    foreach (const QMdmmPlayer *player, d->players)
        res << player;

    return res;
}

QStringList QMdmmRoom::playerNames() const
{
    return d->players.keys();
}

QList<QMdmmPlayer *> QMdmmRoom::alivePlayers()
{
    QList<QMdmmPlayer *> res;
    foreach (QMdmmPlayer *player, d->players) {
        if (player->alive())
            res << player;
    }

    return res;
}

QList<const QMdmmPlayer *> QMdmmRoom::alivePlayers() const
{
    QList<const QMdmmPlayer *> res;
    foreach (const QMdmmPlayer *player, d->players) {
        if (player->alive())
            res << player;
    }

    return res;
}

QStringList QMdmmRoom::alivePlayerNames() const
{
    QStringList res;
    for (QMap<QString, QMdmmPlayer *>::const_iterator it = d->players.constBegin(); it != d->players.constEnd(); ++it) {
        if (it.value()->alive())
            res.push_back(it.key());
    }

    return res;
}

int QMdmmRoom::alivePlayersCount() const
{
    return alivePlayers().size();
}

bool QMdmmRoom::isRoundOver() const
{
    return alivePlayersCount() <= 1;
}

bool QMdmmRoom::isGameOver(QStringList *winnerPlayerNames) const
{
    bool ret = false;
    if (winnerPlayerNames != nullptr)
        winnerPlayerNames->clear();

    foreach (const QMdmmPlayer *player, d->players) {
        if (!player->canUpdateHorse() && !player->canUpdateKnife() && !player->canUpdateMaxHp()) {
            ret = true;
            if (winnerPlayerNames != nullptr)
                *winnerPlayerNames << player->objectName();
        }
    }

    return ret;
}

void QMdmmRoom::prepareForRoundStart()
{
    int i = 0;
    foreach (QMdmmPlayer *player, d->players)
        player->prepareForRoundStart(++i);
}

void QMdmmRoom::resetUpgrades()
{
    foreach (QMdmmPlayer *player, d->players)
        player->resetUpgrades();
}
