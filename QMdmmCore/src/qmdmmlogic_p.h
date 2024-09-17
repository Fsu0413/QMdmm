// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMLOGIC_P
#define QMDMMLOGIC_P

#include "qmdmmlogic.h"

class QMdmmRoom;

struct QMDMMCORE_EXPORT QMdmmLogicPrivate
{
    QMdmmLogicPrivate(QMdmmLogic *q, const QMdmmLogicConfiguration &logicConfiguration);

    QMdmmLogic *q;
    QMdmmLogicConfiguration conf;
    QMdmmRoom *room;
    QStringList players;

    QMdmmLogic::State state;

    QHash<QString, QMdmmData::StoneScissorsCloth> sscForOperationReplies;
    QStringList sscForOperationWinners;
    // QMultiMap / QMultiHash - Which one is faster?
    // QMultiMap provides O(logn) as time complexity, it depends on comparation
    // QMultiHash provides O(1) as time complexity, but it depends on qHash<int>
    QMultiHash<int, QString> desiredOperationOrders;
    QHash<int, QString> confirmedOperationOrders;
    int currentStrivingOperationOrder;
    QHash<QString, QMdmmData::StoneScissorsCloth> sscForOperationOrderReplies;

    // Functions:
    void startBeforeGameStart();
    void startSscForOperation();
    void sscForOperation();
    void startOperationOrder();
    void operationOrder();
    void startSscForOperationOrder();
    void sscForOperationOrder();
    void startOperation();
};

#endif
