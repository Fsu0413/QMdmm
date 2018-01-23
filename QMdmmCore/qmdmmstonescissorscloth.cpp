#include "qmdmmstonescissorscloth.h"

#include <map>
using std::map;

QMdmmStoneScissorsCloth::QMdmmStoneScissorsCloth()
    : m_type(Undefined)
{
}

QMdmmStoneScissorsCloth::QMdmmStoneScissorsCloth(QMdmmStoneScissorsCloth::Type type)
    : m_type(type)
{
}

QMdmmStoneScissorsCloth::QMdmmStoneScissorsCloth(const QMdmmStoneScissorsCloth &other)
    : m_type(other.m_type)
{
}

QMdmmStoneScissorsCloth &QMdmmStoneScissorsCloth::operator=(const QMdmmStoneScissorsCloth &other)
{
    m_type = other.m_type;
    return *this;
}

QMdmmStoneScissorsCloth &QMdmmStoneScissorsCloth::operator=(QMdmmStoneScissorsCloth::Type type)
{
    m_type = type;
    return *this;
}

QMdmmStoneScissorsCloth::~QMdmmStoneScissorsCloth()
{
}

bool QMdmmStoneScissorsCloth::operator>(const QMdmmStoneScissorsCloth &op2) const
{
    return (m_type == Stone && op2.m_type == Scissors) || (m_type == Scissors && op2.m_type == Cloth) || (m_type == Cloth && op2.m_type == Stone);
}

bool QMdmmStoneScissorsCloth::operator==(const QMdmmStoneScissorsCloth &op2) const
{
    return m_type == op2.m_type && m_type != Undefined;
}

vector<vector<QMdmmStoneScissorsCloth>::size_type> QMdmmStoneScissorsCloth::winners(const vector<QMdmmStoneScissorsCloth> &judgers)
{
    map<Type, vector<vector<QMdmmStoneScissorsCloth>::size_type>> judgersMap;

    for (vector<QMdmmStoneScissorsCloth>::size_type i = 0; i < judgers.size(); ++i) {
        const auto &judger = judgers.at(i);
        Type t = judger;
        if (t != Undefined)
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

    return vector<vector<QMdmmStoneScissorsCloth>::size_type>();
}

QMdmmStoneScissorsCloth::operator QMdmmStoneScissorsCloth::Type() const
{
    return m_type;
}
