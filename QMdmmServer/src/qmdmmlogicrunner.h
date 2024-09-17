// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMLOGICRUNNER_H
#define QMDMMLOGICRUNNER_H

#include "qmdmmserverglobal.h"

#include <QThread>

class QMdmmAgent;
struct QMdmmLogicConfiguration;

struct QMdmmLogicRunnerPrivate;

class QMDMMSERVER_EXPORT QMdmmLogicRunner final : public QThread
{
    Q_OBJECT

public:
    // Constuctor and destructor: need to be called in Server thread (so that the QMdmmLogicRunner instance is on Server thread)
    // pay attention
    QMdmmLogicRunner(const QMdmmLogicConfiguration &logicConfiguration, QObject *parent = nullptr);
    ~QMdmmLogicRunner() override;

    // Functions to be called in Server thread
    bool registerAgent(QMdmmAgent *agent);
    bool deregisterAgent(QMdmmAgent *agent);

private:
    QMdmmLogicRunnerPrivate *const d;
};

#endif // QMDMMLOGICRUNNER_H
