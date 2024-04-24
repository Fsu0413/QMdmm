#include "qmdmmlogic.h"

#include "qmdmmagent.h"

QMdmmLogicRunner::QMdmmLogicRunner(QObject *parent)
    : QThread(parent)
{
}

QMdmmLogicRunner::~QMdmmLogicRunner()
{
}

bool QMdmmLogicRunner::registerAgent(QMdmmAgent *agent)
{
    return false;
}

bool QMdmmLogicRunner::deregisterAgent(QMdmmAgent *agent)
{
    return false;
}

// bool QMdmmLogicRunner::incomingMessage(int todo)
// {
// }

// bool QMdmmLogicRunner::outgoingMessage(int todo)
// {
// }

// bool QMdmmLogicRunner::outgoingMessageFromLogic(int todo)
// {
// }

void QMdmmLogicRunner::run()
{
}
