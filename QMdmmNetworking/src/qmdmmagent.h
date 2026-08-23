// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMAGENT_H
#define QMDMMAGENT_H

#include "qmdmmnetworkingglobal.h"

#include <QMdmmData>
#include <QMdmmProtocol>

#include <QHash>
#include <QJsonValue>
#include <QList>
#include <QObject>

QMDMM_EXPORT_NAME(QMdmmAgent)

namespace QMdmmNetworking {

#ifndef DOXYGEN
namespace p {
struct AgentP;
}
#endif

#ifndef DOXYGEN
namespace v0 {
#endif

class Socket;

class QMDMMNETWORKING_EXPORT Agent final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString screenName READ screenName WRITE setScreenName NOTIFY screenNameChanged FINAL)
    Q_PROPERTY(QMdmmCore::Data::AgentState state READ state WRITE setState NOTIFY stateChanged FINAL)

public:
    Q_DISABLE_COPY_MOVE(Agent);

    explicit Agent(const QString &name, QObject *parent = nullptr);
    ~Agent() override;

    // properties
    [[nodiscard]] QString screenName() const;
    void setScreenName(const QString &name);

    [[nodiscard]] QMdmmCore::Data::AgentState state() const;
    void setState(const QMdmmCore::Data::AgentState &state);

    // Controller interface — notifications (logic side → operation side).
    // Each method is the entry point the logic side (LogicRunner / Client) calls to notify the
    // player; it forwards the notification as the corresponding xxxNotified signal, which the
    // operation side (ServerConnection / GUI / Bot) listens to.
    void notifyLogicConfiguration();
    void notifyAgentStateChange(const QString &playerName, const QMdmmCore::Data::AgentState &agentState);
    void notifyPlayerAdd(const QString &playerName, const QString &screenName, const QMdmmCore::Data::AgentState &agentState);
    void notifyPlayerRemove(const QString &playerName);
    void notifyGameStart();
    void notifyRoundStart();
    void notifyStoneScissorsCloth(const QHash<QString, QMdmmCore::Data::StoneScissorsCloth> &replies);
    void notifyActionOrder(const QHash<int, QString> &result);
    void notifyAction(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace);
    void notifyRoundOver();
    void notifyUpgrade(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades);
    void notifyGameOver(const QStringList &playerNames);
    void notifySpeak(const QString &playerName, const QString &content);
    void notifyOperate(const QString &playerName, const QJsonValue &todo);

    // Controller interface — requests (logic side → operation side).
    // Each method is the entry point the logic side (LogicRunner) calls to ask the player for
    // input; it forwards the request as the corresponding xxxRequested signal, which the
    // operation side (ServerConnection / GUI / Bot) listens to.
    //
    // Reply contract: the operation side must answer *asynchronously* (e.g. via QTimer::singleShot
    // or after an event-loop round-trip), never synchronously from inside the xxxRequested
    // handler. A synchronous reply re-enters the request handler and violates the designed
    // async request/reply flow (see ServerConnection::addRequest for the socket-less default).
    void requestStoneScissorsCloth(const QStringList &playerNames, int strivedOrder);
    void requestActionOrder(const QList<int> &remainedOrders, int maximumOrder, int selectionNum);
    void requestAction(int currentOrder);
    void requestUpgrade(int remainingTimes);

    // Controller interface — replies and player actions (operation side → logic side).
    // Each bare-verb method is the entry point the operation side calls (ServerConnection after
    // decoding a wire reply, or GUI / Bot directly); it forwards the reply as the corresponding
    // replyXxx signal, which the logic side (LogicRunner) listens to.
    void stoneScissorsCloth(QMdmmCore::Data::StoneScissorsCloth ssc);
    void actionOrder(const QList<int> &order);
    void action(QMdmmCore::Data::Action act, const QString &toPlayer, int toPlace);
    void upgrade(const QList<QMdmmCore::Data::UpgradeItem> &items);

    // Controller interface — the player's own speech / operation, forwarded to the logic side as
    // the spoken / operated signals. These are the operation-side counterparts of notifySpeak /
    // notifyOperate (which notify the player about *others*).
    void speak(const QString &content);
    void operate(const QJsonValue &todo);

signals:
    void screenNameChanged(const QString &, QPrivateSignal);
    void stateChanged(QMdmmCore::Data::AgentState, QPrivateSignal);

    // Controller interface — notifications forwarded to the operation side.
    void logicConfigurationNotified(QPrivateSignal);
    void agentStateChangeNotified(const QString &playerName, const QMdmmCore::Data::AgentState &agentState, QPrivateSignal);
    void playerAddNotified(const QString &playerName, const QString &screenName, const QMdmmCore::Data::AgentState &agentState, QPrivateSignal);
    void playerRemoveNotified(const QString &playerName, QPrivateSignal);
    void gameStartNotified(QPrivateSignal);
    void roundStartNotified(QPrivateSignal);
    void stoneScissorsClothNotified(const QHash<QString, QMdmmCore::Data::StoneScissorsCloth> &replies, QPrivateSignal);
    void actionOrderNotified(const QHash<int, QString> &result, QPrivateSignal);
    void actionNotified(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace, QPrivateSignal);
    void roundOverNotified(QPrivateSignal);
    void upgradeNotified(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades, QPrivateSignal);
    void gameOverNotified(const QStringList &playerNames, QPrivateSignal);
    void speakNotified(const QString &playerName, const QString &content, QPrivateSignal);
    void operateNotified(const QString &playerName, const QJsonValue &todo, QPrivateSignal);

    // Controller interface — requests forwarded to the operation side.
    void stoneScissorsClothRequested(const QStringList &playerNames, int strivedOrder, QPrivateSignal);
    void actionOrderRequested(const QList<int> &remainedOrders, int maximumOrder, int selectionNum, QPrivateSignal);
    void actionRequested(int currentOrder, QPrivateSignal);
    void upgradeRequested(int remainingTimes, QPrivateSignal);

    // Controller interface — replies forwarded to the logic side.
    void replyStoneScissorsCloth(QMdmmCore::Data::StoneScissorsCloth ssc, QPrivateSignal);
    void replyActionOrder(const QList<int> &order, QPrivateSignal);
    void replyAction(QMdmmCore::Data::Action act, const QString &toPlayer, int toPlace, QPrivateSignal);
    void replyUpgrade(const QList<QMdmmCore::Data::UpgradeItem> &items, QPrivateSignal);

    // Controller interface — the player's own speech / operation forwarded to the logic side.
    void spoken(const QString &content, QPrivateSignal);
    void operated(const QJsonValue &todo, QPrivateSignal);

#ifndef DOXYGEN
private:
    const std::unique_ptr<p::AgentP> d;
#endif
};

#ifndef DOXYGEN
} // namespace v0
inline namespace v1 {
using v0::Agent;
}
#endif
} // namespace QMdmmNetworking

#endif
