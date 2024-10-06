// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMCLIENT_H
#define QMDMMCLIENT_H

#include "qmdmmserverglobal.h"

#include <QObject>

class QMdmmClientPrivate;

class QMDMMSERVER_EXPORT QMdmmClient final : public QObject
{
    Q_OBJECT

public:
    explicit QMdmmClient(QObject *parent = nullptr);
    ~QMdmmClient() override;

private:
    QMdmmClientPrivate *const d;
    Q_DISABLE_COPY_MOVE(QMdmmClient);
};

#endif
