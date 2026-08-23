// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMCLIENT_H
#define QMDMMCLIENT_H

#include "qmdmmnetworkingglobal.h"

#include <QMdmmRoom>

#include <QObject>

QMDMM_EXPORT_NAME(QMdmmClientConfiguration)
QMDMM_EXPORT_NAME(QMdmmClient)

namespace QMdmmNetworking {

#ifndef DOXYGEN
namespace p {
class ClientP;
}
#endif

#ifndef DOXYGEN
namespace v0 {
#endif

class Agent;

struct QMDMMNETWORKING_EXPORT ClientConfiguration final : public QVariantMap
{
    Q_GADGET
    Q_PROPERTY(QString screenName READ screenName WRITE setScreenName DESIGNABLE false FINAL)

public:
    static QMDMMNETWORKING_EXPORT const ClientConfiguration &defaults();

#ifdef Q_MOC_RUN
    Q_INVOKABLE QMdmmClientConfiguration();
    Q_INVOKABLE QMdmmClientConfiguration(const QMdmmClientConfiguration &);
#else
    using QVariantMap::QMap;
    using QVariantMap::operator=;
#endif

    [[nodiscard]] QString screenName() const;
    void setScreenName(const QString &screenName);
};

class QMDMMNETWORKING_EXPORT Client final : public QObject
{
    Q_OBJECT

public:
    Q_DISABLE_COPY_MOVE(Client);

    explicit Client(ClientConfiguration clientConfiguration, QObject *parent = nullptr);
    ~Client() override;

    bool connectToHost(const QString &host, QMdmmCore::Data::AgentState initialState);

    [[nodiscard]] QMdmmCore::Room *room();
    [[nodiscard]] const QMdmmCore::Room *room() const;

    Agent *agent();
    [[nodiscard]] const Agent *agent() const;

public slots: // NOLINT(readability-redundant-access-specifiers)
    // There is no request timer called here. The timeout can be established in UI.
    // This function can be called for a timeout request, to generate a default reply and reply to server
    void requestTimeout();

signals:
    // Emitted once when the connection drops. The client then retries internally;
    // this is the "connection lost" notice for the upper layer (distinct from
    // socketErrorDisconnected, which fires only after the automatic reconnect gives up).
    void socketConnectionLost(const QString &errorString, QPrivateSignal);

    // Emitted when the client re-establishes the connection and re-signed in after
    // a disconnect.
    void socketReconnectSucceeded(QPrivateSignal);

    void socketErrorDisconnected(const QString &errorString, QPrivateSignal);

#ifndef DOXYGEN
private:
    friend class p::ClientP;
    // ClientP is QObject. QPointer can't be used since it is incomplete here
    p::ClientP *const d;
#endif
};

#ifndef DOXYGEN
} // namespace v0

inline namespace v1 {
using v0::Client;
using v0::ClientConfiguration;
} // namespace v1
#endif
} // namespace QMdmmNetworking

#endif
