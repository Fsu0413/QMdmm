// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMLOGIC_H
#define QMDMMLOGIC_H

#include "qmdmmcoreglobal.h"

#include <QObject>

#include <cstdint>

QMDMM_EXPORT_NAME(QMdmmLogic)

class tst_QMdmmLogic;

namespace QMdmmCore {

namespace p {
struct LogicP;
}

#ifndef DOXYGEN
namespace v0 {
#else
inline namespace v1 {
#endif

class LogicConfiguration;

class QMDMMCORE_EXPORT Logic final : public QObject
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

    explicit Logic(const LogicConfiguration &logicConfiguration, QObject *parent = nullptr);
    ~Logic() override;

    [[nodiscard]] State state() const noexcept;

public slots: // NOLINT(readability-redundant-access-specifiers)
    bool addPlayer(const QString &playerName);
    bool removePlayer(const QString &playerName);
    bool roundStart();

    bool sscReply(const QString &playerName, Data::StoneScissorsCloth ssc);
    bool actionOrderReply(const QString &playerName, const QList<int> &desiredOrder);
    bool actionReply(const QString &playerName, Data::Action action, const QString &toPlayer, int toPlace);
    bool upgradeReply(const QString &playerName, const QList<Data::UpgradeItem> &items);

signals: // NOLINT(readability-redundant-access-specifiers)
    void requestSscForAction(const QStringList &playerNames, QPrivateSignal);
    void sscResult(const QHash<QString, Data::StoneScissorsCloth> &replies, QPrivateSignal);
    void requestActionOrder(const QString &playerName, const QList<int> &availableOrders, int maximumOrderNum, int selections, QPrivateSignal);
    void actionOrderResult(const QHash<int, QString> &result, QPrivateSignal);
    void requestSscForActionOrder(const QStringList &playerNames, int strivedOrder, QPrivateSignal);
    void requestAction(const QString &playerName, int actionOrder, QPrivateSignal);
    void actionResult(const QString &playerName, Data::Action action, const QString &toPlayer, int toPlace, QPrivateSignal);
    void roundOver(QPrivateSignal);
    void requestUpgrade(const QString &playerName, int upgradePoint, QPrivateSignal);
    void upgradeResult(const QHash<QString, QList<Data::UpgradeItem>> &upgrades, QPrivateSignal);
    void gameOver(const QStringList &playerNames, QPrivateSignal);

#ifndef DOXYGEN
private:
    friend struct p::LogicP;
    const std::unique_ptr<p::LogicP> d;
    Q_DISABLE_COPY_MOVE(Logic);

    friend class ::tst_QMdmmLogic;
#endif
};

} // namespace v0

#ifndef DOXYGEN
inline namespace v1 {
using v0::Logic;
}
#endif

} // namespace QMdmmCore

#endif
