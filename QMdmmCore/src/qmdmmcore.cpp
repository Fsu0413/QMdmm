// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmcoreglobal.h"

#include <QMap>

bool QMdmmData::isPlaceAdjecent(int p1, int p2)
{
    if (p1 == p2)
        return false;

    if ((p1 != Country) && (p2 != Country))
        return false;

    return true;
}

namespace {
constexpr bool sscGreater(QMdmmData::StoneScissorsCloth op1, QMdmmData::StoneScissorsCloth op2)
{
    return (op1 == QMdmmData::Stone && op2 == QMdmmData::Scissors) || (op1 == QMdmmData::Scissors && op2 == QMdmmData::Cloth)
        || (op1 == QMdmmData::Cloth && op2 == QMdmmData::Stone);
}
} // namespace

QStringList QMdmmData::stoneScissorsClothWinners(const QHash<QString, QMdmmData::StoneScissorsCloth> &judgers)
{
    QMap<QMdmmData::StoneScissorsCloth, QStringList> judgersMap;

    for (QHash<QString, QMdmmData::StoneScissorsCloth>::const_iterator it = judgers.constBegin(); it != judgers.constEnd(); ++it)
        judgersMap[it.value()] << it.key();

    if (judgersMap.count() == 2) {
        QMap<QMdmmData::StoneScissorsCloth, QStringList>::const_iterator it1 = judgersMap.constBegin();
        QMap<QMdmmData::StoneScissorsCloth, QStringList>::const_iterator it2 = judgersMap.constBegin();
        ++it2;

        QMdmmData::StoneScissorsCloth type1 = it1.key();
        QMdmmData::StoneScissorsCloth type2 = it2.key();

        if (!sscGreater(type1, type2))
            std::swap(it1, it2);

        // now it1.value is winner, it2.value is loser
        // we'd make every winners repeat N times (N is loser.count), for the real judgement use
        QStringList d;
        for (int i = 0; i < it2.value().length(); ++i)
            d.append(it1.value());

        return d;
    }

    return {};
}

QVersionNumber QMdmmGlobal::version()
{
    return QVersionNumber::fromString(QStringLiteral(QMDMM_VERSION));
}
