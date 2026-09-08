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
    // Pure virtual so each style subclass is forced to answer with its own
    // strategy; a bot that never answers would otherwise stall the match until
    // the server times it out and disconnects it.
    virtual void handleRockPaperScissorsRequest(const QStringList &playerNames, int strivedOrder) = 0;
    virtual void handleActionOrderRequest(const QList<int> &remainedOrders, int maximumOrder, int selectionNum) = 0;
    virtual void handleActionRequest(int currentOrder) = 0;
    virtual void handleUpgradeRequest(int remainingTimes) = 0;

    // Notification handlers: invoked when the server broadcasts game progress.
    // Virtual for the same reason -- style subclasses track the match here.
    virtual void handleLogicConfigurationNotified();
    virtual void handleRoundStartNotified();
    virtual void handleActionNotified(const QString &playerName, QMdmmCore::Data::Action action, const QString &toPlayer, int toPlace);
    virtual void handleUpgradeNotified(const QHash<QString, QList<QMdmmCore::Data::UpgradeItem>> &upgrades);
    virtual void handleRoundOverNotified();
    virtual void handleGameOverNotified(const QStringList &playerNames);
};

// The three playing styles. Each is a concrete Bot whose strategy is implemented
// (or, for rlBot, deliberately absent) in its own translation unit. They are
// only instantiated through Bot::createBot().

class KnifePreferredBot final : public Bot
{
public:
    explicit KnifePreferredBot(QMdmmNetworking::Client *parent);

protected:
    void handleRockPaperScissorsRequest(const QStringList &playerNames, int strivedOrder) override;
    void handleActionOrderRequest(const QList<int> &remainedOrders, int maximumOrder, int selectionNum) override;
    void handleActionRequest(int currentOrder) override;
    void handleUpgradeRequest(int remainingTimes) override;
};

class HorsePreferredBot final : public Bot
{
public:
    explicit HorsePreferredBot(QMdmmNetworking::Client *parent);

protected:
    void handleRockPaperScissorsRequest(const QStringList &playerNames, int strivedOrder) override;
    void handleActionOrderRequest(const QList<int> &remainedOrders, int maximumOrder, int selectionNum) override;
    void handleActionRequest(int currentOrder) override;
    void handleUpgradeRequest(int remainingTimes) override;
};

class RlBot final : public Bot
{
public:
    explicit RlBot(QMdmmNetworking::Client *parent);

protected:
    void handleRockPaperScissorsRequest(const QStringList &playerNames, int strivedOrder) override;
    void handleActionOrderRequest(const QList<int> &remainedOrders, int maximumOrder, int selectionNum) override;
    void handleActionRequest(int currentOrder) override;
    void handleUpgradeRequest(int remainingTimes) override;
};

#endif
