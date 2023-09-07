// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmserver.h"
#include "qmdmmserverplayer.h"
#include "qmdmmserverroom.h"
#include "qmdmmserversocket.h"
#include <algorithm>
#include <map>
#include <thread>
#include <vector>

using std::make_pair;
using std::map;
using std::string;
using std::thread;
using std::vector;

struct QMdmmServerPrivate
{
    map<QMdmmServerRoom *, thread *> roomThreadMap;
    map<QMdmmServerSocket *, QMdmmServerPlayer *> playerMap;
    map<QMdmmServerPlayer *, QMdmmServerRoom *> roomMap;
    map<QMdmmServerSocket *, QMdmmServerPlayer *> observerMap;
    vector<QMdmmServerSocket *> connectedSockets;
    QMdmmServer *server;

    QMdmmServerPrivate(QMdmmServer *server)
        : server(server)
    {
    }

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
            auto *temp = it->second;
            roomThreadMap.erase(it);
            temp->join();
            delete room;
            delete temp;
        }
    }

    void pongClientFunc(QMdmmServerSocket *socket, const Json::Value &value)
    {
        // TODO
        QMDMM_UNUSED(socket);
        QMDMM_UNUSED(value);
    }

    void pingServerFunc(QMdmmServerSocket *socket, const Json::Value &value)
    {
        socket->notify(QMdmmProtocol::NotifyPongServer, value);
    }

    void signInFunc(QMdmmServerSocket *socket, const Json::Value &value)
    {
        // string playerName
        string playerName = value["playerName"].asString();
        server->addPlayer(socket, playerName);
    }

    void marshalFunc(QMdmmServerSocket *socket, const Json::Value &value)
    {
        // string playerName
        string playerName = value["playerName"].asString();
        int connectionId = value["connectionId"].asInt();
        server->reconnectPlayer(socket, playerName, connectionId);
    }

    void observeFunc(QMdmmServerSocket *socket, const Json::Value &value)
    {
        // TODO
        QMDMM_UNUSED(value);
        socket->disconnect();
    }

    typedef void (QMdmmServerPrivate::*NotifyFunc)(QMdmmServerSocket *socket, const Json::Value &value);
    static const map<QMdmmProtocol::QMdmmNotifyId, NotifyFunc> notifyFuncMap;
};

const map<QMdmmProtocol::QMdmmNotifyId, QMdmmServerPrivate::NotifyFunc> QMdmmServerPrivate::notifyFuncMap {
    make_pair(QMdmmProtocol::NotifyPongClient, &QMdmmServerPrivate::pongClientFunc), make_pair(QMdmmProtocol::NotifyPingServer, &QMdmmServerPrivate::pingServerFunc),
    make_pair(QMdmmProtocol::NotifySignIn, &QMdmmServerPrivate::signInFunc), make_pair(QMdmmProtocol::NotifyMarshal, &QMdmmServerPrivate::marshalFunc),
    make_pair(QMdmmProtocol::NotifyObserve, &QMdmmServerPrivate::observeFunc)};

QMdmmServer::QMdmmServer()
    : d(new QMdmmServerPrivate(this))
{
    // QMDMMD(QMdmmServer);
}

QMdmmServer::~QMdmmServer()
{
    delete d;
}

void QMdmmServer::socketConnected(QMdmmServerSocket *socket)
{
    socket->setServer(this);
    d->connectedSockets.push_back(socket);
}

void QMdmmServer::socketDisconnected(QMdmmServerSocket *socket)
{
    auto csIt = std::find(d->connectedSockets.cbegin(), d->connectedSockets.cend(), socket);
    if (csIt != d->connectedSockets.cend()) {
        auto pmIt = d->playerMap.find(socket);
        if (pmIt != d->playerMap.cend()) {
            pmIt->second->setSocket(nullptr);
            // TODO: disconnect when requesting
            // TODO: report disconnection to other players

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

        auto players = room->players();
        Json::Value connectionInfo;
        connectionInfo["playerName"] = playerName;
        for (auto & it : players) {
            QMdmmServerPlayer *sp = dynamic_cast<QMdmmServerPlayer *>(it);
            if (sp == player) {
                Json::Value v = connectionInfo;
                v["connectionId"] = player->connectionId();
                sp->notify(QMdmmProtocol::NotifyConnected, v);
            } else if (sp != nullptr)
                sp->notify(QMdmmProtocol::NotifyConnected, connectionInfo);
        }

        if (room->full())
            d->startGame(room);
    }
}

bool QMdmmServer::reconnectPlayer(QMdmmServerSocket *socket, const string &playerName, int connectionId)
{
    for (auto it = d->roomMap.begin(); it != d->roomMap.end(); ++it) {
        if (it->first->connectionId() == connectionId && it->second->player(playerName) == it->first) {
            it->first->setSocket(socket);
            d->playerMap[socket] = it->first;

            auto players = it->second->players();
            Json::Value connectionInfo;
            connectionInfo["playerName"] = playerName;
            connectionInfo["isReconnect"] = true;
            for (auto & player : players) {
                QMdmmServerPlayer *sp = dynamic_cast<QMdmmServerPlayer *>(player);
                if (sp == it->first) {
                    Json::Value v = connectionInfo;
                    v["connectionId"] = it->first->connectionId();
                    sp->notify(QMdmmProtocol::NotifyConnected, v);
                } else if (sp != nullptr)
                    sp->notify(QMdmmProtocol::NotifyConnected, connectionInfo);
            }

            return true;
        }
    }

    return false;
}

void QMdmmServer::notifyServer(QMdmmServerSocket *socket, QMdmmProtocol::QMdmmNotifyId notifyId, const Json::Value &notifyData)
{
    if ((notifyId & QMdmmProtocol::NotifyToRoomMask) != 0) {
        auto playerIt = d->playerMap.find(socket);
        if (playerIt != d->playerMap.cend()) {
            auto roomIt = d->roomMap.find(playerIt->second);
            if (roomIt != d->roomMap.cend())
                roomIt->second->notified(playerIt->second, notifyId, notifyData);
        }
    } else if ((notifyId & QMdmmProtocol::NotifyToServerMask) != 0) {
        // process notify
        auto funcit = QMdmmServerPrivate::notifyFuncMap.find(notifyId);
        if (funcit != QMdmmServerPrivate::notifyFuncMap.cend()) {
            auto func = funcit->second;
            if (func != nullptr)
                (d->*func)(socket, notifyData);
        }
    }
}

void QMdmmServer::replyToServer(QMdmmServerSocket *socket, QMdmmProtocol::QMdmmRequestId requestId, const Json::Value &replyData)
{
    auto playerIt = d->playerMap.find(socket);
    if (playerIt != d->playerMap.cend()) {
        auto roomIt = d->roomMap.find(playerIt->second);
        if (roomIt != d->roomMap.cend())
            roomIt->second->replyed(playerIt->second, requestId, replyData);
    }
}
