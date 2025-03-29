// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSERVER_H
#define QMDMMSERVER_H

#include "qmdmmnetworkingglobal.h"

#include <QMdmmProtocol>
#include <QMdmmRoom>

#include <QJsonObject>
#include <QObject>
#include <QString>

#include <cstdint>

QMDMM_EXPORT_NAME(QMdmmServerConfiguration)
QMDMM_EXPORT_NAME(QMdmmServer)

struct QMDMMNETWORKING_EXPORT QMdmmServerConfiguration final : public QJsonObject
{
    Q_GADGET
    Q_PROPERTY(bool tcpEnabled READ tcpEnabled WRITE setTcpEnabled DESIGNABLE false FINAL)
    Q_PROPERTY(uint16_t tcpPort READ tcpPort WRITE setTcpPort DESIGNABLE false FINAL)
    Q_PROPERTY(bool localEnabled READ localEnabled WRITE setLocalEnabled DESIGNABLE false FINAL)
    Q_PROPERTY(QString localSocketName READ localSocketName WRITE setLocalSocketName DESIGNABLE false FINAL)
    Q_PROPERTY(bool websocketEnabled READ websocketEnabled WRITE setWebsocketEnabled DESIGNABLE false FINAL)
    Q_PROPERTY(QString websocketName READ websocketName WRITE setWebsocketName DESIGNABLE false FINAL)
    Q_PROPERTY(uint16_t websocketPort READ websocketPort WRITE setWebsocketPort DESIGNABLE false FINAL)

public:
    static QMDMMNETWORKING_EXPORT const QMdmmServerConfiguration &defaults();

#ifdef Q_MOC_RUN
    Q_INVOKABLE QMdmmServerConfiguration();
    Q_INVOKABLE QMdmmServerConfiguration(const QMdmmServerConfiguration &);
#else
    using QJsonObject::QJsonObject;
    using QJsonObject::operator=;
#endif

    // NOLINTBEGIN(bugprone-macro-parentheses)

#define DEFINE_CONFIGURATION(type, valueName, ValueName) \
    [[nodiscard]] type valueName() const;                \
    void set##ValueName(type valueName);
#define DEFINE_CONFIGURATION2(type, valueName, ValueName) \
    [[nodiscard]] type valueName() const;                 \
    void set##ValueName(const type &valueName);

    // NOLINTEND(bugprone-macro-parentheses)

    DEFINE_CONFIGURATION(bool, tcpEnabled, TcpEnabled)
    DEFINE_CONFIGURATION(uint16_t, tcpPort, TcpPort)
    DEFINE_CONFIGURATION(bool, localEnabled, LocalEnabled)
    DEFINE_CONFIGURATION2(QString, localSocketName, LocalSocketName)
    DEFINE_CONFIGURATION(bool, websocketEnabled, WebsocketEnabled)
    DEFINE_CONFIGURATION2(QString, websocketName, WebsocketName)
    DEFINE_CONFIGURATION(uint16_t, websocketPort, WebsocketPort)

#undef DEFINE_CONFIGURATION2
#undef DEFINE_CONFIGURATION
};

class QMdmmServerPrivate;

class QMDMMNETWORKING_EXPORT QMdmmServer : public QObject
{
    Q_OBJECT

public:
    explicit QMdmmServer(QMdmmServerConfiguration serverConfiguration, QMdmmCore::LogicConfiguration logicConfiguration, QObject *parent = nullptr);
    ~QMdmmServer() override;

public slots: // NOLINT(readability-redundant-access-specifiers)
    bool listen();

private:
    // QMdmmServerPrivate is QObject. QPointer can't be used since it is incomplete here
    QMdmmServerPrivate *const d;
    Q_DISABLE_COPY_MOVE(QMdmmServer);
};

#endif // QMDMMSERVER_H
