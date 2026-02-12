// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <Core/Logger.h>
#include <Core/StackTrace.h>

#include <Singleton/CPP11ThreadModel.h>
#include <Singleton/StaticCreationPolicy.h>
#include <Singleton/SingletonHolder.h>


using LoggerHolder = bj::framework::SingletonHolder<bj::framework::Logger,bj::framework::StaticCreationPolicy,bj::framework::DefaultThreadModel>;

#define LOG_I(x)  LOG_INFO(x,LoggerHolder);
#define LOG_D(x)  LOG_DEBUG(x,LoggerHolder);
#define LOG_E(x)  LOG_ERROR(x,LoggerHolder);
#define LOG_W(x)  LOG_WARNING(x,LoggerHolder);
#define LOG_T(x)  LOG_TRACE(x,LoggerHolder);
