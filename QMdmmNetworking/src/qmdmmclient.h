// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMCLIENT_H
#define QMDMMCLIENT_H

#include "qmdmmnetworkingglobal.h"

#include <QObject>

class QMdmmClientPrivate;

class QMDMMNETWORKING_EXPORT QMdmmClient final : public QObject
{
    Q_OBJECT

public:
    explicit QMdmmClient(QObject *parent = nullptr);
    ~QMdmmClient() override;

    bool connectToHost(const QString &host);

public slots: // NOLINT(readability-redundant-access-specifiers)
    void notifySpeak(const QString &content);
    void notifyOperate(const void *todo);

    void requestTimeout();

signals:
    void socketErrorDisconnected(const QString &errorString, QPrivateSignal);

    void requestStoneScissorsCloth(const QStringList &playerNames, int strivedOrder, QPrivateSignal);
    void requestActionOrder(const QList<int> &remainedOrders, int maximumOrder, int selectionNum, QPrivateSignal);
    void requestAction(int currentOrder, QPrivateSignal);
    void requestUpgrade(int remainingTimes, QPrivateSignal);

    void notifyPlayerAdded(const QString &playerName, const QString &screenName, const QMdmmData::AgentState &agentState, QPrivateSignal);
    void notifyPlayerRemoved(const QString &playerName, QPrivateSignal);
    void notifyGameStart(QPrivateSignal);
    void notifyRoundStart(QPrivateSignal);
    void notifyStoneScissorsCloth(const QHash<QString, QMdmmData::StoneScissorsCloth> &ssc, QPrivateSignal);
    void notifyActionOrder(const QHash<int, QString> &actionOrderResult, QPrivateSignal);
    void notifyAction(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPlace, QPrivateSignal);
    void notifyRoundOver(QPrivateSignal);
    void notifyUpgrade(const QHash<QString, QList<QMdmmData::UpgradeItem>> &upgrades, QPrivateSignal);
    void notifyGameOver(const QStringList &winners, QPrivateSignal);
    void notifySpoken(const QString &playerName, const QString &content, QPrivateSignal);
    void notifyOperated(QPrivateSignal);

private:
    friend class QMdmmClientPrivate;
    QMdmmClientPrivate *const d;
    Q_DISABLE_COPY_MOVE(QMdmmClient);
};

#endif
