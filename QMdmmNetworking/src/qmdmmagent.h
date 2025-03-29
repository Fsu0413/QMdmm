// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMAGENT_H
#define QMDMMAGENT_H

#include "qmdmmnetworkingglobal.h"

#include <QMdmmData>
#include <QMdmmProtocol>

#include <QObject>

QMDMM_EXPORT_NAME(QMdmmAgent)

class QMdmmSocket;

struct QMdmmAgentPrivate;

// no "final" here since it is inherited.

class QMDMMNETWORKING_EXPORT QMdmmAgent : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString screenName READ screenName WRITE setScreenName NOTIFY screenNameChanged FINAL)
    Q_PROPERTY(QMdmmCore::Data::AgentState state READ state WRITE setState NOTIFY stateChanged FINAL)

public:
    explicit QMdmmAgent(const QString &name, QObject *parent = nullptr);
    ~QMdmmAgent() override;

    // properties
    [[nodiscard]] QString screenName() const;
    void setScreenName(const QString &name);

    [[nodiscard]] QMdmmCore::Data::AgentState state() const;
    void setState(const QMdmmCore::Data::AgentState &state);

signals:
    void screenNameChanged(const QString &, QPrivateSignal);
    void stateChanged(QMdmmCore::Data::AgentState, QPrivateSignal);

private:
    const std::unique_ptr<QMdmmAgentPrivate> d;
    Q_DISABLE_COPY_MOVE(QMdmmAgent);
};

#endif
