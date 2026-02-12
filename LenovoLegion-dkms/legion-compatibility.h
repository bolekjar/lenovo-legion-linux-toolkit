// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#ifndef LEGION_COMPATIBILITY_H_
#define LEGION_COMPATIBILITY_H_

#include "legion-machine-information.h"


int is_compatible(struct dmi_machine_information* dmi_machine_info);

#endif /* LEGION_COMPATIBILITY_H_ */
