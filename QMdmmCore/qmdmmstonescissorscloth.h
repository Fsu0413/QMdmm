#ifndef QMDMMSTONESCISSORSCLOTH_H
#define QMDMMSTONESCISSORSCLOTH_H

#include "qmdmmcoreglobal.h"
#include <vector>

using std::vector;

class QMDMMCORE_EXPORT QMdmmStoneScissorsCloth
{
public:
    enum Type
    {
        Undefined = 0,

        Stone,
        Scissors,
        Cloth
    };

    QMdmmStoneScissorsCloth();
    QMdmmStoneScissorsCloth(Type type);

    QMdmmStoneScissorsCloth(const QMdmmStoneScissorsCloth &other);
    QMdmmStoneScissorsCloth &operator=(const QMdmmStoneScissorsCloth &other);
    QMdmmStoneScissorsCloth &operator=(Type type);

    ~QMdmmStoneScissorsCloth();

    bool operator>(const QMdmmStoneScissorsCloth &op2) const;
    bool operator==(const QMdmmStoneScissorsCloth &op2) const;

    static vector<vector<QMdmmStoneScissorsCloth>::size_type> winners(const vector<QMdmmStoneScissorsCloth> &judgers);

    operator Type() const;

private:
    Type m_type;
};

#endif
