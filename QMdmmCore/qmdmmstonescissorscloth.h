#ifndef QMDMMSTONESCISSORSCLOTH_H
#define QMDMMSTONESCISSORSCLOTH_H

#include "qmdmmcoreglobal.h"
#include <vector>

using std::vector;

class QMDMMCORE_EXPORT QMdmmStoneScissorsCloth
{
public:
    QMdmmStoneScissorsCloth();
    QMdmmStoneScissorsCloth(QMdmmData::StoneScissorsCloth type);

    QMdmmStoneScissorsCloth(const QMdmmStoneScissorsCloth &other);
    QMdmmStoneScissorsCloth &operator=(const QMdmmStoneScissorsCloth &other);
    QMdmmStoneScissorsCloth &operator=(QMdmmData::StoneScissorsCloth type);

    ~QMdmmStoneScissorsCloth();

    bool operator>(const QMdmmStoneScissorsCloth &op2) const;
    bool operator==(const QMdmmStoneScissorsCloth &op2) const;

    static vector<vector<QMdmmStoneScissorsCloth>::size_type> winners(const vector<QMdmmStoneScissorsCloth> &judgers);

    operator QMdmmData::StoneScissorsCloth() const;

private:
    QMdmmData::StoneScissorsCloth m_type;
};

#endif
