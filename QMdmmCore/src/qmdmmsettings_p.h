// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSETTINGS_P
#define QMDMMSETTINGS_P

#include "qmdmmsettings.h"

// NOLINTBEGIN(misc-non-private-member-variables-in-classes): This is private header

namespace QMdmmCore {

namespace p {

struct QMDMMCORE_PRIVATE_EXPORT SettingsWrapperP
{
    virtual ~SettingsWrapperP();

    virtual void setValue(const QString &key, const QVariant &value) = 0;
    [[nodiscard]] virtual QVariant value(const QString &key, const QVariant &defaultValue = {}) const = 0;

    virtual void beginGroup(const QString &prefix) = 0;
    virtual void endGroup() = 0;
    [[nodiscard]] virtual QString group() const = 0;

    [[nodiscard]] virtual bool contains(const QString &key) const = 0;
};

struct QMDMMCORE_PRIVATE_EXPORT QSettingsWrapperP : public SettingsWrapperP
{
    QSettings settings;

    explicit QSettingsWrapperP(const QString &organization, const QString &application = {});
    QSettingsWrapperP(QSettings::Scope scope, const QString &organization, const QString &application = {});
    QSettingsWrapperP(QSettings::Format format, QSettings::Scope scope, const QString &organization, const QString &application = {});
    QSettingsWrapperP(const QString &fileName, QSettings::Format format);
    QSettingsWrapperP();
    explicit QSettingsWrapperP(QSettings::Scope scope);
    ~QSettingsWrapperP() override;

    void setValue(const QString &key, const QVariant &value) override;
    [[nodiscard]] QVariant value(const QString &key, const QVariant &defaultValue) const override;
    void beginGroup(const QString &prefix) override;
    void endGroup() override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] bool contains(const QString &key) const override;
};

struct QMDMMCORE_PRIVATE_EXPORT QVariantMapWrapperP : public SettingsWrapperP
{
    QVariantMap map;
    QStringList currentGroup;

    ~QVariantMapWrapperP() override;

    void setValue(const QString &key, const QVariant &value) override;
    [[nodiscard]] QVariant value(const QString &key, const QVariant &defaultValue) const override;
    void beginGroup(const QString &prefix) override;
    void endGroup() override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] bool contains(const QString &key) const override;

    [[nodiscard]] QString keyWithGroup(const QString &key) const;
};

struct QMDMMCORE_PRIVATE_EXPORT SettingsP
{
    QSettingsWrapperP *globalConfig;
    QSettingsWrapperP *userConfig;
    QVariantMapWrapperP *specifiedConfig;

    SettingsP();
    ~SettingsP();

    QSettings::Status saveConfig(Settings::Instance toBeSaved);
};

} // namespace p

} // namespace QMdmmCore

// NOLINTEND(misc-non-private-member-variables-in-classes): This is private header

#endif
