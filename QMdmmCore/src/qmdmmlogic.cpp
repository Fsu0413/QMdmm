#include "qmdmmlogic.h"

#include "qmdmmagent.h"
#include "qmdmmplayer.h"
#include "qmdmmroom.h"

void QMdmmLogic::run()
{
    QMdmmRoom *room = new QMdmmRoom;

    room->addPlayer(QStringLiteral("player1"));
    room->addPlayer(QStringLiteral("player2"));
    room->addPlayer(QStringLiteral("player3"));

    // logic main loop

    delete room;
}

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
    QMdmmLogic logic;
    logic.run();
}
