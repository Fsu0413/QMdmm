// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMLOGIC_H
#define QMDMMLOGIC_H

#include "qmdmmcoreglobal.h"
#include <QThread>

class QMdmmAgent;

// This will be implemented using custom event queue.
// Qt signal-slot does not apply in this class.
class QMDMMCORE_EXPORT QMdmmLogic
{
public:
    void run();
};

class QMDMMCORE_EXPORT QMdmmLogicRunner final : public QThread
{
    Q_OBJECT

public:
    // Constuctor and destructor: need to be called in Server thread (so that the QMdmmLogicRunner instance is on Server thread)
    QMdmmLogicRunner(QObject *parent = nullptr);
    ~QMdmmLogicRunner() override;

    // Functions to be called in Server thread
    bool registerAgent(QMdmmAgent *agent);
    bool deregisterAgent(QMdmmAgent *agent);

    void run() override;
};

#endif
