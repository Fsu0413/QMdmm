// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSETTINGS_H
#define QMDMMSETTINGS_H

#include "qmdmmcoreglobal.h"

#include <QSettings>
#include <memory>

class QMDMMCORE_EXPORT QMdmmSettings
{
    Q_GADGET

public:
    enum Instance : uint8_t
    {
        Global,
        PerUser,
        Specified,
    };
    Q_ENUM(Instance);

    QMdmmSettings();
    ~QMdmmSettings();

    QSettings::Status saveConfig(Instance instance);

    void setValue(const QString &key, const QVariant &value);
    [[nodiscard]] QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;
    [[nodiscard]] QVariant value(Instance instance, const QString &key, const QVariant &defaultValue = QVariant()) const;

    void beginGroup(const QString &prefix);
    void endGroup();

    [[nodiscard]] bool contains(const QString &key) const;
    [[nodiscard]] bool contains(Instance instance, const QString &key) const;

private:
    Q_DISABLE_COPY_MOVE(QMdmmSettings);
};

#endif
