// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "../Settings.h"

#include <ModuleTypes.h>

namespace LenovoLegionDaemon {

/*
 * Settings of one daemon module, group in the daemon settings file (the same file and groups as before modules)
 */
class ModuleSettingsStorage : protected Settings,
                              public Module::ISettingsStorage
{
public:

    explicit ModuleSettingsStorage(const QString& group);

    bool     contains(const QString& key) const                       override;
    QVariant value(const QString& key,const QVariant& def = {}) const  override;
    void     setValue(const QString& key,const QVariant& value)       override;
    void     remove(const QString& key)                               override;
};

}
