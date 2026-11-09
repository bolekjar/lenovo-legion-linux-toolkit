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

   **Stav (2026-11-09): hotovo, otestováno na hardwaru (Legion Pro 7 16IAX10H).**
   - jádro: odstraněné `legion-rapl.*`, `legion-rapl-mmio.*`, `legion-machine-information-sysfs.*`, `legion-dkms.h` (notifier synchronizace), nepoužívané položky `legion_data`; kontrola modelu podle DMI zůstává,
   - daemon: `SysFsDriverLegionMachineInformation` čte `/sys/class/dmi/id/` (bez vazby na kernel modul, `MACHINE_INFORMATION` už není `PLATFORM_LENOVO`),
   - daemon: `CPUPower/RaplPowerLimits` zapisuje limity režimu (stejná pravidla jako jádro: MMIO výchozí hodnoty režimu, v custom aktuální, zapne MMIO; MSR maxima režimu jen když je RAPL zapnutý a MMIO zápis prošel) při startu modulu, načtení kernel modulu, události změny tepelného režimu (`ProviderModule::onDriverEvent`) a v custom režimu hned po nastavení limitů (požadavek klienta, obnovení nastavení),
   - opraveno: provider CPUPower ořezával aktuální hodnoty limitů na 8 bitů (`static_cast<quint8>`),
   - udev pravidlo `99-rapl-readonly.rules` (`rapl-readonly.sh`) zůstává: soubory limitů jsou `0440 root:legion` a sysfs otevření pro zápis souboru bez bitu `w` odmítne i rootovi (ostatní nástroje, např. thermald, limity nezmění); daemon před zápisem přidá vlastníkovi `w` a hned vrátí původní práva.
2. **Jedna cesta pro tepelný režim, hwmon, probe:**
   - tepelný režim jde nastavit jen funkcí `legion_wmi_gz_thermal_mode_set()` (kontrola režimu vůči notebooku, extreme jen s podporou, WMI, uložení režimu zdroje napájení); používá ji `smart_fan/current_value` i `platform_profile`, `platform_profile` čte režim z firmwaru,
   - `smart_fan/current_value` zůstává rozhraním daemonu (stejné na všech jádrech), `platform_profile` je pro ostatní nástroje (power-profiles-daemon),
   - hwmon přes `hwmon_device_register_with_info` (stejná jména `tempX_*`, `fanX_*`),
   - probe nic nenastavuje, jen čte stav.

   **Stav (2026-11-09): hotovo, otestováno na hardwaru (Legion Pro 7 16IAX10H).**
   - mapování profilů: quiet, balanced, performance stejně; extreme = `max-power` (jádro 6.19+), jinak skrytý `balanced-performance` (6.14+); custom = `custom` (6.14+, nově viditelná volba), na starších jádrech zobrazen jako `balanced-performance`,
   - zrušeno: `platform_profile` dřív na baterii tiše měnil performance na balanced (daemon to nikdy nedělal), nešlo nastavit custom ani extreme; `smart_fan` přijímal i 0 a extreme bez podpory,
   - hwmon: zrušeny globální měnitelné tabulky atributů, registrace bez devm (odstraní se před WMI ovladači, ze kterých čte).

3. **Gamezone sysfs**: rozhodnutí autora (2026-11-09): funkce, které daemon nepoužívá, zůstávají (skupiny fans, cpu, gpu, macro_key, other, funkce capdata00); opraveno `get_ir_temp` (volalo metodu `GetPowerChargeMode` místo `GetIRTemp`).

   **Stav (2026-11-09): hotovo.**

4. **Nejnižší podporované jádro: 6.12 LTS** (rozhodnutí autora 2026-11-09).
   - API: `platform_driver.remove` vrací `void` (6.11), `wmi_driver.no_singleton` (6.10), `guard()`/`scoped_guard()`/`__free` (6.5); novější API přes podmínky verze (`platform_profile` ops a custom 6.14, max-power 6.19, `rdmsrq_safe_on_cpu` 7.2),
   - `legion-kernel-version.h`: `#error` pro jádro < 6.12, `dkms.conf`: `BUILD_EXCLUSIVE_KERNEL` (DKMS modul pro starší jádro nestaví), README: požadavek jádra 6.12+,
   - ověřeno: překlad bez varování proti 6.12.116 a 6.18.62 (zdrojáky z kernel.org, `defconfig` + WMI, hwmon, platform_profile, `modules_prepare`), všechny použité symboly jsou v obou jádrech exportované; načtení otestováno na 7.2.

   **Stav (2026-11-09): hotovo.** Načtení na 6.12 a 6.18 netestováno (vyžaduje nabootované jádro).

Volitelné (samostatné rozhodnutí): spouštět modul bez vazby na ACPI zařízení `VPC2004` (WMI ovladače v `module_init`, kontrola modelu přes DMI), aby `ideapad_laptop` nemusel být na blacklistu (Fn-lock, conservation mode, kamera); předem ověřit, že se jeho `platform_profile` (DYTC) nepere s naším.
