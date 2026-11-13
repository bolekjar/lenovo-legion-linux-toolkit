# Consolidation of the kernel module lenovo_legion (plan of 2026-11-09)

## Starting decisions

- **Own self-contained stack, no building on top of the upstream drivers** `lenovo-wmi-*` (`drivers/platform/x86/lenovo/`):
  - upstream exists since kernel 6.16 only and changes between versions (rename `capdata01` → `wmi-capdata`, new attributes, hwmon, battery); older kernels (all LTS up to 6.12) have none of it, the module has to work the same on every kernel,
  - only one driver binds a WMI GUID (`no_singleton` means several devices of one driver, not two drivers on one device); the legacy `wmi_evaluate_method(guid)` is deprecated, does not deliver the events and two drivers would overwrite each other's state (cached mode); the exported functions of upstream need its `wmi_device` and change between versions,
  - the shared GUIDs (gamezone, other, capdata00/01, thermal mode event) are therefore held by our module and **blacklisting the upstream `lenovo_wmi_*` is the right solution**.
- The attribute names stay ours (the daemon and the GUI have them mapped), no unification with upstream.
- `legion_intel_msr` stays unchanged (the kernel has no undervolt, the `msr` driver blocks writes to 0x150 without `allow_writes`).

## Inventory (state before the consolidation)

| Part | What it does | Daemon | Duplicate |
|---|---|---|---|
| gamezone | `platform_profile`, sysfs `legion-wmi-gamezone-0` (smart_fan, disable_tp, disable_win_key, gsync, igpu_mode, over_drive, groups fans, cpu, gpu, macro_key, other) | only smart_fan, disable_tp, disable_win_key, gsync, igpu_mode, get_power_charge_mode, get_thermal_mode | thermal mode in three ways (platform_profile, smart_fan, get_thermal_mode); temperatures and fan speeds in hwmon as well |
| other + capdata01 + ddata | CPU/GPU power limits, bool features of capdata00, group others | 11 limits, fan_full_speed, god_mode_fnq_switchable | – |
| fan curve (ftable, fm) | `legion-wmi-fan-mode-0` | yes | – (not in upstream) |
| events | uevents EVENT_TYPE/EVENT_VALUE | yes | – |
| hwmon | temp1-3, fan1-3 | yes | – |
| machine-information | `legion-dmi/machine-information-0` | yes | **all of it in `/sys/class/dmi/id/`** |
| RAPL (`legion-rapl.c`) | the kernel reads and writes the sysfs `/sys/class/powercap/intel-rapl` | – | **`intel_rapl_msr` (powercap), user space** |
| RAPL MMIO (`legion-rapl-mmio.c`) | `ioremap` of the registers of the processor thermal device, PL1/PL2/PL4, lock | – | **`processor_thermal_rapl` → `intel-rapl-mmio:0`**, two writers without a common lock |
| RAPL synchronization (`legion-dkms.c`) | writes the limits of the mode to RAPL and MMIO after a mode change | – | belongs to the daemon |

Bug found during the inventory: `get_ir_temp` (gamezone other) calls the method `GetPowerChargeMode`.

## Plan

1. **Remove the duplicates of the kernel** (the generic interfaces exist on old kernels as well: DMI always, `intel-rapl` since 3.13, `intel-rapl-mmio` since 5.3):
   - machine-information and the class `legion-dmi` → the daemon reads `/sys/class/dmi/id/` (`SysFsDriverLegionMachineInformation`),
   - `legion-rapl.c`, `legion-rapl-mmio.c` and the RAPL synchronization in `legion-dkms.c` → on the uevent of a thermal mode change the daemon writes the limits of the mode to the powercap `intel-rapl` and `intel-rapl-mmio` (values of the mode from capdata01/ddata, already read by the module Other); the custom mode by the current values,
   - verify on the hardware that the same limits apply after a mode change by Fn+Q as before.

   **State (2026-11-09): done, tested on the hardware (Legion Pro 7 16IAX10H).**
   - kernel: removed `legion-rapl.*`, `legion-rapl-mmio.*`, `legion-machine-information-sysfs.*`, `legion-dkms.h` (synchronization notifier), unused members of `legion_data`; the model check by DMI stays,
   - daemon: `SysFsDriverLegionMachineInformation` reads `/sys/class/dmi/id/` (no dependency on the kernel module, `MACHINE_INFORMATION` is not `PLATFORM_LENOVO` any more),
   - daemon: `CPUPower/RaplPowerLimits` writes the limits of the mode (the same rules as the kernel: MMIO the default values of the mode, the current ones in custom, enables MMIO; MSR the maximums of the mode only when RAPL is enabled and the MMIO write succeeded) on the start of the module, when the kernel module is loaded, on the thermal mode event (`ProviderModule::onDriverEvent`) and in the custom mode at once after the limits were set (client request, restored settings),
   - fixed: the provider CPUPower truncated the current values of the limits to 8 bits (`static_cast<quint8>`),
   - the udev rule `99-rapl-readonly.rules` (`rapl-readonly.sh`) stays: the files of the limits are `0440 root:legion` and sysfs refuses to open a file without the `w` bit for writing even for root (other tools, e.g. thermald, do not change the limits); the daemon adds `w` for the owner before the write and restores the permissions at once.
2. **One way for the thermal mode, hwmon, probe:**
   - the thermal mode is set only by `legion_wmi_gz_thermal_mode_set()` (mode checked against the notebook, extreme only with support, WMI, stored mode of the power source); used by `smart_fan/current_value` and `platform_profile`, `platform_profile` reads the mode of the firmware,
   - `smart_fan/current_value` stays the interface of the daemon (the same on all kernels), `platform_profile` is for the other tools (power-profiles-daemon),
   - hwmon by `hwmon_device_register_with_info` (the same names `tempX_*`, `fanX_*`),
   - probe sets nothing, it only reads the state.

   **State (2026-11-09): done, tested on the hardware (Legion Pro 7 16IAX10H).**
   - mapping of the profiles: quiet, balanced, performance the same; extreme = `max-power` (kernel 6.19+), otherwise the hidden `balanced-performance` (6.14+); custom = `custom` (6.14+, newly a visible choice), shown as `balanced-performance` on older kernels,
   - removed: `platform_profile` silently changed performance to balanced on battery (the daemon never did it), custom and extreme could not be set; `smart_fan` accepted 0 and extreme without support,
   - hwmon: no global modifiable attribute tables, registration without devm (removed before the WMI drivers it reads).

3. **Gamezone sysfs**: decision of the author (2026-11-09): the features the daemon does not use stay (groups fans, cpu, gpu, macro_key, other, features of capdata00); fixed `get_ir_temp` (called the method `GetPowerChargeMode` instead of `GetIRTemp`).

   **State (2026-11-09): done.**

4. **Minimal supported kernel: 6.12 LTS** (decision of the author 2026-11-09).
   - API: `platform_driver.remove` returns `void` (6.11), `wmi_driver.no_singleton` (6.10), `guard()`/`scoped_guard()`/`__free` (6.5); newer APIs by version checks (`platform_profile` ops and custom 6.14, max-power 6.19, `rdmsrq_safe_on_cpu` 6.16),
   - `legion-kernel-version.h`: `#error` for a kernel < 6.12, `dkms.conf`: `BUILD_EXCLUSIVE_KERNEL` (DKMS does not build the module for an older kernel), README: kernel 6.12+ required,
   - verified: built without warnings against 6.12.116 and 6.18.62 (sources of kernel.org, `defconfig` + WMI, hwmon, platform_profile, `modules_prepare`), all used symbols are exported in both kernels; loading tested on 7.2.

   **State (2026-11-09): done.** Loading on 6.12 and 6.18 not tested (needs a booted kernel).

5. **No binding to the ACPI device VPC2004** (decision of the author 2026-11-10, after the hardware test of `ideapad_laptop`).
   - test: `ideapad_laptop` has no DYTC on Legion Pro 7 16IAX10H (no own `platform_profile`, no profile cycle on Fn+Q), it supports `conservation_mode`, `charge_types`, `usb_charging`, `fn_lock`, `camera_power`, `fan_mode`; it binds the WMI GUID `8FC0DE0C` (Fn keys) used by the events of the daemon (keyboard backlight, FnLock, camera) → **running together with `ideapad_laptop` is not possible**, it stays blacklisted,
   - the module has no device of its own: `module_init` checks the model by DMI (`dmi_first_match`, formerly `strcmp` on a possible NULL), registers the firmware attributes class and the WMI drivers (components before the masters), `module_exit` in the reverse order; loaded by the aliases of the WMI GUIDs,
   - the hwmon `legion` is a child of the other mode device, registered when its components are bound,
   - fixed: the WMI drivers were registered in `probe` of the platform driver; `MODULE_DEVICE_TABLE(acpi, …)` on WMI GUID tables (bogus aliases `acpi*:300E:*`…), discrete data without a table; hwmon notifiers registered in `bind` by devm (twice after a repeated bind of the components); `probe` of the other mode and fan method waited up to 5 s for the components to bind,
   - removed: `legion-compatibility.*`, `legion-machine-information.*`, `legion-common.c` (DMI dump to dmesg, DMI is in `/sys/class/dmi/id`),
   - blacklist: `lenovo_wmi_hotkey_utilities` removed (mic mute and audio mute LEDs, the module does not use its GUID).

   **State (2026-11-10): done, tested on the hardware (Legion Pro 7 16IAX10H):** the module is loaded by the WMI aliases after a reboot, all 13 WMI GUIDs are held by its drivers, hwmon `legion` as before (sensors: `legion-virtual-0`, WMI bus); `lenovo_wmi_hotkey_utilities` binds, this model has no mute LEDs (the firmware reports no support).

6. **Audit by the kernel standards** (2026-11-10).
   - bugs:
     - build with `MO=` (kbuild since 6.13 only): DKMS and `make install` failed on 6.12 → build in the source directory (`dkms.conf` without `BUILT_MODULE_LOCATION`, `CLEAN`, `.gitignore`),
     - other mode: attributes with discrete values (steps) accepted any value, the `contains_value` check was missing,
     - MSR: `cache_`, `gpu_`, `uncore_max_overvolt` returned `max_undervolt_uv`,
     - other mode: `args.arg1` of the WMI methods was not set (value from the stack),
     - MSR: Meteor/Arrow Lake by model numbers → `x86_match_cpu` (`INTEL_*`); mailbox from the process context (`*msr*_safe_on_cpu`, `usleep_range`) instead of waiting up to ~3 ms in an IPI with interrupts disabled,
     - after the unbind of the components the pointers to their lists stayed (`cd00/cd01/dd/ft_list`),
     - discrete data: `-E2BIG` also for an exactly filled output,
     - fan curve: write and read by packed little endian structures (`cpu_to_le16`/`le32_to_cpu`, `static_assert` of the size) instead of raw offsets; reading fails when the firmware returns fewer bytes than the table (`legion_wmi_dev_evaluate_buffer` returns the length),
   - standards: MSR attributes by `.dev_groups` of the class (created before the uevent), `sysfs_emit`, no custom uevent variable `DRIVER` (the daemon filters by the subsystem), write-only attributes `0200`, `snprintf`, types of `kstrtouint`/formats, kernel-doc names, `-y` in the Makefile, unused code removed,
   - verified: built as DKMS builds it (in the source directory) against 6.12.116 and 6.18.62 without warnings, all symbols exported; checkpatch 0 errors.

   **State (2026-11-10): done, waits for the test on the hardware.**

**Change (decision of the author 2026-11-13): the ACPI device VPC2004 is bound by the module** (`legion-acpi-vpc.c`, platform driver `legion_acpi_vpc`): the features of `ideapad_laptop` the user loses because of the blacklist – conservation mode (charge limit), USB charging, Fn lock, camera power – are in `/sys/class/legion-firmware-attributes/legion-vpc-0/attributes/` (`current_value`, `supported`, `type`, `display_name`, uevent `EVENT_TYPE` 3000–3003 after a write; a write is verified by reading back, a feature whose write the firmware ignores is marked as not supported – camera power of Legion Pro 7 16IAX10H: `_CFG` reports it, reads 0 while the USB camera works, writes are ignored). The same ACPI methods as `ideapad_laptop` (GBMD/SBMC, HALS/SALS, VPCR/VPCW); only the ACPI device is bound, the WMI GUID `8FC0DE0C` stays with the events driver and `ideapad_laptop` stays blacklisted. Not done: rapid charge (only switched off when the conservation mode is switched on), `charge_types` of BAT0 (power_supply extension).
