// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMLOGIC_P
#define QMDMMLOGIC_P

#include "qmdmmlogic.h"

#include "qmdmmroom.h"

// NOLINTBEGIN(misc-non-private-member-variables-in-classes): This is private header

namespace QMdmmCore {

namespace p {

struct QMDMMCORE_PRIVATE_EXPORT LogicP final
{
    LogicP(const LogicConfiguration &logicConfiguration, Logic *q);

    Logic *q;
    Room *room;

    Logic::State state;

    QHash<QString, Data::RockPaperScissors> rpsForActionReplies;
    QStringList rpsForActionWinners;
    QMultiHash<int, QString> desiredActionOrders;
    QHash<int, QString> confirmedActionOrders;
    QHash<QString, int> actionOrderRemainingSelections;
    QHash<QString, int> actionOrderYields;
    int currentStrivingActionOrder;
    QHash<QString, Data::RockPaperScissors> rpsForActionOrderReplies;
    int currentActionOrder;
    QHash<QString, QList<Data::UpgradeItem>> upgrades;

    // helper functions
    [[nodiscard]] bool actionFeasible(const QString &fromPlayer, Data::Action action, const QString &toPlayer, int toPlace) const;
    bool applyAction(const QString &fromPlayer, Data::Action action, const QString &toPlayer, int toPlace);

    // Functions:
    void startRpsForAction();
    void rpsForAction();
    void startActionOrder();
    void actionOrder();
    void startRpsForActionOrder();
    void rpsForActionOrder();
    void startAction();
    void startUpgrade();
    void upgrade();
};

} // namespace p

} // namespace QMdmmCore

// NOLINTEND(misc-non-private-member-variables-in-classes): This is private header

#endif
