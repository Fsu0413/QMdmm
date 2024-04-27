// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMAGENT_H
#define QMDMMAGENT_H

#include "qmdmmcoreglobal.h"
#include <QObject>

class QIODevice;
struct QMdmmAgentPrivate;

class QMDMMCORE_EXPORT QMdmmAgent : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString screenName READ screenName WRITE setScreenName NOTIFY screenNameChanged FINAL)

public:
    explicit QMdmmAgent(QString name, QObject *parent = nullptr);
    ~QMdmmAgent() override;

    // protocol
    void setStream(QIODevice *stream);

    // properties
    [[nodiscard]] QString screenName() const;
    void setScreenName(const QString &name);

public slots: // NOLINT(readability-redundant-access-specifiers)
    void streamReadyRead();

signals:
    void screenNameChanged(const QString &, QPrivateSignal);

private:
    QMdmmAgentPrivate *const d;
    Q_DISABLE_COPY(QMdmmAgent)
};

#endif
