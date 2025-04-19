// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSETTINGS_H
#define QMDMMSETTINGS_H

#include "qmdmmcoreglobal.h"

#include <QSettings>
#include <QVariant>

#include <memory>

QMDMM_EXPORT_NAME(QMdmmSettings)

namespace QMdmmCore {

#ifndef DOXYGEN
namespace v0 {
#else
inline namespace v1 {
#endif

class QMDMMCORE_EXPORT Settings
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

    Settings();
    ~Settings();

    QSettings::Status saveConfig(Instance instance);

    void setValue(const QString &key, const QVariant &value);
    [[nodiscard]] QVariant value(const QString &key, const QVariant &defaultValue = {}) const;
    [[nodiscard]] QVariant value(Instance instance, const QString &key, const QVariant &defaultValue = {}) const;

    void beginGroup(const QString &prefix);
    void endGroup();

    [[nodiscard]] bool contains(const QString &key) const;
    [[nodiscard]] bool contains(Instance instance, const QString &key) const;

private:
    Q_DISABLE_COPY_MOVE(Settings);
};
} // namespace v0

#ifndef DOXYGEN
inline namespace v1 {
using v0::Settings;
}
#endif

} // namespace QMdmmCore

#endif
