// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMBOT_BOT_H
#define QMDMMBOT_BOT_H

#include <QHash>
#include <QList>
#include <QStringList>

#include <QMdmmClient>
#include <QMdmmData>

#include <QObject>

class Bot : public QObject
{
    Q_OBJECT

public:
    explicit Bot(QMdmmNetworking::Client *parent);
    virtual ~Bot() override = 0;

    [[nodiscard]] QMdmmNetworking::Client *client();
    [[nodiscard]] const QMdmmNetworking::Client *client() const;

    Q_DISABLE_COPY_MOVE(Bot);

    [[nodiscard]] static Bot *createBot(const QString &style, QMdmmNetworking::Client *parent);
    [[nodiscard]] static bool styleExist(const QString &style);

protected slots:
    // Request handlers: invoked when the server asks this bot to make a choice.
    // Virtual so each style subclass can override them with its own strategy.
    // The base implementation does nothing, so a bot without a strategy simply
    // does not answer (the match cannot progress, which is acceptable because
    // the base class is abstract and never used directly).
    virtual void handleRockPaperScissorsRequest(const QStringList &playerNames, int strivedOrder);
    virtual void handleActionOrderRequest(const QList<int> &remainedOrders, int maximumOrder, int selectionNum);
    virtual void handleActionRequest(int currentOrder);
    virtual void handleUpgradeRequest(int remainingTimes);

    // Notification handlers: invoked when the server broadcasts game progress.
    // Virtual for the same reason -- style subclasses track the match here.
    virtual void handleLogicConfigurationNotified();
    virtual void handleRoundStartNotified();
    virtual void handleActionNotified(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace);
    virtual void handleUpgradeNotified(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades);
    virtual void handleRoundOverNotified();
    virtual void handleGameOverNotified(const QStringList &playerNames);
};

#endif
