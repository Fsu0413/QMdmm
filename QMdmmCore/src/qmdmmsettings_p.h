// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef QMDMMSETTINGS_P
#define QMDMMSETTINGS_P

#include "qmdmmsettings.h"

// NOLINTBEGIN(misc-non-private-member-variables-in-classes): This is private header

namespace QMdmmCore {

struct QMDMMCORE_PRIVATE_EXPORT QMdmmSettingsWrapperPrivate
{
    virtual ~QMdmmSettingsWrapperPrivate();

    virtual void setValue(const QString &key, const QVariant &value) = 0;
    [[nodiscard]] virtual QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const = 0;

    virtual void beginGroup(const QString &prefix) = 0;
    virtual void endGroup() = 0;
    [[nodiscard]] virtual QString group() const = 0;

    [[nodiscard]] virtual bool contains(const QString &key) const = 0;
};

struct QMDMMCORE_PRIVATE_EXPORT QMdmmQSettingsWrapperPrivate : public QMdmmSettingsWrapperPrivate
{
    QSettings settings;

    explicit QMdmmQSettingsWrapperPrivate(const QString &organization, const QString &application = QString());
    QMdmmQSettingsWrapperPrivate(QSettings::Scope scope, const QString &organization, const QString &application = QString());
    QMdmmQSettingsWrapperPrivate(QSettings::Format format, QSettings::Scope scope, const QString &organization, const QString &application = QString());
    QMdmmQSettingsWrapperPrivate(const QString &fileName, QSettings::Format format);
    QMdmmQSettingsWrapperPrivate();
    explicit QMdmmQSettingsWrapperPrivate(QSettings::Scope scope);
    ~QMdmmQSettingsWrapperPrivate() override;

    void setValue(const QString &key, const QVariant &value) override;
    [[nodiscard]] QVariant value(const QString &key, const QVariant &defaultValue) const override;
    void beginGroup(const QString &prefix) override;
    void endGroup() override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] bool contains(const QString &key) const override;
};

struct QMDMMCORE_PRIVATE_EXPORT QMdmmQVariantMapWrapperPrivate : public QMdmmSettingsWrapperPrivate
{
    QVariantMap map;
    QStringList currentGroup;

    ~QMdmmQVariantMapWrapperPrivate() override;

    void setValue(const QString &key, const QVariant &value) override;
    [[nodiscard]] QVariant value(const QString &key, const QVariant &defaultValue) const override;
    void beginGroup(const QString &prefix) override;
    void endGroup() override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] bool contains(const QString &key) const override;

    [[nodiscard]] QString keyWithGroup(const QString &key) const;
};

struct QMDMMCORE_PRIVATE_EXPORT QMdmmSettingsPrivate
{
    QMdmmQSettingsWrapperPrivate *globalConfig;
    QMdmmQSettingsWrapperPrivate *userConfig;
    QMdmmQVariantMapWrapperPrivate *specifiedConfig;

    QMdmmSettingsPrivate();
    ~QMdmmSettingsPrivate();

    QSettings::Status saveConfig(QMdmmSettings::Instance toBeSaved);
};

} // namespace QMdmmCore

// NOLINTEND(misc-non-private-member-variables-in-classes): This is private header

#endif
