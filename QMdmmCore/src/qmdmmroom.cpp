// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmroom.h"
#include "qmdmmlogic.h"
#include "qmdmmplayer.h"

#include <QMap>
#include <QString>

struct QMdmmRoomPrivate
{
    QMdmmLogic *logic = nullptr;
    QMap<QString, QMdmmPlayer *> players;
};

QMdmmRoom::QMdmmRoom(QMdmmLogic *logic, QObject *parent)
    : QObject(parent)
    , d(new QMdmmRoomPrivate)
{
    d->logic = logic;
}

QMdmmRoom::~QMdmmRoom()
{
    delete d;
}

QMdmmLogic *QMdmmRoom::logic()
{
    return d->logic;
}

const QMdmmLogic *QMdmmRoom::logic() const
{
    return d->logic;
}

bool QMdmmRoom::addPlayer(const QString &playerName)
{
    if (d->players.contains(playerName))
        return false;

    d->players.insert(playerName, new QMdmmPlayer(playerName, this));
    return true;
}

bool QMdmmRoom::removePlayer(const QString &playerName)
{
    auto it = d->players.find(playerName);

    if (it != d->players.end()) {
        delete it.value();
        d->players.erase(it);
        return true;
    }

    return false;
}

bool QMdmmRoom::full() const
{
    return d->players.size() >= 3;
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
    foreach (const auto &player, d->players) {
        if (player->alive())
            res.push_back(player);
    }

    return res;
}

int QMdmmRoom::alivePlayersCount() const
{
    return alivePlayers().size();
}
