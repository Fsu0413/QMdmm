// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMAGENT_H
#define QMDMMAGENT_H

#include "qmdmmcoreglobal.h"
#include <QObject>

class QMDMMCORE_EXPORT QMdmmAgent : public QObject
{
    Q_OBJECT

public:
    QMdmmAgent(QObject *parent = nullptr);
    ~QMdmmAgent() override;
};

#endif
