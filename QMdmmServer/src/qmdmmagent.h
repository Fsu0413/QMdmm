// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMAGENT_H
#define QMDMMAGENT_H

#include "qmdmmserverglobal.h"

#include <QMdmmCore/QMdmmProtocol>
#include <QObject>

class QMdmmSocket;

class QMdmmAgentPrivate;

class QMDMMSERVER_EXPORT QMdmmAgent : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString screenName READ screenName WRITE setScreenName NOTIFY screenNameChanged FINAL)
    Q_PROPERTY(bool ready READ ready WRITE setReady NOTIFY readyChanged FINAL)

public:
    explicit QMdmmAgent(QString name, QObject *parent = nullptr);
    ~QMdmmAgent() override;

    // properties
    [[nodiscard]] QString screenName() const;
    void setScreenName(const QString &name);

    [[nodiscard]] bool ready() const;
    void setReady(bool ready);

    void setSocket(QMdmmSocket *socket);

signals:
    void screenNameChanged(const QString &, QPrivateSignal);
    void readyChanged(bool, QPrivateSignal);

    void sendPacket(QMdmmPacket packet);

private:
    QMdmmAgentPrivate *const d;
    Q_DISABLE_COPY(QMdmmAgent)
};

#endif
