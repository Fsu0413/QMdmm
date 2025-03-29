// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMLOGICRUNNER_H
#define QMDMMLOGICRUNNER_H

#include "qmdmmnetworkingglobal.h"

#include <QMdmmLogicConfiguration>
#include <QMdmmProtocol>

class QMdmmAgent;
class QMdmmSocket;

QMDMM_EXPORT_NAME(QMdmmLogicRunner)

// for a simpler logic, I decided to make QMdmmLogicRunner handle only one complete game.
// so that there will be less need to implement Lobby or something that a player may select the room he / she wants to join in.
// If a player / agent exits mid-game, current practice would be to destroy this QMdmmLogicRunner and its children and disconnect all the agents
// Players who wants to continue playing need rejoin.

class QMdmmLogicRunnerPrivate;

// TODO: Implement Lobby when other contents are ready
class QMDMMNETWORKING_EXPORT QMdmmLogicRunner final : public QObject
{
    Q_OBJECT

public:
    // Constructor and destructor: need to be called in Server thread (so that the QMdmmLogicRunner instance is on Server thread)
    explicit QMdmmLogicRunner(const QMdmmCore::LogicConfiguration &logicConfiguration, QObject *parent = nullptr);
    ~QMdmmLogicRunner() override;

    // Functions to be called in Server thread
    QMdmmAgent *addSocket(const QString &playerName, const QString &screenName, const QMdmmCore::Data::AgentState &agentState, QMdmmSocket *socket);

    QMdmmAgent *agent(const QString &playerName);
    [[nodiscard]] const QMdmmAgent *agent(const QString &playerName) const;

    [[nodiscard]] bool full() const;

signals: // NOLINT(readability-redundant-access-specifiers)
    void gameOver(QPrivateSignal);

private:
    friend class QMdmmLogicRunnerPrivate;
    // QMdmmLogicRunnerPrivate is QObject. QPointer can't be used since it is incomplete here
    QMdmmLogicRunnerPrivate *const d;
    Q_DISABLE_COPY_MOVE(QMdmmLogicRunner);
};

#endif // QMDMMLOGICRUNNER_H
