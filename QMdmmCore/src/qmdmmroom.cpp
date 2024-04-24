// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmroom.h"
#include "qmdmmplayer.h"

#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

using std::map;
using std::ostringstream;
using std::out_of_range;
using std::string;
using std::vector;

struct QMdmmRoomPrivate
{
    QMdmmRoomPrivate() = default;
    map<string, QMdmmPlayer *> players;
};

QMdmmRoom::QMdmmRoom()
    : d(new QMdmmRoomPrivate)
{
}

QMdmmRoom::~QMdmmRoom()
{
    for (auto &[name, player] : d->players)
        delete player;

    delete d;
}

bool QMdmmRoom::addPlayer(const string &playerName)
{
    if (d->players.find(playerName) != d->players.cend())
        return false;

    d->players.emplace(playerName, new QMdmmPlayer(playerName));
    return true;
}

bool QMdmmRoom::removePlayer(const std::string &playerName)
{
    std::map<string, QMdmmPlayer *>::iterator it = d->players.find(playerName);
    if (it != d->players.cend()) {
        delete it->second;
        d->players.erase(it);
        return true;
    }

    return false;
}

bool QMdmmRoom::full() const
{
    return d->players.size() >= 3;
}

QMdmmPlayer *QMdmmRoom::player(const string &playerName) const
{
    try {
        return d->players.at(playerName);
    } catch (const out_of_range &) {
        return nullptr;
    }
}

vector<QMdmmPlayer *> QMdmmRoom::players() const
{
    vector<QMdmmPlayer *> res;
    for (const auto &[name, player] : d->players)
        res.push_back(player);

    return res;
}

vector<string> QMdmmRoom::playerNames() const
{
    vector<string> res;
    for (const auto &[name, player] : d->players)
        res.push_back(name);

    return res;
}

vector<QMdmmPlayer *> QMdmmRoom::alivePlayers() const
{
    vector<QMdmmPlayer *> res;
    for (const auto &[name, player] : d->players) {
        if (player->alive())
            res.push_back(player);
    }

    return res;
}

int QMdmmRoom::alivePlayersCount() const
{
    return int(alivePlayers().size());
}
