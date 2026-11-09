# Konsolidace kernel modulu lenovo_legion (plán z 2026-11-09)

## Výchozí rozhodnutí

- **Vlastní uzavřený stack, žádné stavění nad upstream ovladači** `lenovo-wmi-*` (`drivers/platform/x86/lenovo/`):
  - upstream existuje až od jádra 6.16 a mezi verzemi se mění (přejmenování `capdata01` → `wmi-capdata`, nové atributy, hwmon, baterie); starší jádra (všechna LTS do 6.12) nemají nic, modul musí fungovat stejně na každém jádře,
  - na jeden WMI GUID se naváže jen jeden ovladač (`no_singleton` znamená víc zařízení jednoho ovladače, ne dva ovladače na jednom zařízení); legacy `wmi_evaluate_method(guid)` je zastaralé, nedoručí události a dva ovladače by si přepisovaly stav (cache režimu); exportované funkce upstreamu potřebují jeho `wmi_device` a mezi verzemi se mění,
  - sdílené GUID (gamezone, other, capdata00/01, thermal-mode event) proto drží náš modul a **blacklist upstream `lenovo_wmi_*` je správné řešení**.
- Jména atributů zůstávají naše (daemon a GUI je mají namapované), sjednocení s upstreamem se nedělá.
- `legion_intel_msr` zůstává beze změny (jádro undervolt nemá, `msr` driver zápis do 0x150 bez `allow_writes` blokuje).

## Inventura (stav před konsolidací)

| Část | Co dělá | Daemon | Duplicita |
|---|---|---|---|
| gamezone | `platform_profile`, sysfs `legion-wmi-gamezone-0` (smart_fan, disable_tp, disable_win_key, gsync, igpu_mode, over_drive, skupiny fans, cpu, gpu, macro_key, other) | jen smart_fan, disable_tp, disable_win_key, gsync, igpu_mode, get_power_charge_mode, get_thermal_mode | tepelný režim třemi cestami (platform_profile, smart_fan, get_thermal_mode); teploty a otáčky i v hwmon |
| other + capdata01 + ddata | výkonové limity CPU/GPU, bool funkce capdata00, skupina others | 11 limitů, fan_full_speed, god_mode_fnq_switchable | – |
| fan curve (ftable, fm) | `legion-wmi-fan-mode-0` | ano | – (upstream nemá) |
| events | uevents EVENT_TYPE/EVENT_VALUE | ano | – |
| hwmon | temp1-3, fan1-3 | ano | – |
| machine-information | `legion-dmi/machine-information-0` | ano | **celé v `/sys/class/dmi/id/`** |
| RAPL (`legion-rapl.c`) | jádro čte a zapisuje sysfs `/sys/class/powercap/intel-rapl` | – | **`intel_rapl_msr` (powercap), user space** |
| RAPL MMIO (`legion-rapl-mmio.c`) | `ioremap` registrů processor thermal zařízení, PL1/PL2/PL4, zámek | – | **`processor_thermal_rapl` → `intel-rapl-mmio:0`**, dva zapisovatelé bez společného zámku |
| synchronizace RAPL (`legion-dkms.c`) | po změně režimu zapíše limity režimu do RAPL a MMIO | – | patří do daemonu |

Chyba nalezená při inventuře: `get_ir_temp` (gamezone other) volá metodu `GetPowerChargeMode`.

## Plán

1. **Odstranit duplicity s jádrem** (obecná rozhraní existují i na starých jádrech: DMI odjakživa, `intel-rapl` od 3.13, `intel-rapl-mmio` od 5.3):
   - machine-information a třída `legion-dmi` → daemon čte `/sys/class/dmi/id/` (`SysFsDriverLegionMachineInformation`),
   - `legion-rapl.c`, `legion-rapl-mmio.c` a synchronizace RAPL v `legion-dkms.c` → daemon na uevent změny tepelného režimu zapíše limity režimu do powercapu `intel-rapl` a `intel-rapl-mmio` (hodnoty režimu z capdata01/ddata, které už čte modul Other); custom režim podle aktuálních hodnot,
   - ověřit na hardwaru, že po změně režimu Fn+Q platí stejné limity jako dřív.
2. **Jedna cesta pro tepelný režim, hwmon, probe:**
   - `platform_profile` (custom a extreme jako upstream) + uevent; `smart_fan/current_value` přechodně ponechat pro daemon, pak odstranit,
   - hwmon přes `devm_hwmon_device_register_with_info` (stejná jména `tempX_*`, `fanX_*`),
   - probe nic nenastavuje (žádný Balanced při načtení), jen čte stav.
3. **Zúžit gamezone sysfs** na používané funkce, teploty a otáčky jen v hwmon; nepoužívané (fans, cpu, gpu, macro_key, other) odstranit nebo přesunout do debugfs podle rozhodnutí autora; opravit / odstranit `get_ir_temp`.
4. **Nejnižší podporované jádro**: stanovit (návrh LTS 6.1 nebo 6.6), sjednotit `#if LINUX_VERSION_CODE`, ověřit překlad a načtení aspoň na tom jádře a na aktuálním.

Volitelné (samostatné rozhodnutí): spouštět modul bez vazby na ACPI zařízení `VPC2004` (WMI ovladače v `module_init`, kontrola modelu přes DMI), aby `ideapad_laptop` nemusel být na blacklistu (Fn-lock, conservation mode, kamera); předem ověřit, že se jeho `platform_profile` (DYTC) nepere s naším.
