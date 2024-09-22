// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSERVER_H
#define QMDMMSERVER_H

#include "qmdmmserverglobal.h"

#include <QMdmmProtocol>

#include <QObject>
#include <QString>

#include <cstdint>

struct QMDMMSERVER_EXPORT QMdmmServerConfiguration final
{
    uint16_t tcpPort = 6366U;
    QString localSocketName = QStringLiteral("QMdmm");
};

class QMdmmServerPrivate;

class QMDMMSERVER_EXPORT QMdmmServer : public QObject
{
    Q_OBJECT

public:
    QMdmmServer(const QMdmmServerConfiguration &serverConfiguration, QObject *parent = nullptr);
    ~QMdmmServer() override;

private:
    QMdmmServerPrivate *const d;
};

#endif // QMDMMSERVER_H
