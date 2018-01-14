#include "qmdmmserver.h"
#include "qmdmmserverplayer.h"
#include "qmdmmserverroom.h"
#include "qmdmmserversocket.h"
#include <algorithm>
#include <map>
#include <thread>
#include <vector>

using std::map;
using std::thread;
using std::vector;

struct QMdmmServerPrivate
{
    QMdmmServerPrivate()
    {
    }

    map<QMdmmServerRoom *, thread *> roomThreadMap;
    map<QMdmmServerSocket *, QMdmmServerPlayer *> playerMap;
    map<QMdmmServerPlayer *, QMdmmServerRoom *> roomMap;
    map<QMdmmServerSocket *, QMdmmServerPlayer *> observerMap;
    vector<QMdmmServerSocket *> connectedSockets;

    void startGame(QMdmmServerRoom *room)
    {
        auto it = roomThreadMap.find(room);
        if (it == roomThreadMap.cend())
            roomThreadMap[room] = new thread([room]() { room->run(); });
    }

    void stopGame(QMdmmServerRoom *room)
    {
        auto it = roomThreadMap.find(room);
        if (it != roomThreadMap.cend()) {
            auto temp = it->second;
            roomThreadMap.erase(it);
            temp->join();
            delete room;
            delete temp;
        }
    }
};

QMdmmServer::QMdmmServer()
    : d_ptr(new QMdmmServerPrivate)
{
    // QMDMMD(QMdmmServer);
}

QMdmmServer::~QMdmmServer()
{
    QMDMMD(QMdmmServer);
    delete d;
}

void QMdmmServer::socketConnected(QMdmmServerSocket *socket)
{
    QMDMMD(QMdmmServer);
    d->connectedSockets.push_back(socket);
}

void QMdmmServer::socketDisconnected(QMdmmServerSocket *socket)
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

void QMdmmServer::addPlayer(QMdmmServerSocket *socket, const string &playerName)
{
    QMDMMD(QMdmmServer);
    auto csIt = std::find(d->connectedSockets.cbegin(), d->connectedSockets.cend(), socket);
    if (csIt != d->connectedSockets.cend()) {
        QMdmmServerPlayer *player = new QMdmmServerPlayer;
        player->setSocket(socket);
        QMdmmServerRoom *room = nullptr;

        for (auto it = d->roomThreadMap.begin(); it != d->roomThreadMap.end(); ++it) {
            if (!it->first->full()) {
                room = it->first;
                break;
            }
        }

        if (room == nullptr) {
            room = new QMdmmServerRoom;
            d->roomThreadMap[room] = nullptr;
        }

        room->addPlayer(player, playerName);
        d->playerMap[socket] = player;
        d->roomMap[player] = room;
        if (room->full())
            d->startGame(room);
    }
}

bool QMdmmServer::reconnectPlayer(QMdmmServerSocket *socket, int connectId)
{
    QMDMMD(QMdmmServer);

    for (auto it = d->roomMap.begin(); it != d->roomMap.end(); ++it) {
        if (it->first->connectId() == connectId) {
            it->first->setSocket(socket);
            d->playerMap[socket] = it->first;
            return true;
        }
    }

    return false;
}

void QMdmmServer::notifyServer(QMdmmServerSocket *socket, QMdmmProtocol::QMdmmNotifyId notifyId, const string &notifyData)
{
}

void QMdmmServer::replyToServer(QMdmmServerSocket *socket, QMdmmProtocol::QMdmmRequestId requestId, const string &replyData)
{
    QMDMMD(QMdmmServer);

    auto playerIt = d->playerMap.find(socket);
    if (playerIt != d->playerMap.cend()) {
        auto roomIt = d->roomMap.find(playerIt->second);
        if (roomIt != d->roomMap.cend())
            roomIt->second->replyed(playerIt->second, requestId, replyData);
    }
}
