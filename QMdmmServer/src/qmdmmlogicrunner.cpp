// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmlogicrunner.h"
#include "qmdmmlogicrunner_p.h"

#include <QMetaType>

QMdmmServerAgentPrivate::QMdmmServerAgentPrivate(const QString &name, QMdmmLogicRunnerPrivate *parent)
    : QMdmmAgent(name, parent)
    , isReconnect(false)
{
}

QMdmmServerAgentPrivate::~QMdmmServerAgentPrivate() = default;

void QMdmmServerAgentPrivate::setSocket(QMdmmSocket *_socket)
{
    if (socket != nullptr)
        socket->deleteLater();

    socket = _socket;
    if (socket != nullptr) {
        connect(socket, &QMdmmSocket::packetReceived, this, &QMdmmServerAgentPrivate::packetReceived);
        connect(socket, &QMdmmSocket::socketDisconnected, this, &QMdmmServerAgentPrivate::socketDisconnected);
        connect(this, &QMdmmServerAgentPrivate::sendPacket, socket, &QMdmmSocket::sendPacket);
    }
}

void QMdmmServerAgentPrivate::packetReceived(QMdmmPacket packet)
{
    (void)packet;
}

void QMdmmServerAgentPrivate::socketDisconnected()
{
    isReconnect = true;
    socket->deleteLater();
    socket = nullptr;
}

void QMdmmServerAgentPrivate::socketReconnected()
{
    isReconnect = false;
}

QMdmmLogicRunnerPrivate::QMdmmLogicRunnerPrivate(const QMdmmLogicConfiguration &logicConfiguration, QMdmmLogicRunner *parent)
    : QObject(parent)
    , conf(logicConfiguration)
{
    logicThread = new QThread(this);
    logic = new QMdmmLogic(conf);
    logic->moveToThread(logicThread);
    connect(logicThread, &QThread::finished, logic, &QMdmmLogic::deleteLater);
    logicThread->start();

#define CONNECTRUNNERTOLOGIC(signalName)                                                                           \
    do {                                                                                                           \
        connect(this, &QMdmmLogicRunnerPrivate::signalName, logic, &QMdmmLogic::signalName, Qt::QueuedConnection); \
    } while (0)

    CONNECTRUNNERTOLOGIC(addPlayer);
    CONNECTRUNNERTOLOGIC(removePlayer);
    CONNECTRUNNERTOLOGIC(gameStart);
    CONNECTRUNNERTOLOGIC(sscReply);
    CONNECTRUNNERTOLOGIC(actionOrderReply);
    CONNECTRUNNERTOLOGIC(actionReply);
    CONNECTRUNNERTOLOGIC(upgradeReply);

#undef CONNECTRUNNERTOLOGIC

#define CONNECTLOGICTORUNNER(signalName)                                                                           \
    do {                                                                                                           \
        connect(logic, &QMdmmLogic::signalName, this, &QMdmmLogicRunnerPrivate::signalName, Qt::QueuedConnection); \
    } while (0)

    CONNECTLOGICTORUNNER(requestSscForAction);
    CONNECTLOGICTORUNNER(sscResult);
    CONNECTLOGICTORUNNER(requestActionOrder);
    CONNECTLOGICTORUNNER(actionOrderResult);
    CONNECTLOGICTORUNNER(requestSscForActionOrder);
    CONNECTLOGICTORUNNER(requestAction);
    CONNECTLOGICTORUNNER(actionResult);
    CONNECTLOGICTORUNNER(requestUpgrade);
    CONNECTLOGICTORUNNER(upgradeResult);

#undef CONNECTLOGICTORUNNER
}

QMdmmLogicRunnerPrivate::~QMdmmLogicRunnerPrivate() = default;

void QMdmmLogicRunnerPrivate::requestSscForAction(const QStringList &playerNames)
{
}

void QMdmmLogicRunnerPrivate::sscResult(const QHash<QString, QMdmmData::StoneScissorsCloth> &replies)
{
}

void QMdmmLogicRunnerPrivate::requestActionOrder(const QString &playerName, const QList<int> &availableOrders, int maximumOrderNum, int selections)
{
}

void QMdmmLogicRunnerPrivate::actionOrderResult(const QHash<int, QString> &result)
{
}

void QMdmmLogicRunnerPrivate::requestSscForActionOrder(const QStringList &playerNames, int strivedOrder)
{
}

void QMdmmLogicRunnerPrivate::requestAction(const QString &playerName, int actionOrder)
{
}

void QMdmmLogicRunnerPrivate::actionResult(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPosition)
{
}

void QMdmmLogicRunnerPrivate::requestUpgrade(const QString &playerName, int upgradePoint)
{
}

void QMdmmLogicRunnerPrivate::upgradeResult(const QHash<QString, QList<QMdmmData::UpgradeItem>> &upgrades)
{
}

QMdmmLogicRunner::QMdmmLogicRunner(const QMdmmLogicConfiguration &logicConfiguration, QObject *parent)
    : QObject(parent)
    , d(new QMdmmLogicRunnerPrivate(logicConfiguration, this))
{
}

QMdmmLogicRunner::~QMdmmLogicRunner() = default;

QMdmmAgent *QMdmmLogicRunner::addSocket(const QString &playerName, QMdmmSocket *socket)
{
    if (d->agents.contains(playerName))
        return nullptr;

    QMdmmServerAgentPrivate *agent = new QMdmmServerAgentPrivate(playerName, d);
    agent->setSocket(socket);
    d->agents.insert(playerName, agent);
    return agent;
}

bool QMdmmLogicRunner::removeSocket(const QString &playerName)
{
    return false;
}

bool QMdmmLogicRunner::full() const
{
    return false;
}

bool QMdmmLogicRunner::gameRunning() const
{
    return false;
}
