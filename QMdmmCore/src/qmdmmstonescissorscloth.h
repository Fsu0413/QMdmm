// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSTONESCISSORSCLOTH_H
#define QMDMMSTONESCISSORSCLOTH_H

#include "qmdmmcoreglobal.h"
#include <QList>

class QMDMMCORE_EXPORT QMdmmStoneScissorsCloth
{
public:
    QMdmmStoneScissorsCloth();
    /* implicit */ QMdmmStoneScissorsCloth(QMdmmData::StoneScissorsCloth type);

    QMdmmStoneScissorsCloth(const QMdmmStoneScissorsCloth &other);
    QMdmmStoneScissorsCloth &operator=(const QMdmmStoneScissorsCloth &other);
    QMdmmStoneScissorsCloth &operator=(QMdmmData::StoneScissorsCloth type);

    ~QMdmmStoneScissorsCloth();

    bool operator>(const QMdmmStoneScissorsCloth &op2) const;
    bool operator==(const QMdmmStoneScissorsCloth &op2) const;

    static QList<int> winners(const QList<QMdmmStoneScissorsCloth> &judgers);

    /* implicit */ operator QMdmmData::StoneScissorsCloth() const;

private:
    QMdmmData::StoneScissorsCloth m_type;
};

#endif
