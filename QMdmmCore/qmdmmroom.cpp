#include "qmdmmroom.h"
#include "qmdmmplayer.h"

#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

using std::map;
using std::ostringstream;
using std::out_of_range;

struct QMdmmRoomPrivate
{
    constexpr QMdmmRoomPrivate()
        : players()
    {
    }

    map<string, QMdmmPlayer *> players;
};

QMdmmRoom::QMdmmRoom()
    : d_ptr(new QMdmmRoomPrivate)
{
}

QMdmmRoom::~QMdmmRoom()
{
    delete d_ptr;
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

    QMDMMD(QMdmmRoom);
    if (d->players.find(useName) != d->players.cend()) {
        ostringstream oss;
        oss << no++;
        useName = userName + oss.str();
    }
    d->players[useName] = player;

    return useName;
}

bool QMdmmRoom::removePlayer(QMdmmPlayer *player)
{
    QMDMMD(QMdmmRoom);
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
    QMDMMD(QMdmmRoom);
    auto it = d->players.find(playerName);
    if (it != d->players.cend()) {
        d->players.erase(it);
        return true;
    }

    return false;
}

QMdmmPlayer *QMdmmRoom::player(const string &playerName) const
{
    QMDMMD(const QMdmmRoom);
    try {
        return d->players.at(playerName);
    } catch (const out_of_range &) {
        return nullptr;
    }
}

string QMdmmRoom::playerName(QMdmmPlayer *player) const
{
    QMDMMD(const QMdmmRoom);
    for (auto it = d->players.begin(); it != d->players.end(); ++it) {
        if (it->second == player)
            return it->first;
    }

    return string();
}

vector<QMdmmPlayer *> QMdmmRoom::players() const
{
    QMDMMD(const QMdmmRoom);
    vector<QMdmmPlayer *> res;
    for (auto it = d->players.begin(); it != d->players.end(); ++it)
        res.push_back(it->second);

    return res;
}

vector<string> QMdmmRoom::playerNames() const
{
    QMDMMD(const QMdmmRoom);
    vector<string> res;
    for (auto it = d->players.begin(); it != d->players.end(); ++it)
        res.push_back(it->first);

    return res;
}
