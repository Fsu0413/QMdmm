// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmserverroom.h"
#include "qmdmmserverplayer.h"
#include "qmdmmserversocket.h"
#include <QMdmmCore/QMdmmStoneScissorsCloth>
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <map>
#include <mutex>
#include <vector>

using std::condition_variable;
using std::deque;
using std::function;
using std::map;
using std::mutex;
using std::string;
using std::unique_lock;
using std::vector;
using std::chrono::duration;
using std::chrono::seconds;
using std::chrono::system_clock;
using std::chrono::time_point;

QMdmmServerRoom::GameOverType QMdmmServerRoom::RoundOver(QMdmmServerRoom::GameOverType::RoundOver);
QMdmmServerRoom::GameOverType QMdmmServerRoom::GameOver(QMdmmServerRoom::GameOverType::GameOver);
QMdmmServerRoom::GameOverType QMdmmServerRoom::ErrorOver(QMdmmServerRoom::GameOverType::ErrorOver);

struct QMdmmServerRoomPrivate
{
    // only one request can be done at one time.
    // this request can have results from multiple players
    // server timeout is 2 times longer than client timeout
    // note: failsafe must be done here!!!
    QMdmmProtocol::QMdmmRequestId nowRequestId;
    condition_variable cond;
    mutex condMutex;
    map<QMdmmServerPlayer *, Json::Value> replys;

    QMdmmServerRoom *parent;

    QMdmmServerRoomPrivate();

    void doRequest(vector<QMdmmServerPlayer *> players, QMdmmProtocol::QMdmmRequestId requestId, const Json::Value &requestData);
    void doRequest(QMdmmServerPlayer *player, QMdmmProtocol::QMdmmRequestId requestId, const Json::Value &requestData);
    void waitForReply(function<bool()> checkReplyFunc);
    void doNotify(vector<QMdmmServerPlayer *> players, QMdmmProtocol::QMdmmNotifyId notifyId, const Json::Value &notifyData);

    vector<QMdmmServerPlayer *> stoneScissorsCloth(const vector<QMdmmServerPlayer *> &players);
    void sortWinnerOrder(vector<QMdmmServerPlayer *> &winners);
    void operate(const vector<QMdmmServerPlayer *> &winners);

    vector<QMdmmServerPlayer *> serverPlayers() const;
};

QMdmmServerRoomPrivate::QMdmmServerRoomPrivate()
    : nowRequestId(QMdmmProtocol::RequestInvalid)
    , parent(nullptr)
{
}

void QMdmmServerRoomPrivate::doRequest(vector<QMdmmServerPlayer *> players, QMdmmProtocol::QMdmmRequestId requestId, const Json::Value &requestData)
{
    replys.clear();
    nowRequestId = requestId;

    for (QMdmmServerPlayer *player : players)
        player->request(requestId, requestData);
}

void QMdmmServerRoomPrivate::doRequest(QMdmmServerPlayer *player, QMdmmProtocol::QMdmmRequestId requestId, const Json::Value &requestData)
{
    doRequest(vector<QMdmmServerPlayer *> {player}, requestId, requestData);
}

void QMdmmServerRoomPrivate::waitForReply(function<bool()> checkReplyFunc)
{
    unique_lock<mutex> uniqueLock(condMutex);

#ifdef HASTIMEOUT
    time_point nowTime = system_clock::now();
    time_point timeoutTime = now + seconds(2 * timeout);
    if (!cond.wait_until(uniqueLock, timeoutTime, checkReplyFunc)) {
        // timeout, time to disconnect the player who lost the connection then continues
    }
#else
    cond.wait(uniqueLock, checkReplyFunc);
#endif

    nowRequestId = QMdmmProtocol::RequestInvalid;
}

void QMdmmServerRoomPrivate::doNotify(vector<QMdmmServerPlayer *> players, QMdmmProtocol::QMdmmNotifyId notifyId, const Json::Value &notifyData)
{
    for (QMdmmServerPlayer *player : players)
        player->notify(notifyId, notifyData);
}

vector<QMdmmServerPlayer *> QMdmmServerRoomPrivate::stoneScissorsCloth(const vector<QMdmmServerPlayer *> &players)
{
    vector<QMdmmServerPlayer *> ret;

    if (parent == nullptr)
        return ret;

    if (players.empty())
        return ret;

    doRequest(players, QMdmmProtocol::RequestStoneScissorsCloth, Json::Value());
    waitForReply([this, players]() -> bool { return replys.size() == players.size(); });

    vector<QMdmmStoneScissorsCloth> sscs;
    for (vector<QMdmmServerPlayer *>::size_type i = 0; i < players.size(); ++i) {
        QMdmmServerPlayer *player = players.at(i);
        int n = replys.at(player)["ssc"].asInt();
        sscs.push_back(static_cast<QMdmmData::StoneScissorsCloth>(n));
    }

    auto winners = QMdmmStoneScissorsCloth::winners(sscs);

    // notify winner to client
    // TODO: complete the protocol

    for (auto i : winners)
        ret.push_back(players.at(i));

    return ret;
}

void QMdmmServerRoomPrivate::sortWinnerOrder(vector<QMdmmServerPlayer *> &winners)
{
    // number of winners can be 1 or 2
    if (winners.size() == 1)
        return;

    // only one winner?
    if (winners.at(0) == winners.at(1))
        return;

    auto winnersCopy = winners;
    doRequest(winners, QMdmmProtocol::RequestStriveForFirstOrLast, Json::Value());

#if 1
    // if (judgefirst)
    waitForReply([this, winnersCopy]() -> bool { return replys.size() == winnersCopy.size(); });
    // notify the selection to client

#else
    // else if (fight for first)
    // wait for the first result, then wait ONLY for another 1s for avoiding network delay
    waitForReply([this]() -> bool { return !replys.empty(); });

    // notify the selection to client
    QMdmmServerPlayer *p = replys.begin()->first();

    time_point nowTime = system_clock::now();
    time_point timeoutTime = now + seconds(1);
    // NO SLEEP(1) HERE!!!
    while (!cond.wait_until(uniqueLock, timeoutTime, []() -> bool { return replys.size() == winnersCopy.size(); })) {
        // timeout
    }
    // endif
#endif

    winners.clear();

    map<int, QMdmmServerPlayer *> winnerMap;
    for (auto x : replys) {
        int n = x.second["firstOrLast"].asInt();
        auto it = winnerMap.find(n);
        if (it == winnerMap.cend())
            winnerMap[n] = x.first;
        else {
            // now only 2 winners are allowed
            vector<QMdmmServerPlayer *> judgements {x.first, it->second};

            vector<QMdmmServerPlayer *> winners = stoneScissorsCloth(judgements);
            while (winners.empty())
                winners = stoneScissorsCloth(judgements);

            winnerMap[n] = (*winners.begin());
        }
    }

    // fill map

    for (int i = 0; i < 2; ++i) {
        auto it = winnerMap.find(i + 1);
        if (it != winnerMap.cend()) {
            auto winnersCopyIt = std::find(winnersCopy.cbegin(), winnersCopy.cend(), it->second);
            if (winnersCopyIt != winnersCopy.cend())
                winnersCopy.erase(winnersCopyIt);
        }
    }
    if (!winnersCopy.empty()) {
        for (int i = 0; i < 2; ++i) {
            auto it = winnerMap.find(i + 1);
            if (it == winnerMap.cend()) {
                auto winnersCopyIt = winnersCopy.cbegin();
                winnerMap[i + 1] = (*winnersCopyIt);
                winnersCopy.erase(winnersCopyIt);

                if (winnersCopy.empty())
                    break;
            }
        }
    }
    for (int i = 0; i < 2; ++i)
        winners.push_back(winnerMap[i + 1]);
}

void QMdmmServerRoomPrivate::operate(const vector<QMdmmServerPlayer *> &winners)
{
    for (const auto &winner : winners) {
        doRequest(winner, QMdmmProtocol::RequestOperation, Json::Value());
        waitForReply([this]() -> bool { return replys.size() == 1; });

        // choice sanity check: Shall be done again on client
        // Moves Catagary:
        //  Buy:
        //    Buy knife: only available when winner is in city without knife
        //    Buy horse: only available when winner is in city without horse
        //  Move:
        //    Go to country: only available when winner is in city
        //    Go to city No. x (on GUI it may says "Go to the city where Player X at"): only available when winner is in country
        //  Make other players move:
        //    Push sb. to city No. x: only available when both players are in country
        //    Push sb. to country: only available when both players are in same city
        //    Pull sb. to city No. x: only available when winner is in city No. x and another player is in country
        //    Pull sb. to country: only available when winner is in country and another player is in city
        //  Damage:
        //    Slash sb.: only available when both players are in same place. Life punishment will be applied when both players are in city
        //    Kick sb.: only available when both players are in same city
        //  Do nothing:
        //    Zhuangbi; Dese; Xianbai: always available

        Json::Value operationReply = replys.begin()->second;
        string operation = operationReply["operation"].asString();
        if (operation == string("buyknife")) {
            if (!winner->canBuyKnife()) {
                // ??
                continue;
            }

            if (!winner->buyKnife()) {
                // ??
                continue;
            }
        } else if (operation == string("buyhorse")) {
            if (!winner->canBuyHorse()) {
                // ??
                continue;
            }

            if (!winner->buyHorse()) {
                // ??
                continue;
            }
        } else if (operation == string("move")) {
            QMdmmData::Place targetPlace = static_cast<QMdmmData::Place>(operationReply["toPlace"].asInt());
            if (!winner->canMove(targetPlace)) {
                // ??
                continue;
            }

            if (!winner->move(targetPlace)) {
                // ??
                continue;
            }
        } else if (operation == string("letmove")) {
            QMdmmServerPlayer *targetPlayer = dynamic_cast<QMdmmServerPlayer *>(parent->player(operationReply["toPlayer"].asString()));
            if (targetPlayer == nullptr) {
                // ??
                continue;
            }
            QMdmmData::Place targetPlace = static_cast<QMdmmData::Place>(operationReply["toPlace"].asInt());
            if (!winner->canLetMove(targetPlayer, targetPlace)) {
                // ??
                continue;
            }

            if (!winner->letMove(targetPlayer, targetPlace)) {
                // ??
                continue;
            }
        } else if (operation == string("slash")) {
            QMdmmServerPlayer *targetPlayer = dynamic_cast<QMdmmServerPlayer *>(parent->player(operationReply["toPlayer"].asString()));
            if (targetPlayer == nullptr) {
                // ??
                continue;
            }

            if (!winner->canSlash(targetPlayer)) {
                // ??
                continue;
            }

            if (!winner->slash(targetPlayer)) {
                // ??
                continue;
            }
        } else if (operation == string("kick")) {
            QMdmmServerPlayer *targetPlayer = dynamic_cast<QMdmmServerPlayer *>(parent->player(operationReply["toPlayer"].asString()));
            if (targetPlayer == nullptr) {
                // ??
                continue;
            }

            if (!winner->canKick(targetPlayer)) {
                // ??
                continue;
            }

            if (!winner->kick(targetPlayer)) {
                // ??
                continue;
            }
        } else if ((operation == string("zhuangbi")) || (operation == string("dese")) || (operation == string("xianbai"))) {
            if (!winner->doNothing(operation)) {
                // ??
                continue;
            }
        } else {
            // ??
            continue;
        }
        operationReply["fromPlayer"] = winner->name();
        doNotify(serverPlayers(), QMdmmProtocol::NotifyOperation, operationReply);
    }
}

vector<QMdmmServerPlayer *> QMdmmServerRoomPrivate::serverPlayers() const
{
    auto players = parent->players();
    vector<QMdmmServerPlayer *> r;
    for (QMdmmPlayer *player : players) {
        auto splayer = dynamic_cast<QMdmmServerPlayer *>(player);
        if (splayer != nullptr)
            r.push_back(splayer);
    }

    return r;
}

QMdmmServerRoom::QMdmmServerRoom()
    : d_ptr(new QMdmmServerRoomPrivate)
{
    QMDMMD(QMdmmServerRoom);
    d->parent = this;
}

QMdmmServerRoom::~QMdmmServerRoom()
{
    QMDMMD(QMdmmServerRoom);
    delete d;
}

void QMdmmServerRoom::run()
{
    forever {
        try {
            // main run
            // 1. StoneScissorsCloth, continue if tie
            // 2. If multiple players won, deside the order to operate
            // 3. Operate
        } catch (const GameOverType &type) {
            if (type == RoundOver) {
                // update
            } else {
                // Prepare to exit thread
                break;
            }
        }
    }
}

void QMdmmServerRoom::notified(QMdmmServerPlayer *player, QMdmmProtocol::QMdmmNotifyId notifyId, const Json::Value &notifyData)
{
    // only speak and operation can be notified to room?
    // consider putting this logic to QMdmmServer
    QMDMM_UNUSED(player);
    QMDMM_UNUSED(notifyId);
    QMDMM_UNUSED(notifyData);
}

void QMdmmServerRoom::replyed(QMdmmServerPlayer *player, QMdmmProtocol::QMdmmRequestId requestId, const Json::Value &notifyData)
{
    QMDMMD(QMdmmServerRoom);

    unique_lock<mutex> uniqueLock(d->condMutex);
    QMDMM_UNUSED(uniqueLock);

    if (d->nowRequestId == requestId) {
        d->replys[player] = notifyData;
    } else {
        d->replys[player] = Json::Value();
    }

    d->cond.notify_one();
}

constexpr QMdmmServerRoom::GameOverType::GameOverType(const QMdmmServerRoom::GameOverType &other)
    : t(other.t)
{
}

constexpr QMdmmServerRoom::GameOverType::GameOverType(const QMdmmServerRoom::GameOverType::Type type)
    : t(type)
{
}

QMdmmServerRoom::GameOverType &QMdmmServerRoom::GameOverType::operator=(const QMdmmServerRoom::GameOverType &other)
{
    t = other.t;
    return *this;
}

bool QMdmmServerRoom::GameOverType::operator==(const QMdmmServerRoom::GameOverType &other) const
{
    return t == other.t;
}
