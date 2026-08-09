// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMGUI_GAMECLIENT_H_
#define QMDMMGUI_GAMECLIENT_H_

#include <QObject>
#include <QVariantList>
#include <QStringList>
#include <QHash>

#include <QMdmmClient>
#include <QMdmmRoom>
#include <QMdmmPlayer>
#include <QMdmmServer>

QMDMM_EXPORT_NAME(QMdmmGameClient)

/**
 * @brief Bridge between the QML GUI and the networking / core engine.
 *
 * Owns the human @c Client, optionally an in-process @c Server plus a few
 * auto-replying bot @c Client s (so a single user can fill a room and actually
 * play a full match), and exposes a QML-friendly view of the synchronized
 * @c Room model.
 */
class QMdmmGameClient : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList players READ players NOTIFY playersChanged)
    Q_PROPERTY(QString gameState READ gameState NOTIFY gameStateChanged)
    Q_PROPERTY(QString localName READ localName NOTIFY localNameChanged)
    Q_PROPERTY(QVariantList chatLog READ chatLog NOTIFY chatLogChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(int playerCount READ playerCount WRITE setPlayerCount NOTIFY playerCountChanged)

public:
    enum class GameState : uint8_t
    {
        Start,
        Lobby,
        Playing,
        GameOver,
    };
    Q_ENUM(GameState)

    explicit QMdmmGameClient(QObject *parent = nullptr);
    ~QMdmmGameClient() override;

    QVariantList players() const;
    QString gameState() const;
    QString localName() const;
    QVariantList chatLog() const;
    QString statusMessage() const;
    int playerCount() const;
    void setPlayerCount(int n);

    Q_INVOKABLE void startLocalGame(const QString &playerName);
    Q_INVOKABLE void connectOnline(const QString &host, const QString &playerName);
    Q_INVOKABLE void disconnectAll();

    // Request replies (for the human player)
    Q_INVOKABLE void replySsc(int ssc);
    Q_INVOKABLE void replyActionOrder(const QVariantList &orders);
    Q_INVOKABLE void replyAction(int action, const QString &toPlayer, int toPlace);
    Q_INVOKABLE void replyUpgrade(const QVariantList &items);
    Q_INVOKABLE void speak(const QString &text);

    // Helpers for the action / upgrade UI
    Q_INVOKABLE QVariantList getActionOptions() const;
    Q_INVOKABLE QVariantList getUpgradeOptions() const;

    // Display-name lookups (the Room model only stores the internal player name)
    Q_INVOKABLE QString screenName(const QString &playerName) const;
    Q_INVOKABLE bool isYou(const QString &playerName) const;
    Q_INVOKABLE QString placeName(int place) const;

signals:
    void playersChanged();
    void gameStateChanged();
    void localNameChanged();
    void chatLogChanged();
    void statusMessageChanged(const QString &);
    void playerCountChanged();

    void requestStoneScissorsCloth(const QStringList &playerNames, int strivedOrder);
    void requestActionOrder(const QList<int> &remainedOrders, int maximumOrder, int selectionNum);
    void requestAction(int currentOrder);
    void requestUpgrade(int remainingTimes);

    void playerAdded(const QString &playerName, const QString &screenName, int agentState);
    void playerRemoved(const QString &playerName);
    void gameStart();
    void roundStart();
    void roundOver();
    void sscResult(const QVariantMap &results);
    void actionOrderResult(const QVariantMap &result);
    void actionResult(const QString &playerName, int action, const QString &toPlayer, int toPlace);
    void upgradeResult(const QVariantMap &upgrades);
    void gameOver(const QStringList &winners);
    void spoken(const QString &playerName, const QString &content);
    void errorOccurred(const QString &msg);

private:
    void wireClient(QMdmmNetworking::Client *client);
    void addBot(const QString &name);
    void reset();
    void setGameState(GameState s);
    void setStatusMessage(const QString &msg);
    QMdmmCore::Player *localPlayer() const;
    QVariantList actionListFor(const QMdmmCore::Player *from) const;

    QMdmmNetworking::Client *m_human = nullptr;
    QMdmmNetworking::Server *m_server = nullptr;
    QList<QMdmmNetworking::Client *> m_bots;
    QMdmmCore::Room *m_room = nullptr;

    QString m_localName;
    QString m_localScreen;
    GameState m_state = GameState::Start;
    QVariantList m_chat;
    QString m_status;
    int m_playerCount = 3;
    bool m_online = false;

    QHash<QString, QString> m_screenNames;
    QHash<QString, QMdmmCore::Data::AgentState> m_agentStates;
};

#endif // QMDMMGUI_GAMECLIENT_H_
