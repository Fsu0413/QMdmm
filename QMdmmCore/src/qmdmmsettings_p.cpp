// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmsettings_p.h"
#include "qmdmmsettings.h"

#include <QDir>
#include <QSettings>

namespace QMdmmCore {

namespace p {

// for reading global / per-user configuration, QSettings is the suitable way
// for specified configuration, QVariantMap is more proper since QSettings always autosaves configuration (This is not desired behavior!)

// QSettings is NOT some type of QVariantMap, where it supports grouping, and some API names are different (insert vs. setValue)
// But it is needed to provide a unique API on top of QVariantMap and QSettings, where the API name difference should go away.
// For QVariantMap the grouping shoule be implemented by ourselves (not so complex, a few lines of code only).

// The unique API is following QMdmmSettingsWrapperPrivate

SettingsWrapperP::~SettingsWrapperP() = default;

QSettingsWrapperP::QSettingsWrapperP(const QString &organization, const QString &application)
    : settings(organization, application)
{
}

QSettingsWrapperP::QSettingsWrapperP(QSettings::Scope scope, const QString &organization, const QString &application)
    : settings(scope, organization, application)
{
}

QSettingsWrapperP::QSettingsWrapperP(QSettings::Format format, QSettings::Scope scope, const QString &organization, const QString &application)
    : settings(format, scope, organization, application)
{
}

QSettingsWrapperP::QSettingsWrapperP(const QString &fileName, QSettings::Format format)
    : settings(fileName, format)
{
}

QSettingsWrapperP::QSettingsWrapperP() = default;

QSettingsWrapperP::QSettingsWrapperP(QSettings::Scope scope)
    : settings(scope)
{
}

QSettingsWrapperP::~QSettingsWrapperP() = default;

void QSettingsWrapperP::setValue(const QString &key, const QVariant &value)
{
    settings.setValue(key, value);
}

QVariant QSettingsWrapperP::value(const QString &key, const QVariant &defaultValue) const
{
    return settings.value(key, defaultValue);
}

void QSettingsWrapperP::beginGroup(const QString &prefix)
{
    settings.beginGroup(prefix);
}

void QSettingsWrapperP::endGroup()
{
    settings.endGroup();
}

QString QSettingsWrapperP::group() const
{
    return settings.group();
}

bool QSettingsWrapperP::contains(const QString &key) const
{
    return settings.contains(key);
}

QVariantMapWrapperP::~QVariantMapWrapperP() = default;

void QVariantMapWrapperP::setValue(const QString &key, const QVariant &value)
{
    map.insert(keyWithGroup(key), value);
}

QVariant QVariantMapWrapperP::value(const QString &key, const QVariant &defaultValue) const
{
    return map.value(keyWithGroup(key), defaultValue);
}

void QVariantMapWrapperP::beginGroup(const QString &prefix)
{
    currentGroup.append(prefix);
}

void QVariantMapWrapperP::endGroup()
{
    Q_ASSERT(!currentGroup.isEmpty());
    currentGroup.removeLast();
}

QString QVariantMapWrapperP::group() const
{
    return currentGroup.join(QStringLiteral("/"));
}

bool QVariantMapWrapperP::contains(const QString &key) const
{
    return map.contains(keyWithGroup(key));
}

QString QVariantMapWrapperP::keyWithGroup(const QString &key) const
{
    QStringList groupPlusKey = currentGroup;
    groupPlusKey.append(key);

    return groupPlusKey.join(QStringLiteral("/"));
}

namespace {
std::once_flag qSettingsInitialized;

void initializeQSettings()
{
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, QStringLiteral(QMDMM_CONFIGURATION_PREFIX));
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QDir::home().absoluteFilePath(QStringLiteral(".QMdmm")));
}
} // namespace

SettingsP::SettingsP()
    : globalConfig(nullptr)
    , userConfig(nullptr)
    , specifiedConfig(nullptr)
{
    std::call_once(qSettingsInitialized, &initializeQSettings);

    globalConfig = new QSettingsWrapperP(QSettings::SystemScope, QStringLiteral("Fsu0413.me"), QStringLiteral("QMdmm"));
    userConfig = new QSettingsWrapperP(QSettings::UserScope, QStringLiteral("Fsu0413.me"), QStringLiteral("QMdmm"));
    specifiedConfig = new QVariantMapWrapperP;
}

SettingsP::~SettingsP()
{
    delete globalConfig;
    delete userConfig;
    delete specifiedConfig;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
QSettings::Status SettingsP::saveConfig(Settings::Instance instance)
{
    Q_ASSERT((instance == Settings::Global) || (instance == Settings::PerUser));

    QSettings *toBeSaved = nullptr;

    switch (instance) {
    case Settings::Global:
        toBeSaved = &globalConfig->settings;
        break;
    case Settings::PerUser:
        toBeSaved = &userConfig->settings;
        break;
    default:
        break;
    }

    Q_ASSERT(toBeSaved != nullptr);

    // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
    if (!toBeSaved->isWritable()) {
        qWarning("Setting file is not writable. Saving the file may fail."
#ifndef Q_OS_WIN
                 " Please try running this program with 'sudo'."
#endif
        );
    }

    // Do not check the current group for now.
    // It is not thread safe popping the group then pushing it again
    // TODO: find a thread-safe way (lock?) to ignore the group on toBeSaved

    foreach (const QString &key, specifiedConfig->map.keys())
        toBeSaved->setValue(key, specifiedConfig->value(key, {}));

    return toBeSaved->status();
}

} // namespace p

} // namespace QMdmmCore
