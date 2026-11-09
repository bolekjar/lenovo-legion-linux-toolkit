/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#ifndef LEGION_KERNEL_VERSION_H
#define LEGION_KERNEL_VERSION_H

#include <linux/version.h>

/*
 * Minimal supported kernel 6.12 (LTS): platform_driver.remove returning void (6.11), wmi_driver.no_singleton (6.10),
 * guard()/scoped_guard()/__free (6.5). Build tested on 6.12, 6.18 and 7.2. Newer APIs are used by version checks
 * (platform_profile ops and custom 6.14, max-power 6.19, rdmsrq_safe_on_cpu 7.2). dkms.conf: BUILD_EXCLUSIVE_KERNEL.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 12, 0)
#error "Lenovo Legion Linux Toolkit kernel modules need kernel 6.12 or newer"
#endif

#endif /* LEGION_KERNEL_VERSION_H */
