// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmstonescissorscloth.h"

#include <map>
using std::map;
using std::vector;

QMdmmStoneScissorsCloth::QMdmmStoneScissorsCloth()
    : m_type(QMdmmData::Stone)
{
}

QMdmmStoneScissorsCloth::QMdmmStoneScissorsCloth(QMdmmData::StoneScissorsCloth type)
    : m_type(type)
{
}

QMdmmStoneScissorsCloth::QMdmmStoneScissorsCloth(const QMdmmStoneScissorsCloth &other) = default;

QMdmmStoneScissorsCloth &QMdmmStoneScissorsCloth::operator=(const QMdmmStoneScissorsCloth &other) = default;

QMdmmStoneScissorsCloth &QMdmmStoneScissorsCloth::operator=(QMdmmData::StoneScissorsCloth type)
{
    m_type = type;
    return *this;
}

QMdmmStoneScissorsCloth::~QMdmmStoneScissorsCloth() = default;

bool QMdmmStoneScissorsCloth::operator>(const QMdmmStoneScissorsCloth &op2) const
{
    return (m_type == QMdmmData::Stone && op2.m_type == QMdmmData::Scissors) || (m_type == QMdmmData::Scissors && op2.m_type == QMdmmData::Cloth)
        || (m_type == QMdmmData::Cloth && op2.m_type == QMdmmData::Stone);
}

bool QMdmmStoneScissorsCloth::operator==(const QMdmmStoneScissorsCloth &op2) const
{
    return m_type == op2.m_type;
}

vector<vector<QMdmmStoneScissorsCloth>::size_type> QMdmmStoneScissorsCloth::winners(const vector<QMdmmStoneScissorsCloth> &judgers)
{
    map<QMdmmData::StoneScissorsCloth, vector<vector<QMdmmStoneScissorsCloth>::size_type>> judgersMap;

    for (vector<QMdmmStoneScissorsCloth>::size_type i = 0; i < judgers.size(); ++i) {
        const auto &judger = judgers.at(i);
        QMdmmData::StoneScissorsCloth t = judger;
        judgersMap[t].push_back(i);
    }

    if (judgersMap.size() == 2) {
        auto it1 = judgersMap.begin();
        auto it2 = judgersMap.begin();
        ++it2;

        QMdmmStoneScissorsCloth type1 = it1->first;
        QMdmmStoneScissorsCloth type2 = it2->first;

        if (type1 > type2)
            return it1->second;
        else
            return it2->second;
    }

    return {};
}

QMdmmStoneScissorsCloth::operator QMdmmData::StoneScissorsCloth() const
{
    return m_type;
}
