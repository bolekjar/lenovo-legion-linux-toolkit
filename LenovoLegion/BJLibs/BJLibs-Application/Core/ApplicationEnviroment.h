// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

namespace bj { namespace framework {

/*
 * Define neccessary macros
 */
#define BL_QUOTE(name)              #name
#define BL_STR(macro)               BL_QUOTE(macro)

#define BL_DATA_DIR                 BL_DATA_DIR_DEFINE
#define BL_DATA_DIR_STR             BL_STR(BL_DATA_DIR)

#define BL_LOG_DIR                  BL_LOG_DIR_DEFINE
#define BL_LOG_DIR_STR              BL_STR(BL_LOG_DIR)

#define BL_APP_NAME                 BL_APP_NAME_DEFINE
#define BL_APP_NAME_STR             BL_STR(BL_APP_NAME)

#define BL_APP_NAME2                BL_APP_NAME_DEFINE2
#define BL_APP_NAME2_STR            BL_STR(BL_APP_NAME2)

#define BL_APP_NAME3                BL_APP_NAME_DEFINE3
#define BL_APP_NAME3_STR            BL_STR(BL_APP_NAME3)

#define BL_APP_NAME4                BL_APP_NAME_DEFINE4
#define BL_APP_NAME4_STR            BL_STR(BL_APP_NAME4)

#define BL_APP_NAME5                BL_APP_NAME_DEFINE5
#define BL_APP_NAME5_STR            BL_STR(BL_APP_NAME5)

#define BL_APP_NAME6                BL_APP_NAME_DEFINE6
#define BL_APP_NAME6_STR            BL_STR(BL_APP_NAME6)

#define BL_APP_NAME7                BL_APP_NAME_DEFINE7
#define BL_APP_NAME7_STR            BL_STR(BL_APP_NAME7)

#define BL_APP_NAME8                BL_APP_NAME_DEFINE8
#define BL_APP_NAME8_STR            BL_STR(BL_APP_NAME8)

#define BL_APP_NAME9                BL_APP_NAME_DEFINE9
#define BL_APP_NAME9_STR            BL_STR(BL_APP_NAME9)

#define BL_APP_NAME10               BL_APP_NAME_DEFINE10
#define BL_APP_NAME10_STR           BL_STR(BL_APP_NAME10)


#define BL_MODULES_DIR              BL_MODULES_DIR_DEFINE
#define BL_MODULES_DIR_STR          BL_STR(BL_MODULES_DIR)

#define BL_APP_VERSION              BL_APP_VERSION_DEFINE
#define BL_APP_VERSION_STR          BL_STR(BL_APP_VERSION)

}}
