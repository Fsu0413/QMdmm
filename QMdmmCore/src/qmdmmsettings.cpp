// SPDX-License-Identifier: AGPL-3.0-or-later

#include "qmdmmsettings.h"
#include "qmdmmsettings_p.h"

#include <QGlobalStatic>
#include <QSettings>

using namespace QMdmmCore::p;

namespace QMdmmCore {

#ifndef DOXYGEN
namespace v0 {
#else
inline namespace v1 {
#endif

#ifndef DOXYGEN

// This can't be simply put into the private source file, since the instance is static so...

namespace {
// NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
Q_GLOBAL_STATIC(SettingsP, d)
} // namespace

#endif

/**
 * @class Settings
 * @brief The configuration files maintained by MDMM game or system
 *
 * This setting class maintains 3 configuration instance, in where one for global default (@c Settings::Global),
 * another one for per-user default (@c Settings::PerUser), and remained one for user specified (@c Settings::Specified).
 *
 * Global default and per-user default configurations are loaded when initialized. The specified one is initialized empty.
 *
 * The setter only sets the specified one. Other settings should be explicitly saved.
 *
 * The specified one is read by default, then the per-user default one, last the global default one.
 */

/**
 * @enum Settings::Instance
 * @brief The identifier of the 3 instances
 */

/**
 * @var Settings::Instance Settings::Global
 * @brief Global default configuration
 *
 * @var Settings::Instance Settings::PerUser
 * @brief Per-user default configuration
 *
 * @var Settings::Instance Settings::Specified
 * @brief User specified configuration
 */

/**
 * @brief ctor.
 */
Settings::Settings() = default;

/**
 * @brief dtor.
 */
Settings::~Settings() = default;

/**
 * @brief Save current configuration
 * @param instance The instance to be saved
 * @return the status as <tt>QSettings::Status</tt>
 *
 * This is the only method to modify global / per-user settings, which @c setValue() don't.
 *
 * @warning Calling this method while one or more group is active is undefined behavior.
 */
QSettings::Status Settings::saveConfig(Instance instance)
{
    Q_ASSERT(instance != Specified);

    return d->saveConfig(instance);
}

/**
 * @brief Set the specified value
 * @param key Key.
 * @param value Value.
 *
 * This can only modify modifies @c Settings::Specified one.
 *
 * @note To prevent unwanted configuration file saving, <tt>setValue(Instance)</tt> won't be implemented or supported.
 */
void Settings::setValue(const QString &key, const QVariant &value)
{
    d->specifiedConfig->setValue(key, value);
}

/**
 * @brief Get a value by key
 * @param key Key.
 * @param defaultValue The default value to be returned if the key does not exist on all 3 instances
 * @return The set value.
 */
QVariant Settings::value(const QString &key, const QVariant &defaultValue) const
{
    return d->specifiedConfig->value(key, d->userConfig->value(key, d->globalConfig->value(key, defaultValue)));
}

/**
 * @brief Get a value by key from a specific instance
 * @param instance The specified instance
 * @param key Key.
 * @param defaultValue The default value to be returned if the key does not exist
 * @return The set value.
 */
QVariant Settings::value(Instance instance, const QString &key, const QVariant &defaultValue) const
{
    SettingsWrapperP *c = nullptr;
    switch (instance) {
    case Global:
        c = d->globalConfig;
        break;
    case PerUser:
        c = d->userConfig;
        break;
    case Specified:
        c = d->specifiedConfig;
        break;
    default:
        break;
    }

    Q_ASSERT(c != nullptr);

    return c->value(key, defaultValue);
}

/**
 * @brief Replicates @c QSettings::beginGroup() on all 3 configuration instances
 * @param prefix
 *
 * @note To reduce desync, neither <tt>beginGroup(Instance)</tt> nor <tt>endGroup(Instance)</tt> will implemented or supported.
 */
void Settings::beginGroup(const QString &prefix)
{
    d->globalConfig->beginGroup(prefix);
    d->userConfig->beginGroup(prefix);
    d->specifiedConfig->beginGroup(prefix);
}

/**
 * @brief Replicates @c QSettings::endGroup() on all 3 configuration instances
 *
 * @note To reduce desync, neither <tt>beginGroup(Instance)</tt> nor <tt>endGroup(Instance)</tt> will implemented or supported.
 */
void Settings::endGroup()
{
    d->specifiedConfig->endGroup();
    d->userConfig->endGroup();
    d->globalConfig->endGroup();
}

/**
 * @brief Judge if a specific key is configured in either of the instances
 * @param key Key.
 * @return If @c key is set
 */
bool Settings::contains(const QString &key) const
{
    return d->specifiedConfig->contains(key) || d->userConfig->contains(key) || d->globalConfig->contains(key);
}

/**
 * @brief Judge if a specific key is configured in the specified instance
 * @param instance The specified instance
 * @param key Key.
 * @return If @c key is set
 */
bool Settings::contains(Instance instance, const QString &key) const
{
    SettingsWrapperP *c = nullptr;
    switch (instance) {
    case Global:
        c = d->globalConfig;
        break;
    case PerUser:
        c = d->userConfig;
        break;
    case Specified:
        c = d->specifiedConfig;
        break;
    default:
        break;
    }

    Q_ASSERT(c != nullptr);

    return c->contains(key);
}

} // namespace v0
} // namespace QMdmmCore
