#include "qmdmmserverroom.h"
#include "qmdmmserverplayer.h"
#include "qmdmmserversocket.h"
#include <QMdmmCore/QMdmmStoneScissorsCloth>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <vector>

using std::chrono::duration;
using std::chrono::seconds;
using std::chrono::system_clock;
using std::chrono::time_point;
using std::condition_variable;
using std::function;
using std::map;
using std::mutex;
using std::string;
using std::unique_lock;
using std::vector;

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
    map<QMdmmServerPlayer *, string> replys;

    QMdmmServerRoom *parent;

    QMdmmServerRoomPrivate();

    void doRequest(vector<QMdmmServerPlayer *> players, QMdmmProtocol::QMdmmRequestId requestId, const string &requestData);
    void waitForReply(function<bool()> checkReplyFunc);
    void doNotify(vector<QMdmmServerPlayer *> players, QMdmmProtocol::QMdmmNotifyId notifyId, const string &notifyData);

    vector<QMdmmServerPlayer *> stoneScissorsCloth();
};

QMdmmServerRoomPrivate::QMdmmServerRoomPrivate()
    : nowRequestId(QMdmmProtocol::RequestInvalid)
    , parent(nullptr)
{
}

void QMdmmServerRoomPrivate::doRequest(vector<QMdmmServerPlayer *> players, QMdmmProtocol::QMdmmRequestId requestId, const string &requestData)
{
    replys.clear();
    for (QMdmmServerPlayer *player : players)
        player->request(requestId, requestData);

    nowRequestId = requestId;
}

void QMdmmServerRoomPrivate::waitForReply(function<bool()> checkReplyFunc)
{
    unique_lock<mutex> uniqueLock(condMutex);

#ifdef HASTIMEOUT
    time_point nowTime = system_clock::now();
    time_point timeoutTime = now + seconds(2 * timeout);
    if (!cond.wait_until(uniqueLock, timeoutTime, checkReplyFunc)) {
        // timeout
    }
#else
    cond.wait(uniqueLock, checkReplyFunc);
#endif
}

void QMdmmServerRoomPrivate::doNotify(vector<QMdmmServerPlayer *> players, QMdmmProtocol::QMdmmNotifyId notifyId, const string &notifyData)
{
    for (QMdmmServerPlayer *player : players)
        player->notify(notifyId, notifyData);
}

vector<QMdmmServerPlayer *> QMdmmServerRoomPrivate::stoneScissorsCloth()
{
    vector<QMdmmServerPlayer *> ret;

    if (parent == nullptr)
        return ret;

    vector<QMdmmServerPlayer *> players;
    for (QMdmmPlayer *p : parent->players()) {
        QMdmmServerPlayer *sp = dynamic_cast<QMdmmServerPlayer *>(p);
        if (sp != nullptr && sp->alive())
            players.push_back(sp);
    }

    if (players.empty())
        return ret;

    doRequest(players, QMdmmProtocol::RequestStoneScissorsCloth, string());
    waitForReply([this, players]() -> bool { return replys.size() == players.size(); });

    vector<QMdmmStoneScissorsCloth> sscs;
    for (vector<QMdmmServerPlayer *>::size_type i = 0; i < players.size(); ++i) {
        QMdmmServerPlayer *player = players.at(i);
        int n = atoi(replys[player].c_str());
        sscs.push_back(static_cast<QMdmmData::StoneScissorsCloth>(n));
    }

    auto winners = QMdmmStoneScissorsCloth::winners(sscs);

    for (auto i : winners)
        ret.push_back(players.at(i));

    return ret;
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

void QMdmmServerRoom::notified(QMdmmServerPlayer *player, QMdmmProtocol::QMdmmNotifyId notifyId, const string &notifyData)
{
    // only speak data can be notified to room?
    // consider putting this logic to QMdmmServer
}

void QMdmmServerRoom::replyed(QMdmmServerPlayer *player, QMdmmProtocol::QMdmmRequestId requestId, const string &notifyData)
{
    QMDMMD(QMdmmServerRoom);

    unique_lock<mutex> uniqueLock(d->condMutex);
    QMDMM_UNUSED(uniqueLock);

    if (d->nowRequestId == requestId) {
        d->replys[player] = notifyData;
    } else {
        d->replys[player] = string();
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
