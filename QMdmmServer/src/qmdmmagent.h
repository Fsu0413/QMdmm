// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMAGENT_H
#define QMDMMAGENT_H

#include "qmdmmserverglobal.h"

#include <QMdmmProtocol>

#include <QObject>

class QMdmmSocket;

struct QMdmmAgentPrivate;

class QMDMMSERVER_EXPORT QMdmmAgent : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString screenName READ screenName WRITE setScreenName NOTIFY screenNameChanged FINAL)
    Q_PROPERTY(QMdmmProtocol::AgentState state READ state WRITE setState NOTIFY stateChanged FINAL)

public:
    explicit QMdmmAgent(const QString &name, QObject *parent = nullptr);
    ~QMdmmAgent() override;

    // properties
    [[nodiscard]] QString screenName() const;
    void setScreenName(const QString &name);

    [[nodiscard]] QMdmmProtocol::AgentState state() const;
    void setState(QMdmmProtocol::AgentState state);

signals:
    void screenNameChanged(const QString &, QPrivateSignal);
    void stateChanged(QMdmmProtocol::AgentState, QPrivateSignal);

private:
    QMdmmAgentPrivate *const d;
    Q_DISABLE_COPY(QMdmmAgent)
};

#endif
