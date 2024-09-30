// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMLOGIC_H
#define QMDMMLOGIC_H

#include "qmdmmcoreglobal.h"
#include <QObject>

struct QMDMMCORE_EXPORT QMdmmLogicConfiguration final
{
    int playerNumPerRoom = 3;

    int initialKnifeDamage = 1;
    int maximumKnifeDamage = 5;

    int initialHorseDamage = 2;
    int maximumHorseDamage = 10;

    int initialMaxHp = 10;
    int maximumMaxHp = 20;

    int punishHpModifier = 2; // 1 / punishHpModifier for punished HP when slashed in city, 0 for disable.
    enum PunishHpRoundStrategy
    {
        RoundDown, // 1.1 -> 1 1.4 -> 1, 1.5 -> 1, 1.9 -> 1, 2.0 -> 2
        RoundToNearest, // 1.1 -> 1, 1.4 -> 1, 1.5 -> 2, 1.9 -> 2, 2.0 -> 2
        RoundUp, // 1.1 -> 2, 1.4 -> 2, 1.5 -> 2, 1.9 -> 2, 2.0 -> 2
        PlusOne, // 1.1 -> 2, 1.4 -> 2, 1.5 -> 2, 1.9 -> 2, 2.0 -> 3
    } punishHpRoundStrategy
        = RoundToNearest;

    bool zeroHpAsDead = true; // Treat one with hp value 0 as dead, which is the case in early versions. If false only one with minus hp value are treated as dead
    bool enableLetMove = true; // Let move is nonexistent in early versions

    [[nodiscard]] QJsonValue serialize() const;
    bool deserialize(const QJsonValue &value);
};

struct QMdmmLogicPrivate;

class QMDMMCORE_EXPORT QMdmmLogic final : public QObject
{
    Q_OBJECT
    friend struct QMdmmLogicPrivate;

public:
    enum State
    {
        BeforeGameStart,
        SscForAction,
        ActionOrder,
        SscForActionOrder,
        Action,
        Upgrade,
        GameFinish,
    };
    Q_ENUM(State)

    QMdmmLogic(const QMdmmLogicConfiguration &logicConfiguration, QObject *parent = nullptr);
    ~QMdmmLogic() override;

    [[nodiscard]] const QMdmmLogicConfiguration &configuration() const;
    [[nodiscard]] State state() const;

public slots: // NOLINT(readability-redundant-access-specifiers)
    void addPlayer(const QString &playerName);
    void removePlayer(const QString &playerName);
    void gameStart();

    void sscReply(const QString &playerName, QMdmmData::StoneScissorsCloth ssc);
    void actionOrderReply(const QString &playerName, const QList<int> &desiredOrder);
    void actionReply(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPosition);
    void upgradeReply(const QString &playerName, const QList<QMdmmData::UpgradeItem> &items);

signals: // NOLINT(readability-redundant-access-specifiers)
    void requestSscForAction(const QStringList &playerNames, QPrivateSignal);
    void sscResult(const QHash<QString, QMdmmData::StoneScissorsCloth> &replies, QPrivateSignal);
    void requestActionOrder(const QString &playerName, const QList<int> &availableOrders, int maximumOrderNum, int selections, QPrivateSignal);
    void actionOrderResult(const QHash<int, QString> &result, QPrivateSignal);
    void requestSscForActionOrder(const QStringList &playerNames, int strivedOrder, QPrivateSignal);
    void requestAction(const QString &playerName, int actionOrder, QPrivateSignal);
    void actionResult(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPosition, QPrivateSignal);
    void requestUpgrade(const QString &playerName, int upgradePoint, QPrivateSignal);
    void upgradeResult(const QHash<QString, QList<QMdmmData::UpgradeItem>> &upgrades, QPrivateSignal);

private:
    QMdmmLogicPrivate *const d;
};

#endif
