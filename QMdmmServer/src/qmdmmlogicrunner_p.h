// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMLOGICRUNNER_P
#define QMDMMLOGICRUNNER_P

#include "qmdmmlogicrunner.h"

#include "qmdmmagent.h"
#include "qmdmmsocket.h"

#include <QMdmmLogic>

#include <QPointer>
#include <QThread>

class QMDMMSERVER_PRIVATE_EXPORT QMdmmServerAgentPrivate : public QMdmmAgent
{
    Q_OBJECT

public:
    QMdmmServerAgentPrivate(const QString &name, QMdmmLogicRunnerPrivate *parent);
    ~QMdmmServerAgentPrivate() override;

    void setSocket(QMdmmSocket *_socket);

    QPointer<QMdmmSocket> socket;
    bool isReconnect;

signals:
    void sendPacket(QMdmmPacket packet);

public slots: // NOLINT(readability-redundant-access-specifiers)
    void packetReceived(QMdmmPacket packet);
    void socketDisconnected();
    void socketReconnected();
};

class QMDMMSERVER_PRIVATE_EXPORT QMdmmLogicRunnerPrivate : public QObject
{
    Q_OBJECT

public:
    QMdmmLogicRunnerPrivate(const QMdmmLogicConfiguration &logicConfiguration, QMdmmLogicRunner *parent);
    ~QMdmmLogicRunnerPrivate() override;

    QHash<QString, QMdmmServerAgentPrivate *> agents;

    QThread *logicThread;
    QPointer<QMdmmLogic> logic;

    QMdmmLogicConfiguration conf;

    // TODO: check if these queue connected signals / slots works with or without qRegisterMetaType<>()
    // If not, a new global function is needed in QMdmmCore for doing this work
    // I did a test of these combinations and found that all these types work without qRegisterMetaType<>() but it needs further testing

public slots: // NOLINT(readability-redundant-access-specifiers)
    void requestSscForAction(const QStringList &playerNames);
    void sscResult(const QHash<QString, QMdmmData::StoneScissorsCloth> &replies);
    void requestActionOrder(const QString &playerName, const QList<int> &availableOrders, int maximumOrderNum, int selections);
    void actionOrderResult(const QHash<int, QString> &result);
    void requestSscForActionOrder(const QStringList &playerNames, int strivedOrder);
    void requestAction(const QString &playerName, int actionOrder);
    void actionResult(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPosition);
    void requestUpgrade(const QString &playerName, int upgradePoint);
    void upgradeResult(const QHash<QString, QList<QMdmmData::UpgradeItem>> &upgrades);

signals: // NOLINT(readability-redundant-access-specifiers)
    void addPlayer(const QString &playerName);
    void removePlayer(const QString &playerName);
    void gameStart();

    void sscReply(const QString &playerName, QMdmmData::StoneScissorsCloth ssc);
    void actionOrderReply(const QString &playerName, const QList<int> &desiredOrder);
    void actionReply(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPosition);
    void upgradeReply(const QString &playerName, const QList<QMdmmData::UpgradeItem> &items);
};

#endif
