// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMLOGIC_H
#define QMDMMLOGIC_H

#include "qmdmmcoreglobal.h"

#include <QJsonObject>
#include <QObject>

struct QMDMMCORE_EXPORT QMdmmLogicConfiguration final : public QJsonObject
{
    static const QMdmmLogicConfiguration &defaults();

    enum PunishHpRoundStrategy
    {
        RoundDown, // 1.1 -> 1, 1.4 -> 1, 1.5 -> 1, 1.9 -> 1, 2.0 -> 2
        RoundToNearest45, // 1.1 -> 1, 1.4 -> 1, 1.5 -> 2, 1.9 -> 2, 2.0 -> 2
        RoundUp, // 1.1 -> 2, 1.4 -> 2, 1.5 -> 2, 1.9 -> 2, 2.0 -> 2
        PlusOne, // 1.1 -> 2, 1.4 -> 2, 1.5 -> 2, 1.9 -> 2, 2.0 -> 3
    };

    using QJsonObject::QJsonObject;
    using QJsonObject::operator=;

#define DEFINE_CONFIGURATION(type, valueName, ValueName) \
    [[nodiscard]] type valueName() const;                \
    void set##ValueName(type valueName);

    // TODO: move to ServerConfiguration
    DEFINE_CONFIGURATION(int, playerNumPerRoom, PlayerNumPerRoom)
    DEFINE_CONFIGURATION(int, requestTimeout, RequestTimeout)

    // I had experienced 2 versions of MDMM, they are mostly same but with minor differences.
    // In both versions one can only buy k/h in city.
    // In both versions one can slash only if knife is bought and kick only if horse is bought.
    // In both versions one can only slash / kick other one when they are at same place.
    // In both versions one can kick other one in city, and the kicked one will be force moved to country.
    // In both versions one can only move to adjecent place at a time. Different cities are not adjecent, while country is adjecent to each city.
    // Version 1 (Legacy): initial 7/1/3 mh/kd/hd, maximum 7/3/5 mh/kd/hd (Yeah, no maxHp upgrade). One with zero HP still alive. No "Let move"s, no punish HP.
    // This is the version I had experienced in primary school.
    // This version knife upgrades are more valuable. Horse is of no value since there is no HP punish.
    // Version 2: initial 10/1/2 mh/kd/hd, maximum 20/10/10 mh/kd/hd. One with zero HP dies. With "Let move"s (pull sb. in / push sb. out of city stuff), with punish HP.
    // This is the version I had experienced in junior high school.
    // This version horse can be effectly used in multi-player. pull - kick loop is fun!

    // Default configurations matches rules of Version 2 and can be tweaked.

    // standard configuration
    DEFINE_CONFIGURATION(int, initialKnifeDamage, InitialKnifeDamage)
    DEFINE_CONFIGURATION(int, maximumKnifeDamage, MaximumKnifeDamage)
    DEFINE_CONFIGURATION(int, initialHorseDamage, InitialHorseDamage)
    DEFINE_CONFIGURATION(int, maximumHorseDamage, MaximumHorseDamage)
    DEFINE_CONFIGURATION(int, initialMaxHp, InitialMaxHp)
    DEFINE_CONFIGURATION(int, maximumMaxHp, MaximumMaxHp)

    // legacy experience
    DEFINE_CONFIGURATION(int, punishHpModifier, PunishHpModifier)
    DEFINE_CONFIGURATION(QMdmmLogicConfiguration::PunishHpRoundStrategy, punishHpRoundStrategy, PunishHpRoundStrategy)
    DEFINE_CONFIGURATION(bool, zeroHpAsDead, ZeroHpAsDead)
    DEFINE_CONFIGURATION(bool, enableLetMove, EnableLetMove)
    DEFINE_CONFIGURATION(bool, canBuyOnlyInInitialCity, CanBuyOnlyInInitialCity)

#undef DEFINE_CONFIGURATION

    bool deserialize(const QJsonValue &value);
};

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

    [[nodiscard]] const QMdmmLogicConfiguration &configuration() const;
    [[nodiscard]] State state() const;

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
    QMdmmLogicPrivate *const d;
    Q_DISABLE_COPY_MOVE(QMdmmLogic);
};

#endif
