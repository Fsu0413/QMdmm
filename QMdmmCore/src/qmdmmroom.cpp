// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmroom.h"
#include "qmdmmlogic.h"
#include "qmdmmplayer.h"

#include <QMap>
#include <QString>

struct QMdmmRoomPrivate
{
    QMap<QString, QMdmmPlayer *> players;
};

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

    return ret;
}

bool QMdmmRoom::removePlayer(const QString &playerName)
{
    QMap<QString, QMdmmPlayer *>::iterator it = d->players.find(playerName);

    if (it != d->players.end()) {
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

QMdmmPlayer *QMdmmRoom::player(const QString &playerName) const
{
    return d->players.value(playerName, nullptr);
}

QList<QMdmmPlayer *> QMdmmRoom::players() const
{
    return d->players.values();
}

QStringList QMdmmRoom::playerNames() const
{
    return d->players.keys();
}

QList<QMdmmPlayer *> QMdmmRoom::alivePlayers() const
{
    QList<QMdmmPlayer *> res;
    foreach (QMdmmPlayer *player, d->players) {
        if (player->alive())
            res.push_back(player);
    }

    return res;
}

int QMdmmRoom::alivePlayersCount() const
{
    return alivePlayers().size();
}
