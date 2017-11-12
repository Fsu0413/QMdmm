#include "qmdmmserver.h"
#include "qmdmmserverplayer.h"
#include "qmdmmserverroom.h"
#include <algorithm>
#include <map>
#include <vector>

using std::map;
using std::vector;

struct QMdmmServerPrivate
{
    QMdmmServerPrivate()
        : room(nullptr)
    {
    }

    QMdmmServerRoom *room;
    map<QMdmmSocket *, QMdmmServerPlayer *> playerMap;
    map<QMdmmSocket *, QMdmmServerPlayer *> observerMap;
    vector<QMdmmSocket *> connectedSockets;
};

QMdmmServer::QMdmmServer()
    : d_ptr(new QMdmmServerPrivate)
{
    QMDMMD(QMdmmServer);
    d->room = new QMdmmServerRoom;
}

QMdmmServer::~QMdmmServer()
{
    QMDMMD(QMdmmServer);
    delete d->room;
    delete d;
}

QMdmmServerRoom *QMdmmServer::room()
{
    QMDMMD(QMdmmServer);
    return d->room;
}

const QMdmmServerRoom *QMdmmServer::room() const
{
    QMDMMD(const QMdmmServer);
    return d->room;
}

void QMdmmServer::socketConnected(QMdmmSocket *socket)
{
    QMDMMD(QMdmmServer);
    d->connectedSockets.push_back(socket);
}

void QMdmmServer::socketDisconnected(QMdmmSocket *socket)
{
    QMDMMD(QMdmmServer);
    auto csIt = std::find(d->connectedSockets.cbegin(), d->connectedSockets.cend(), socket);
    if (csIt != d->connectedSockets.cend()) {
        auto pmIt = d->playerMap.find(socket);
        if (pmIt != d->playerMap.cend()) {
            pmIt->second->setSocket(nullptr);
            // TODO: disconnect when requesting

            d->playerMap.erase(pmIt);
        }

        auto omIt = d->observerMap.find(socket);
        if (omIt != d->observerMap.cend())
            d->observerMap.erase(omIt);

        d->connectedSockets.erase(csIt);
    }
}

void QMdmmServer::addPlayer(QMdmmSocket *socket, const string &playerName)
{
    QMDMMD(QMdmmServer);
    auto csIt = std::find(d->connectedSockets.cbegin(), d->connectedSockets.cend(), socket);
    if (csIt != d->connectedSockets.cend()) {
        QMdmmServerPlayer *player = new QMdmmServerPlayer;
        player->setSocket(socket);
        d->room->addPlayer(player, playerName);
        d->playerMap[socket] = player;
    }
}

bool QMdmmServer::reconnectPlayer(QMdmmSocket *socket, int connectId, const string &playerName)
{
    QMDMMD(QMdmmServer);
    auto csIt = std::find(d->connectedSockets.cbegin(), d->connectedSockets.cend(), socket);
    if (csIt != d->connectedSockets.cend()) {
        QMdmmServerPlayer *player = dynamic_cast<QMdmmServerPlayer *>(d->room->player(playerName));
        if (player == nullptr)
            return false;
        else if (player->connectId() != connectId)
            return false;
        else {
            player->setSocket(socket);
            d->playerMap[socket] = player;

            // TODO: notify reconnect
            // reconnectNotify(socket);
            return true;
        }
    }

    return false;
}
