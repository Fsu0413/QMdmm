// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMLOGICRUNNER_H
#define QMDMMLOGICRUNNER_H

#include "qmdmmserverglobal.h"

class QMdmmAgent;
class QMdmmSocket;
struct QMdmmLogicConfiguration;
class QMdmmLogicRunnerPrivate;

class QMDMMSERVER_EXPORT QMdmmLogicRunner final : public QObject
{
    Q_OBJECT

public:
    // Constuctor and destructor: need to be called in Server thread (so that the QMdmmLogicRunner instance is on Server thread)
    // pay attention
    QMdmmLogicRunner(const QMdmmLogicConfiguration &logicConfiguration, QObject *parent = nullptr);
    ~QMdmmLogicRunner() override;

    // Functions to be called in Server thread
    QMdmmAgent *addSocket(const QString &playerName, QMdmmSocket *socket);
    bool removeSocket(const QString &playerName);

    // TODO: property: full, gamerunning implementation
    [[nodiscard]] bool full() const;
    [[nodiscard]] bool gameRunning() const;

private:
    QMdmmLogicRunnerPrivate *const d;
};

#endif // QMDMMLOGICRUNNER_H
