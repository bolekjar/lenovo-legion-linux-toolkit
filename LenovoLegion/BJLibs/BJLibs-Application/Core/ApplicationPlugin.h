// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#define APPLICATION_PLUGIN_DEF(plugin,pluginFactoryName) \
extern "C" {                    \
plugin*  pluginFactoryName();   \
}                               \

#define APPLICATION_PLUGIN_DEC(plugin,pluginFactoryName) \
plugin*  pluginFactoryName()



#define APPLICATION_PLUGIN_DEF_PARAM1(plugin,pluginFactoryName,p1) \
extern "C" {                    \
    plugin*  pluginFactoryName(p1);   \
}                               \

#define APPLICATION_PLUGIN_DEC_PARAM1(plugin,pluginFactoryName,p1) \
    plugin*  pluginFactoryName(p1)


#define APPLICATION_PLUGIN_DEF_PARAM2(plugin,pluginFactoryName,p1,p2) \
extern "C" {                    \
    plugin*  pluginFactoryName(p1,p2);   \
}                               \

#define APPLICATION_PLUGIN_DEC_PARAM2(plugin,pluginFactoryName,p1,p2) \
    plugin*  pluginFactoryName(p1,p2)


#define APPLICATION_PLUGIN_DEF_PARAM3(plugin,pluginFactoryName,p1,p2,p3) \
extern "C" {                    \
    plugin*  pluginFactoryName(p1,p2,p3);   \
}                               \

#define APPLICATION_PLUGIN_DEC_PARAM3(plugin,pluginFactoryName,p1,p2,p3) \
    plugin*  pluginFactoryName(p1,p2,p3)
