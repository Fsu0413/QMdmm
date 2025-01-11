// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMLOGIC_H
#define QMDMMLOGIC_H

#include "qmdmmcoreglobal.h"

#include <QObject>

struct QMdmmLogicConfiguration;

struct QMdmmLogicPrivate;

class QMDMMCORE_EXPORT QMdmmLogic final : public QObject
{
    Q_OBJECT

public:
    enum State
    {
        BeforeRoundStart,
        SscForAction,
        ActionOrder,
        SscForActionOrder,
        Action,
        Upgrade,
    };
    Q_ENUM(State)

    explicit QMdmmLogic(const QMdmmLogicConfiguration &logicConfiguration, QObject *parent = nullptr);
    ~QMdmmLogic() override;

    [[nodiscard]] State state() const noexcept;

public slots: // NOLINT(readability-redundant-access-specifiers)
    void addPlayer(const QString &playerName);
    void removePlayer(const QString &playerName);
    void roundStart();

    void sscReply(const QString &playerName, QMdmmData::StoneScissorsCloth ssc);
    void actionOrderReply(const QString &playerName, const QList<int> &desiredOrder);
    void actionReply(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPlace);
    void upgradeReply(const QString &playerName, const QList<QMdmmData::UpgradeItem> &items);

signals: // NOLINT(readability-redundant-access-specifiers)
    void requestSscForAction(const QStringList &playerNames, QPrivateSignal);
    void sscResult(const QHash<QString, QMdmmData::StoneScissorsCloth> &replies, QPrivateSignal);
    void requestActionOrder(const QString &playerName, const QList<int> &availableOrders, int maximumOrderNum, int selections, QPrivateSignal);
    void actionOrderResult(const QHash<int, QString> &result, QPrivateSignal);
    void requestSscForActionOrder(const QStringList &playerNames, int strivedOrder, QPrivateSignal);
    void requestAction(const QString &playerName, int actionOrder, QPrivateSignal);
    void actionResult(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPlace, QPrivateSignal);
    void roundOver(QPrivateSignal);
    void requestUpgrade(const QString &playerName, int upgradePoint, QPrivateSignal);
    void upgradeResult(const QHash<QString, QList<QMdmmData::UpgradeItem>> &upgrades, QPrivateSignal);
    void gameOver(const QStringList &playerNames, QPrivateSignal);

private:
    friend struct QMdmmLogicPrivate;
    const std::unique_ptr<QMdmmLogicPrivate> d;
    Q_DISABLE_COPY_MOVE(QMdmmLogic);
};

#endif
