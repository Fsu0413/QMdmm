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

public slots: // NOLINT(readability-redundant-access-specifiers)
    void requestTimeout();

signals:
    void requestStoneScissorsCloth(const QStringList &playerNames, int strivedOrder);
    void requestActionOrder(const QList<int> &remainedOrders, int maximumOrder, int selectionNum);
    void requestAction(int currentOrder);
    void requestUpgrade(int remainingTimes);

    void notifyPlayerAdded(const QString &playerName, const QString &screenName, const QMdmmData::AgentState &agentState);
    void notifyPlayerRemoved(const QString &playerName);
    void notifyGameStart();
    void notifyRoundStart();
    void notifyStoneScissorsCloth(const QHash<QString, QMdmmData::StoneScissorsCloth> &ssc);
    void notifyActionOrder(const QHash<int, QString> &actionOrderResult);
    void notifyAction(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPlace);
    void notifyRoundOver();
    void notifyUpgrade(const QHash<QString, QList<QMdmmData::UpgradeItem>> &upgrades);
    void notifyGameOver(const QStringList &winners);
    void notifySpoken(const QString &playerName, const QString &content);
    void notifyOperated();

private:
    QMdmmClientPrivate *const d;
    Q_DISABLE_COPY_MOVE(QMdmmClient);
};

#endif
