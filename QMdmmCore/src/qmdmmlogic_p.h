// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMLOGIC_P
#define QMDMMLOGIC_P

#include "qmdmmlogic.h"

class QMdmmRoom;

struct QMDMMCORE_PRIVATE_EXPORT QMdmmLogicPrivate final
{
    QMdmmLogicPrivate(const QMdmmLogicConfiguration &logicConfiguration, QMdmmLogic *q);

    QMdmmLogic *q;
    QMdmmRoom *room;
    QStringList players;

    QMdmmLogic::State state;

    QHash<QString, QMdmmData::StoneScissorsCloth> sscForActionReplies;
    QStringList sscForActionWinners;
    QMultiHash<int, QString> desiredActionOrders;
    QHash<int, QString> confirmedActionOrders;
    int currentStrivingActionOrder;
    QHash<QString, QMdmmData::StoneScissorsCloth> sscForActionOrderReplies;
    int currentActionOrder;
    QHash<QString, QList<QMdmmData::UpgradeItem>> upgrades;

    // helper functions
    [[nodiscard]] bool actionFeasible(const QString &fromPlayer, QMdmmData::Action action, const QString &toPlayer, int toPlace) const;
    bool applyAction(const QString &fromPlayer, QMdmmData::Action action, const QString &toPlayer, int toPlace);

    // Functions:
    void startSscForAction();
    void sscForAction();
    void startActionOrder();
    void actionOrder();
    void startSscForActionOrder();
    void sscForActionOrder();
    void startAction();
    void startUpgrade();
    void upgrade();
};

#endif
