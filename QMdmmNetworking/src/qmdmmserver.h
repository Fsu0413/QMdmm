// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSERVER_H
#define QMDMMSERVER_H

#include "qmdmmnetworkingglobal.h"

#include <QMdmmProtocol>
#include <QMdmmRoom>

#include <QObject>
#include <QString>

#include <cstdint>

struct QMDMMNETWORKING_EXPORT QMdmmServerConfiguration final
{
    bool tcpEnabled = true;
    uint16_t tcpPort = 6366U;
    bool localEnabled = true;
    QString localSocketName = QStringLiteral("QMdmm");
    bool websocketEnabled = true;
    QString websocketName = QStringLiteral("QMdmm");
    uint16_t websocketPort = 6367U;

    QMdmmLogicConfiguration logicConfiguration;
};

class QMdmmServerPrivate;

class QMDMMNETWORKING_EXPORT QMdmmServer : public QObject
{
    Q_OBJECT

public:
    explicit QMdmmServer(const QMdmmServerConfiguration &serverConfiguration, QObject *parent = nullptr);
    ~QMdmmServer() override;

public slots: // NOLINT(readability-redundant-access-specifiers)
    bool listenTcpServer();
    bool listenLocalServer();
    bool listenWebsocketServer();

private:
    QMdmmServerPrivate *const d;
    Q_DISABLE_COPY_MOVE(QMdmmServer);
};

#endif // QMDMMSERVER_H
