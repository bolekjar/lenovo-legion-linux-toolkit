// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <Core/ApplicationPlugin.h>
#include <Core/ApplicationModulesLoader.h>

#define CONNECTX_GAME__QUOTE(name)              #name
#define CONNECTX_GAME__STR(macro)               CONNECTX_GAME__QUOTE(macro)

#define CONNECTX_GAME_FACTORY                   connectXGameFactory
#define CONNECTX_GAME_FACTORY_NAME              CONNECTX_GAME__STR(CONNECTX_GAME_FACTORY)

#define CONNECTX_GAME_FACTORY_INFO              connectXGameInfoFactory
#define CONNECTX_GAME_FACTORY_INFO_NAME         CONNECTX_GAME__STR(CONNECTX_GAME_FACTORY_INFO)

#define CONNECTX_GAME_FACTORY_PLUGIN_DEF        APPLICATION_PLUGIN_DEF(bj::connectX::ConnectXGameSimulation,CONNECTX_GAME_FACTORY)
#define CONNECTX_GAME_FACTORY_PLUGIN_DEC        APPLICATION_PLUGIN_DEC(bj::connectX::ConnectXGameSimulation,CONNECTX_GAME_FACTORY)

#define CONNECTX_GAME_FACTORY_INFO_PLUGIN_DEF   APPLICATION_PLUGIN_DEF(bj::connectX::ConnectXGameInfo,CONNECTX_GAME_FACTORY_INFO)
#define CONNECTX_GAME_FACTORY_INFO_PLUGIN_DEC   APPLICATION_PLUGIN_DEC(bj::connectX::ConnectXGameInfo,CONNECTX_GAME_FACTORY_INFO)

#define CONNECTX_GAME_MODULE_SIGNATURE          bj::connectX::ConnectXGameInfo*(),bj::connectX::ConnectXGameSimulation*()
#define CONNECTX_GAME_MODULE_FACTORIES_NAME     {CONNECTX_GAME_FACTORY_INFO_NAME,CONNECTX_GAME_FACTORY_NAME}

namespace bj { namespace connectX {

class ConnectXGameInfo;
class ConnectXGameSimulation;

using ConnectXGameModules            = framework::ApplicationModulesLoader::Modules<CONNECTX_GAME_MODULE_SIGNATURE>;
using ConnectXGameModuleGetGameInfo  = framework::ApplicationModulesLoader::ModuleItemGet<0,CONNECTX_GAME_MODULE_SIGNATURE>;
using ConnectXGameModuleGetGame      = framework::ApplicationModulesLoader::ModuleItemGet<1,CONNECTX_GAME_MODULE_SIGNATURE>;


}}
