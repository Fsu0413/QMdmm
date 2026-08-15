// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMAGENT_H
#define QMDMMAGENT_H

#include "qmdmmnetworkingglobal.h"

#include <QMdmmData>
#include <QMdmmProtocol>

#include <QObject>

QMDMM_EXPORT_NAME(QMdmmAgent)

namespace QMdmmNetworking {

#ifndef DOXYGEN
namespace p {
struct AgentP;
}
#endif

#ifndef DOXYGEN
namespace v0 {
#endif

class Socket;

// no "final" here since it is inherited.

class QMDMMNETWORKING_EXPORT Agent : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString screenName READ screenName WRITE setScreenName NOTIFY screenNameChanged FINAL)
    Q_PROPERTY(QMdmmCore::Data::AgentState state READ state WRITE setState NOTIFY stateChanged FINAL)

public:
    Q_DISABLE_COPY_MOVE(Agent);

    explicit Agent(const QString &name, QObject *parent = nullptr);
    ~Agent() override;

    // properties
    [[nodiscard]] QString screenName() const;
    void setScreenName(const QString &name);

    [[nodiscard]] QMdmmCore::Data::AgentState state() const;
    void setState(const QMdmmCore::Data::AgentState &state);

signals:
    void screenNameChanged(const QString &, QPrivateSignal);
    void stateChanged(QMdmmCore::Data::AgentState, QPrivateSignal);

#ifndef DOXYGEN
private:
    const std::unique_ptr<p::AgentP> d;
#endif
};

#ifndef DOXYGEN
} // namespace v0
inline namespace v1 {
using v0::Agent;
}
#endif
} // namespace QMdmmNetworking

#endif
