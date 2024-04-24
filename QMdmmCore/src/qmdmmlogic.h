
#ifndef QMDMMLOGIC_H
#define QMDMMLOGIC_H

#include "qmdmmcoreglobal.h"

class QMdmmAgent;

class QMDMMCORE_EXPORT QMdmmLogic
{
public:
    void run();
};

class QMDMMCORE_EXPORT QMdmmLogicRunner final
{
public:
    // Constuctor and destructor: need to be called in Server thread (so that the QMdmmLogicRunner instance is on Server thread)
    QMdmmLogicRunner();
    ~QMdmmLogicRunner();

    // Functions to be called in Server thread
    bool registerAgent(QMdmmAgent *agent);
    bool deregisterAgent(QMdmmAgent *agent);

    bool incomingMessage(int todo, ...);
    bool outgoingMessage(int todo, ...);

    // Functions to be called in Logic thread, or "callback" function to QMdmmLogic
    // maybe private then friend?
    bool outgoingMessageFromLogic(int todo, ...);
};

#endif
