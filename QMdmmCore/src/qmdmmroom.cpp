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
    delete d;
}

string QMdmmRoom::addPlayer(QMdmmPlayer *player, const string &userName)
{
    string useName = userName;
    static int no = 1;

    if (userName.empty()) {
        ostringstream oss;
        oss << no++;
        useName = string("QMdmm") + oss.str();
    }

    while (d->players.find(useName) != d->players.cend()) {
        ostringstream oss;
        oss << no++;
        useName = useName + oss.str();
    }
    d->players[useName] = player;
    player->setName(useName);

    return useName;
}

bool QMdmmRoom::full() const
{
    return d->players.size() >= 3;
}

bool QMdmmRoom::removePlayer(QMdmmPlayer *player)
{
    for (auto it = d->players.begin(); it != d->players.end(); ++it) {
        if (it->second == player) {
            d->players.erase(it);
            return true;
        }
    }

    return false;
}

bool QMdmmRoom::removePlayer(const string &playerName)
{
    auto it = d->players.find(playerName);
    if (it != d->players.cend()) {
        d->players.erase(it);
        return true;
    }

    return false;
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
    for (auto it = d->players.begin(); it != d->players.end(); ++it)
        res.push_back(it->second);

    return res;
}

vector<string> QMdmmRoom::playerNames() const
{
    vector<string> res;
    for (auto it = d->players.begin(); it != d->players.end(); ++it)
        res.push_back(it->first);

    return res;
}

vector<QMdmmPlayer *> QMdmmRoom::alivePlayers() const
{
    vector<QMdmmPlayer *> res;
    for (auto it = d->players.begin(); it != d->players.end(); ++it) {
        if (it->second->alive())
            res.push_back(it->second);
    }

    return res;
}

int QMdmmRoom::alivePlayersCount() const
{
    return int(alivePlayers().size());
}
