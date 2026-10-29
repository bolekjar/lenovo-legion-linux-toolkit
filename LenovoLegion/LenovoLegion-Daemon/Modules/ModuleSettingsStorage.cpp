// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ModuleSettingsStorage.h"

namespace LenovoLegionDaemon {

ModuleSettingsStorage::ModuleSettingsStorage(const QString &group) :
    Settings(group)
{}

bool ModuleSettingsStorage::contains(const QString &key) const
{
    return m_settings.contains(key);
}

QVariant ModuleSettingsStorage::value(const QString &key, const QVariant &def) const
{
    return m_settings.value(key,def);
}

void ModuleSettingsStorage::setValue(const QString &key, const QVariant &value)
{
    m_settings.setValue(key,value);
}

void ModuleSettingsStorage::remove(const QString &key)
{
    m_settings.remove(key);
}

}
