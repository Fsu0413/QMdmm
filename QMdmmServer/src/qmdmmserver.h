// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSERVER_H
#define QMDMMSERVER_H

#include "qmdmmserverglobal.h"

#include <QMdmmLogic>
#include <QMdmmProtocol>

#include <QObject>
#include <QString>

#include <cstdint>

struct QMDMMSERVER_EXPORT QMdmmServerConfiguration final
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

class QMDMMSERVER_EXPORT QMdmmServer : public QObject
{
    Q_OBJECT

public:
    QMdmmServer(const QMdmmServerConfiguration &serverConfiguration, QObject *parent = nullptr);
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
