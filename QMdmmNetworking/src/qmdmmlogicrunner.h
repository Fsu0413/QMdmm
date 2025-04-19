// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMLOGICRUNNER_H
#define QMDMMLOGICRUNNER_H

#include "qmdmmnetworkingglobal.h"

#include <QMdmmLogicConfiguration>
#include <QMdmmProtocol>

QMDMM_EXPORT_NAME(QMdmmLogicRunner)

namespace QMdmmNetworking {

namespace p {
class LogicRunnerP;
}

namespace v0 {
class Agent;
class Socket;

// for a simpler logic, I decided to make LogicRunner handle only one complete game.
// so that there will be less need to implement Lobby or something that a player may select the room he / she wants to join in.
// If a player / agent exits mid-game, current practice would be to destroy this LogicRunner and its children and disconnect all the agents
// Players who wants to continue playing need rejoin.

// TODO: Implement Lobby when other contents are ready
class QMDMMNETWORKING_EXPORT LogicRunner final : public QObject
{
    Q_OBJECT

public:
    // Constructor and destructor: need to be called in Server thread (so that the LogicRunner instance is on Server thread)
    explicit LogicRunner(const QMdmmCore::LogicConfiguration &logicConfiguration, QObject *parent = nullptr);
    ~LogicRunner() override;

    // Functions to be called in Server thread
    Agent *addSocket(const QString &playerName, const QString &screenName, const QMdmmCore::Data::AgentState &agentState, Socket *socket);

    Agent *agent(const QString &playerName);
    [[nodiscard]] const Agent *agent(const QString &playerName) const;

    [[nodiscard]] bool full() const;

signals: // NOLINT(readability-redundant-access-specifiers)
    void gameOver(QPrivateSignal);

private:
    friend class p::LogicRunnerP;
    // LogicRunnerP is QObject. QPointer can't be used since it is incomplete here
    p::LogicRunnerP *const d;
    Q_DISABLE_COPY_MOVE(LogicRunner);
};
} // namespace v0

inline namespace v1 {
using v0::LogicRunner;
}

} // namespace QMdmmNetworking
#endif // QMDMMLOGICRUNNER_H
