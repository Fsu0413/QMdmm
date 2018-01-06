#include "qmdmmserver.h"
#include "qmdmmserverplayer.h"
#include "qmdmmserverroom.h"
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
        : room(nullptr)
        , roomThread(nullptr)
    {
    }

    QMdmmServerRoom *room;
    thread *roomThread;
    map<QMdmmServerSocket *, QMdmmServerPlayer *> playerMap;
    map<QMdmmServerSocket *, QMdmmServerPlayer *> observerMap;
    vector<QMdmmServerSocket *> connectedSockets;

    void startGame()
    {
        if (roomThread != nullptr)
            roomThread = new thread([this]() { room->run(); });
    }

    void stopGame()
    {
        if (roomThread != nullptr) {
            roomThread->join();
            delete roomThread;
        }
    }
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
    if (!d->room->full()) {
        auto csIt = std::find(d->connectedSockets.cbegin(), d->connectedSockets.cend(), socket);
        if (csIt != d->connectedSockets.cend()) {
            QMdmmServerPlayer *player = new QMdmmServerPlayer;
            player->setSocket(socket);
            d->room->addPlayer(player, playerName);
            d->playerMap[socket] = player;

            if (d->room->full())
                d->startGame();
        }
    }
}

bool QMdmmServer::reconnectPlayer(QMdmmServerSocket *socket, int connectId, const string &playerName)
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

void QMdmmServer::notifyServer(QMdmmServerSocket *socket, QMdmmProtocol::QMdmmNotifyId notifyId, const string &notifyData)
{
}
