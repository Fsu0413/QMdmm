// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMLOGIC_H
#define QMDMMLOGIC_H

#include "qmdmmcoreglobal.h"

#include <QObject>

#include <cstdint>

class tst_QMdmmLogic;

namespace QMdmmCore {

struct QMdmmLogicPrivate;

namespace v0 {

class QMdmmLogicConfiguration;

class QMDMMCORE_EXPORT QMdmmLogic final : public QObject
{
    Q_OBJECT

public:
    enum State : uint8_t
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
    bool addPlayer(const QString &playerName);
    bool removePlayer(const QString &playerName);
    bool roundStart();

    bool sscReply(const QString &playerName, QMdmmData::StoneScissorsCloth ssc);
    bool actionOrderReply(const QString &playerName, const QList<int> &desiredOrder);
    bool actionReply(const QString &playerName, QMdmmData::Action action, const QString &toPlayer, int toPlace);
    bool upgradeReply(const QString &playerName, const QList<QMdmmData::UpgradeItem> &items);

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

#ifndef DOXYGEN
private:
    friend struct QMdmmCore::QMdmmLogicPrivate;
    const std::unique_ptr<QMdmmCore::QMdmmLogicPrivate> d;
    Q_DISABLE_COPY_MOVE(QMdmmLogic);

    friend class ::tst_QMdmmLogic;
#endif
};

} // namespace v0

inline namespace v1 {
using v0::QMdmmLogic;
}

} // namespace QMdmmCore

#endif
