// SPDX-License-Identifier: AGPL-3.0-or-later

#include "gameclient.h"

#include <QJsonObject>
#include <QRandomGenerator>
#include <QVariantMap>

#include <QMdmmLogicConfiguration>

using namespace QMdmmCore;
using namespace QMdmmNetworking;

namespace {
constexpr char LOCAL_HOST[] = "qmdmm://localhost:6366";
} // namespace

QMdmmGameClient::QMdmmGameClient(QObject *parent)
    : QObject(parent)
{
}

QMdmmGameClient::~QMdmmGameClient()
{
    reset();
}

void QMdmmGameClient::reset()
{
    qDeleteAll(m_bots);
    m_bots.clear();

    delete m_human;
    m_human = nullptr;

    delete m_server;
    m_server = nullptr;

    m_room = nullptr;
    m_localName.clear();
    m_localScreen.clear();
    m_screenNames.clear();
    m_agentStates.clear();
    m_chat.clear();

    emit chatLogChanged();
    emit playersChanged();
}

QVariantList QMdmmGameClient::players() const
{
    QVariantList ret;
    if (m_room == nullptr)
        return ret;
    const QList<Player *> ps = m_room->players();
    ret.reserve(ps.size());
    for (Player *p : ps)
        ret.append(QVariant::fromValue(static_cast<QObject *>(p)));
    return ret;
}

QString QMdmmGameClient::gameState() const
{
    switch (m_state) {
    case GameState::Start:
        return QStringLiteral("start");
    case GameState::Lobby:
        return QStringLiteral("lobby");
    case GameState::Playing:
        return QStringLiteral("playing");
    case GameState::GameOver:
        return QStringLiteral("gameover");
    }
    return QStringLiteral("start");
}

QString QMdmmGameClient::localName() const
{
    return m_localName;
}

QVariantList QMdmmGameClient::chatLog() const
{
    return m_chat;
}

QString QMdmmGameClient::statusMessage() const
{
    return m_status;
}

int QMdmmGameClient::playerCount() const
{
    return m_playerCount;
}

void QMdmmGameClient::setPlayerCount(int n)
{
    n = qBound(1, n, 6);
    if (n == m_playerCount)
        return;
    m_playerCount = n;
    emit playerCountChanged();
}

void QMdmmGameClient::setGameState(GameState s)
{
    if (s == m_state)
        return;
    m_state = s;
    emit gameStateChanged();
}

void QMdmmGameClient::setStatusMessage(const QString &msg)
{
    if (msg == m_status)
        return;
    m_status = msg;
    emit statusMessageChanged(msg);
}

QMdmmCore::Player *QMdmmGameClient::localPlayer() const
{
    if (m_room == nullptr || m_localName.isEmpty())
        return nullptr;
    return m_room->player(m_localName);
}

QString QMdmmGameClient::screenName(const QString &playerName) const
{
    return m_screenNames.value(playerName, playerName);
}

bool QMdmmGameClient::isYou(const QString &playerName) const
{
    return playerName == m_localName;
}

QString QMdmmGameClient::placeName(int place) const
{
    if (place == Data::Country)
        return QStringLiteral("国家");
    return QStringLiteral("城市 %1").arg(place);
}

void QMdmmGameClient::wireClient(Client *client)
{
    // request signals -> re-emit for QML
    connect(client, &Client::requestStoneScissorsCloth, this,
            [this](const QStringList &playerNames, int strivedOrder) {
                emit requestStoneScissorsCloth(playerNames, strivedOrder);
            });
    connect(client, &Client::requestActionOrder, this,
            [this](const QList<int> &remainedOrders, int maximumOrder, int selectionNum) {
                emit requestActionOrder(remainedOrders, maximumOrder, selectionNum);
            });
    connect(client, &Client::requestAction, this, [this](int currentOrder) {
        emit requestAction(currentOrder);
    });
    connect(client, &Client::requestUpgrade, this, [this](int remainingTimes) {
        emit requestUpgrade(remainingTimes);
    });

    // notify signals -> re-emit (and keep the local view in sync)
    connect(client, &Client::notifyPlayerAdded, this,
            [this](const QString &playerName, const QString &screenName, const Data::AgentState &agentState) {
                m_screenNames.insert(playerName, screenName);
                m_agentStates.insert(playerName, agentState);
                emit playerAdded(playerName, screenName, static_cast<int>(agentState));
                emit playersChanged();
            });
    connect(client, &Client::notifyPlayerRemoved, this, [this](const QString &playerName) {
        m_screenNames.remove(playerName);
        m_agentStates.remove(playerName);
        emit playerRemoved(playerName);
        emit playersChanged();
    });
    connect(client, &Client::notifyGameStart, this, [this]() {
        setGameState(GameState::Playing);
        emit gameStart();
    });
    connect(client, &Client::notifyRoundStart, this, [this]() {
        emit roundStart();
    });
    connect(client, &Client::notifyRoundOver, this, [this]() {
        emit roundOver();
    });
    connect(client, &Client::notifyStoneScissorsCloth, this,
            [this](const QHash<QString, Data::StoneScissorsCloth> &replies) {
                QVariantMap m;
                for (auto it = replies.constBegin(); it != replies.constEnd(); ++it)
                    m.insert(it.key(), static_cast<int>(it.value()));
                emit sscResult(m);
            });
    connect(client, &Client::notifyActionOrder, this, [this](const QHash<int, QString> &result) {
        QVariantMap m;
        for (auto it = result.constBegin(); it != result.constEnd(); ++it)
            m.insert(QString::number(it.key()), it.value());
        emit actionOrderResult(m);
    });
    connect(client, &Client::notifyAction, this,
            [this](const QString &playerName, Data::Action action, const QString &toPlayer, int toPlace) {
                emit actionResult(playerName, static_cast<int>(action), toPlayer, toPlace);
            });
    connect(client, &Client::notifyUpgrade, this,
            [this](const QHash<QString, QList<Data::UpgradeItem>> &upgrades) {
                QVariantMap m;
                for (auto it = upgrades.constBegin(); it != upgrades.constEnd(); ++it) {
                    QVariantList l;
                    l.reserve(it.value().size());
                    for (Data::UpgradeItem u : it.value())
                        l.append(static_cast<int>(u));
                    m.insert(it.key(), l);
                }
                emit upgradeResult(m);
            });
    connect(client, &Client::notifyGameOver, this, [this](const QStringList &winners) {
        setGameState(GameState::GameOver);
        emit gameOver(winners);
    });
    connect(client, &Client::notifySpoken, this, [this](const QString &playerName, const QString &content) {
        QVariantMap entry;
        entry.insert(QStringLiteral("name"), playerName);
        entry.insert(QStringLiteral("screen"), screenName(playerName));
        entry.insert(QStringLiteral("content"), content);
        m_chat.append(entry);
        emit chatLogChanged();
    });
    connect(client, &Client::socketErrorDisconnected, this, [this](const QString &errorString) {
        setStatusMessage(errorString);
        emit errorOccurred(errorString);
    });
}

void QMdmmGameClient::addBot(const QString &name)
{
    ClientConfiguration cfg;
    cfg.setScreenName(name);
    auto *bot = new Client(cfg, this);
    bot->setObjectName(name);

    // Auto-reply: mirror the server's default-reply behaviour so the room fills
    // and the match progresses without a human driving the bot.
    connect(bot, &Client::requestStoneScissorsCloth, bot, [bot]() {
        bot->replyStoneScissorsCloth(static_cast<Data::StoneScissorsCloth>(QRandomGenerator::global()->generate() % 3));
    });
    connect(bot, &Client::requestActionOrder, bot,
            [bot](const QList<int> &remainedOrders, int, int selectionNum) {
                QList<int> ao;
                ao.reserve(selectionNum);
                for (int i = 0; i < selectionNum && i < remainedOrders.size(); ++i)
                    ao.append(remainedOrders.at(i));
                bot->replyActionOrder(ao);
            });
    connect(bot, &Client::requestAction, bot, [bot]() {
        bot->replyAction(Data::DoNothing, {}, 0);
    });
    connect(bot, &Client::requestUpgrade, bot, [bot](int remainingTimes) {
        QList<Data::UpgradeItem> ups;
        ups.reserve(remainingTimes);
        for (int i = 0; i < remainingTimes; ++i)
            ups.append(Data::UpgradeMaxHp);
        bot->replyUpgrade(ups);
    });

    bot->connectToHost(QString::fromLatin1(LOCAL_HOST), Data::StateOnlineBot);
    m_bots.append(bot);
}

void QMdmmGameClient::startLocalGame(const QString &playerName)
{
    reset();

    // In-process server so a single user can actually play a full match.
    LogicConfiguration conf = LogicConfiguration::defaults();
    conf.setPlayerNumPerRoom(m_playerCount);
    m_server = new Server(ServerConfiguration::defaults(), conf, this);
    if (!m_server->listen()) {
        setStatusMessage(QStringLiteral("无法启动本地服务器"));
        delete m_server;
        m_server = nullptr;
        return;
    }

    ClientConfiguration hc;
    hc.setScreenName(playerName.isEmpty() ? QStringLiteral("You") : playerName);
    m_human = new Client(hc, this);
    m_localName = m_human->objectName();
    m_localScreen = hc.screenName();
    m_room = m_human->room();
    wireClient(m_human);
    m_human->connectToHost(QString::fromLatin1(LOCAL_HOST), Data::StateOnline);

    for (int i = 1; i < m_playerCount; ++i)
        addBot(QStringLiteral("Bot %1").arg(i));

    emit localNameChanged();
    setGameState(GameState::Lobby);
    setStatusMessage(QStringLiteral("已连接到本地服务器，等待其他玩家加入…"));
}

void QMdmmGameClient::connectOnline(const QString &host, const QString &playerName)
{
    reset();

    ClientConfiguration hc;
    hc.setScreenName(playerName.isEmpty() ? QStringLiteral("You") : playerName);
    m_human = new Client(hc, this);
    m_localName = m_human->objectName();
    m_localScreen = hc.screenName();
    m_room = m_human->room();
    wireClient(m_human);

    QString addr = host.trimmed();
    if (!addr.contains(QLatin1String("://")))
        addr = QStringLiteral("qmdmm://") + addr;
    m_human->connectToHost(addr, Data::StateOnline);

    emit localNameChanged();
    setGameState(GameState::Lobby);
    setStatusMessage(QStringLiteral("正在连接服务器…"));
}

void QMdmmGameClient::disconnectAll()
{
    reset();
    setGameState(GameState::Start);
    setStatusMessage(QStringLiteral("已断开连接"));
}

void QMdmmGameClient::replySsc(int ssc)
{
    if (m_human)
        m_human->replyStoneScissorsCloth(static_cast<Data::StoneScissorsCloth>(ssc));
}

void QMdmmGameClient::replyActionOrder(const QVariantList &orders)
{
    if (!m_human)
        return;
    QList<int> ao;
    ao.reserve(orders.size());
    for (const QVariant &v : orders)
        ao.append(v.toInt());
    m_human->replyActionOrder(ao);
}

void QMdmmGameClient::replyAction(int action, const QString &toPlayer, int toPlace)
{
    if (m_human)
        m_human->replyAction(static_cast<Data::Action>(action), toPlayer, toPlace);
}

void QMdmmGameClient::replyUpgrade(const QVariantList &items)
{
    if (!m_human)
        return;
    QList<Data::UpgradeItem> ups;
    ups.reserve(items.size());
    for (const QVariant &v : items)
        ups.append(static_cast<Data::UpgradeItem>(v.toInt()));
    m_human->replyUpgrade(ups);
}

void QMdmmGameClient::speak(const QString &text)
{
    if (m_human && !text.isEmpty())
        m_human->notifySpeak(text);
}

QVariantList QMdmmGameClient::actionListFor(const Player *from) const
{
    QVariantList ret;
    if (from == nullptr || m_room == nullptr)
        return ret;

    const auto make = [](Data::Action a, const QString &label, const QString &target, int place) {
        QVariantMap m;
        m.insert(QStringLiteral("action"), static_cast<int>(a));
        m.insert(QStringLiteral("label"), label);
        m.insert(QStringLiteral("target"), target);
        m.insert(QStringLiteral("place"), place);
        return m;
    };

    if (from->alive())
        ret.append(make(Data::DoNothing, QStringLiteral("不动 / 歇着"), QString(), -1));
    if (from->canBuyKnife())
        ret.append(make(Data::BuyKnife, QStringLiteral("买刀"), QString(), -1));
    if (from->canBuyHorse())
        ret.append(make(Data::BuyHorse, QStringLiteral("买马"), QString(), -1));

    const int here = from->place();
    // Move to any adjacent place (Country <-> one city).
    for (int to = 0; to <= m_playerCount; ++to) {
        if (to == here)
            continue;
        if (Data::isPlaceAdjacent(here, to) && from->canMove(to))
            ret.append(make(Data::Move, QStringLiteral("移动到 %1").arg(placeName(to)), QString(), to));
    }

    for (const Player *other : m_room->players()) {
        if (other == from || !other->alive())
            continue;
        const QString screen = screenName(other->objectName());
        if (from->canSlash(other))
            ret.append(make(Data::Slash, QStringLiteral("砍 %1").arg(screen), other->objectName(), -1));
        if (from->canKick(other))
            ret.append(make(Data::Kick, QStringLiteral("踢 %1").arg(screen), other->objectName(), -1));
        for (int to = 0; to <= m_playerCount; ++to) {
            if (to == other->place())
                continue;
            if (Data::isPlaceAdjacent(other->place(), to) && from->canLetMove(other, to))
                ret.append(make(Data::LetMove, QStringLiteral("拉 %1 到 %2").arg(screen, placeName(to)),
                               other->objectName(), to));
        }
    }
    return ret;
}

QVariantList QMdmmGameClient::getActionOptions() const
{
    return actionListFor(localPlayer());
}

QVariantList QMdmmGameClient::getUpgradeOptions() const
{
    QVariantList ret;
    const Player *p = localPlayer();
    if (p == nullptr)
        return ret;

    auto add = [&](Data::UpgradeItem item, const QString &label) {
        QVariantMap m;
        m.insert(QStringLiteral("item"), static_cast<int>(item));
        m.insert(QStringLiteral("label"), label);
        ret.append(m);
    };
    if (p->canUpgradeKnife())
        add(Data::UpgradeKnife, QStringLiteral("升级刀伤"));
    if (p->canUpgradeHorse())
        add(Data::UpgradeHorse, QStringLiteral("升级马伤"));
    if (p->canUpgradeMaxHp())
        add(Data::UpgradeMaxHp, QStringLiteral("升级最大生命"));
    return ret;
}
