# LenovoLegion-Daemon – framework a moduly

Stav: návrh, v kódu zatím nic nezměněno.

Cíl: z daemona udělat **obecný framework** (protokol, autorizace, načítání modulů, životní cyklus, udev, nastavení, logování, izolace chyb), který nezná žádný konkrétní hardware. Veškerá znalost hardwaru (dnes `SysFsDrivers/`, `SysFsDataProviders/`, `DataProviders/`) bude v **modulech**, které jsou na frameworku i na sobě navzájem co nejvíc nezávislé. Výsledkem má být robustnější systém: chyba jednoho modulu neovlivní ostatní ani spojení s GUI.

---

## 1. Současný stav

**Drivery (16)** dědí z `SysFsDriver`. Provider s nimi komunikuje dvěma způsoby:

- **Za běhu jen přes název:** `SysFsDriverManager::getDriverDesriptor(DRIVER_NAME)` vrátí mapu `název → cesta v sysfs`.
- **Při překladu přes hlavičku driveru:** pomocná struktura (např. `SysFsDriverLegionIntelMSR::IntelMSR`) postavená z té mapy, výčty událostí a `DRIVER_NAME`.

Driver je v podstatě držák cest v sysfs a reakce na udev události. Samostatně nemá smysl, vždy ho používá nějaký provider.

**Providery (17 sysfs a 3 ostatní)** dědí z `DataProvider`:

- rozhraní tvoří `serializeAndGetData`, `deserializeAndSetData`, `init`, `clean` a `kernelEventHandler`,
- identifikuje je `dataType` z centrálního `MessageRegistry.proto`,
- od driver manageru používají jen `getDriverDesriptor`, `getDriverDescriptorsInVector`, `blockKernelEvent`, `refreshDriver` a `processAllUdevEvents`.

**Závislosti providerů na driverech:**

| Provider | Drivery |
|---|---|
| Battery | LegionGameZone, PowerSuplyBattery0 |
| CPUFrequency | CPUXList |
| CPUInfo | CPUInfo |
| CPUOptions | CPUXList |
| CPUPower | LegionOther |
| CPUPState | CPUIntelPState |
| CPUSMT | CPU, CPUXList |
| CPUTopology | CPU, CPUAtom, CPUCore |
| FanCurve | LegionFanMode |
| FanOption | LegionOther |
| GPUPower | LegionOther |
| HWMon | CPUXList, IntelPowercapRapl, LegionHWMon |
| IntelMSR | LegionIntelMSR |
| MachineInformation | LegionMachineInformation |
| Other | LegionEvents, LegionGameZone |
| OtherGpuSwitch | LegionGameZone |
| PowerProfile | LegionGameZone, LegionOther |
| *RGBController* | *LegionEvents (události)* |
| *NvidiaNvml, DaemonSettings* | *žádný driver* |

`SysFsDriverACPIPlatformProfile` se jen registruje v `Application`, žádný provider ani notifier ho nepoužívá. Kandidát na odstranění.

**Framework už teď skoro nezávisí na obsahu zpráv.** `ProtocolProcessor` posílá payload jako neprůhledné bajty a směruje ho jen podle `dataType`. Jediná místa, která znají konkrétní hardware nebo zprávy:

1. **`Application` (konstruktor):** natvrdo registruje 16 driverů a 20 providerů.
2. **`ProtocolProcessorNotifier`:** převádí události 6 konkrétních driverů a 2 providerů na zprávu `Notification`.
3. **`DaemonSettingsManager` a `Settings.h`/`Settings.cpp`:** 27 tříd pro načtení a uložení nastavení, jedna pro každý provider, a pevné pořadí aplikace (nejdřív power profile, některé jen v režimu custom).
4. **Centrální `.proto` v `LenovoLegion-PrepareBuild`:** všechny zprávy, výčet `DataType` a výčet akcí v `Notification`.

**BJLibs už modulový mechanismus má:**

- `ApplicationModulesLoader` načítá `.so` z adresáře `modules/` přes `QLibrary` a funkce `extern "C"` hledá podle názvu,
- makra `APPLICATION_PLUGIN_DEF`/`DEC` deklarují factory funkce,
- `appInitImpl` dostane `ApplicationModulesHandler`, ale daemon ho zatím ignoruje.

---

## 2. Framework a moduly

### 2.1 Rozdělení odpovědnosti

| Framework (daemon) | Modul |
|---|---|
| socket, protokol, autorizace (skupina `legion`) | znalost hardwaru (sysfs, HID, NVML) |
| načtení a ověření modulů (vlastník, práva, ABI) | vlastní `.proto` zprávy |
| životní cyklus, pořadí, závislosti | čtení a zápis hodnot, kontrola rozsahu |
| udev monitor a rozesílání událostí | reakce na události, vlastní notifikace |
| úložiště nastavení (per modul) | obsah a formát vlastního nastavení |
| logger | |
| systémové služby sdílené více moduly (seznam CPU, Legion WMI atributy) | |
| izolace chyb (výjimka → vypnutí modulu, ne pád daemona) | |

Framework nezná žádnou zprávu modulu. Pracuje jen s názvem modulu a neprůhlednými bajty.

### 2.2 Feature moduly místo samostatných driverů

Modul = **jedna funkce** (dnešní provider) **včetně svých driverů**. Driver, který používá jen jeden provider, se stane vnitřním detailem modulu a ven se nedostane. Mezi moduly pak nejsou závislosti při překladu.

Drivery sdílené více providery se stanou **systémovými službami frameworku**. Moduly je používají přes rozhraní SDK, ne přes hlavičky jiných modulů:

| Sdílený driver | Používají | Služba frameworku |
|---|---|---|
| CPUXList | CPUFrequency, CPUOptions, CPUSMT, HWMon | `ICpuList` – seznam CPU a jejich sysfs cesty |
| CPU | CPUSMT, CPUTopology | součást `ICpuList` |
| LegionOther | CPUPower, FanOption, GPUPower, PowerProfile | `ILegionAttributes` – Legion WMI atributy (other) |
| LegionGameZone | Battery, Other, OtherGpuSwitch, PowerProfile | součást `ILegionAttributes` (gamezone) |
| LegionEvents | Other, RGBController | `ILegionEvents` – WMI události firmwaru |

Systémové služby jsou obecné pro Legion notebooky a Linux, ne pro jednu funkci, proto patří do frameworku.

### 2.3 Vlastní zprávy a dynamický `dataType`

- Každý modul má **vlastní `.proto`**. Zprávu zná jen modul a GUI, framework ne.
- `dataType` už není centrální výčet. Modul se identifikuje **názvem** (`"IntelMSR"`) a framework mu při startu přidělí číslo (`quint8`, formát hlavičky zprávy se nemění).
- GUI se při připojení zeptá novým požadavkem `LIST_MODULES` na seznam `{název, dataType, verze}` a dál pracuje s přidělenými čísly.
- Nový modul tak nevyžaduje změnu frameworku ani protokolu.
- Přechodně může framework přidělovat čísla podle dnešního `MessageRegistry.proto`, aby staré GUI fungovalo dál.

### 2.4 Nastavení

- Framework poskytne modulu **vlastní úložiště**: skupinu `[IntelMSR]` v `QSettings`, nebo samostatný soubor `data/modules/IntelMSR.ini`.
- Formát a obsah určuje modul. Z `Settings.cpp`/`Settings.h` se stane 20 malých kusů v modulech.
- Framework si ponechá jen **pořadí aplikace** a podmínku **„jen v custom profilu“** (`SettingsPolicy`).

### 2.5 Notifikace

- `Notification.proto` dnes obsahuje centrální výčet akcí.
- Ve frameworku bude obecná notifikace `{modul, payload}`. Payload je zpráva definovaná modulem. Framework ji jen přepošle a GUI si ji vyloží podle modulu.
- Převod události na notifikaci (dnes v `ProtocolProcessorNotifier`) se přesune do modulu, který událost logicky vlastní.

### 2.6 Izolace chyb a robustnost

Modul v `.so` běží **ve stejném procesu** jako framework:

| Chyba v modulu | `.so` v procesu daemona | Modul jako samostatný proces |
|---|---|---|
| Výjimka | framework ji zachytí, modul vypne, zbytek běží | stejné |
| Segfault, `abort`, nekonečná smyčka | **spadne nebo zamrzne celý daemon** | spadne jen modul, framework ho restartuje |
| Poškození paměti | ovlivní celý daemon | jen modul |
| Únik paměti nebo fd | celý daemon | jen modul |

Pravidla pro moduly v `.so`:

- Framework obalí **každé volání modulu** (`init`, get, set, událost, nastavení) do `try/catch`.
- Výjimka z get/set: framework odpoví klientovi chybou a spojení **neukončí** (dnes `Application::notify()` odpojí GUI).
- Opakované chyby nebo chyba v `init`: framework modul **vypne** (stav `FAILED`), zaloguje to a ostatní moduly běží dál. GUI se o stavu dozví z `LIST_MODULES`.
- Výjimka nesmí projít kódem Qt (signál/slot), to je nedefinované chování. Framework proto volá moduly přímo, ne přes sloty.

Opravdovou izolaci pádů dají jen moduly jako **samostatné procesy**: framework je spouští a komunikuje s nimi přes socketpair stejným protokolem, jaký už existuje. Je to výrazně složitější, ale dává smysl pro rizikové části s cizím kódem:

- **NVML** (proprietární `libnvidia-ml`),
- **RGB/HID** (`hidapi`, komunikace s klávesnicí).

Doporučený kompromis: sysfs moduly jako `.so` s izolací výjimek, NVML a RGB později jako samostatné procesy. API modulu (kap. 4) je navržené tak, aby šlo za stejné rozhraní schovat i modul v jiném procesu.

---

## 3. Architektura

```
LenovoLegion-Daemon                (framework: protokol, autorizace, moduly, udev, nastavení, izolace chyb)
 ├─ libLenovoLegionDaemonSDK.so    rozhraní modulu, IModuleHost, systémové služby (ICpuList,
 │                                 ILegionAttributes, ILegionEvents), SysFsDriver, logger
 ├─ libBJLibs.so                   sdílená (logger, výjimky – viz kap. 6)
 └─ modules/                       root:root 0755, každý .so root:root 0644
     ├─ libModuleIntelMSR.so       + IntelMSR.proto, interní driver LegionIntelMSR
     ├─ libModuleHWMon.so          + HWMon.proto, interní drivery IntelPowercapRapl, LegionHWMon
     ├─ libModuleNvidiaNvml.so     (volitelný, jediný linkuje libnvidia-ml)
     ├─ libModuleRGBController.so  (volitelný, jediný linkuje hidapi)
     └─ ...
```

Zprávy modulů se překládají do knihovny modulu (`libModuleX.so`) a GUI. Žádná zpráva není ve více `.so` zároveň (viz kap. 6, bod 2).

---

## 4. API modulu

### 4.1 Identifikace modulu

```cpp
// LenovoLegionDaemonSDK/ModuleApi.h
namespace LenovoLegionDaemon::Module {

constexpr uint32_t ABI_VERSION = 1;          // zvýšit při každé nekompatibilní změně SDK

struct Info {
    uint32_t            abiVersion;          // musí odpovídat ABI_VERSION frameworku
    const char*         name;                // "IntelMSR" – unikátní identifikace modulu
    const char*         version;             // verze modulu
    const char* const*  requiredServices;    // pole zakončené nullptr, např. {"CpuList", nullptr}
};

}

/* Exportované symboly, hledá je ApplicationModulesLoader */
extern "C" const LenovoLegionDaemon::Module::Info* legion_module_info();
extern "C" LenovoLegionDaemon::IModule*            legion_module_create(LenovoLegionDaemon::IModuleHost& host);
extern "C" void                                    legion_module_destroy(LenovoLegionDaemon::IModule* module);
```

Framework načte `legion_module_info()` dřív, než cokoli vytvoří:

- při jiném `abiVersion` modul odmítne,
- modul se jménem, které už je načtené, odmítne,
- modul, kterému chybí služba z `requiredServices`, nevytvoří (jen zaloguje).

`legion_module_destroy` uvolní modul ve stejné knihovně, ve které vznikl (alokátor a typy z jednoho `.so`).

### 4.2 Rozhraní modulu

```cpp
// LenovoLegionDaemonSDK/IModule.h
class IModule {
public:
    virtual ~IModule() = default;

    /* Lifecycle – výjimka z init → modul FAILED, ostatní běží */
    virtual void init()  = 0;
    virtual void clean() = 0;

    /* Data – payload je zpráva modulu (protobuf), framework ji nezná */
    virtual QByteArray get(const QByteArray& request) = 0;
    virtual QByteArray set(const QByteArray& request) = 0;

    /* Kernel events – udev filtr si modul registruje přes host, framework mu předá jen jeho události */
    virtual void onKernelEvent(const KernelEvent& event) {}

    /* Settings */
    struct SettingsPolicy {
        int  order      = 100;    // pořadí aplikace při startu (PowerProfile = 0)
        bool customOnly = false;  // aplikovat jen při uloženém CUSTOM profilu (FanCurve, CPUPower, GPUPower)
        bool persistent = false;  // modul ukládá nastavení
    };
    virtual SettingsPolicy settingsPolicy() const          { return {}; }
    virtual void loadSettings(ISettingsStorage& storage)   {}   // načti uložené a aplikuj
    virtual void saveSettings(ISettingsStorage& storage)   {}   // přečti stav a ulož
};
```

### 4.3 Rozhraní frameworku pro modul

```cpp
// LenovoLegionDaemonSDK/IModuleHost.h
class IModuleHost {
public:
    virtual ~IModuleHost() = default;

    /* Logging (sdílený logger frameworku, viz kap. 6 bod 1) */
    virtual ILogger& logger() = 0;

    /* Kernel events */
    virtual void addKernelEventFilter(const KernelEvent::Filter& filter) = 0;
    virtual void blockKernelEvents(bool block) = 0;                     // jen události tohoto modulu
    virtual void processPendingKernelEvents(int timeoutInMiliseconds) = 0;

    /* Notification to clients – payload je zpráva modulu */
    virtual void notify(const QByteArray& payload) = 0;

    /* Shared system services */
    virtual ICpuList&          cpuList() = 0;
    virtual ILegionAttributes& legionAttributes() = 0;
    virtual ILegionEvents&     legionEvents() = 0;
};
```

Modul nedostane `SysFsDriverManager`, `DataProviderManager` ani jiný modul. Vidí jen `IModuleHost`.

### 4.4 Načítání ve frameworku

```
appInitImpl(modules):
  1. ověřit modules/ (root:root, bez zápisu pro skupinu a ostatní), stejně jako prepareSocketDir()
  2. modules->load()                                      (ApplicationModulesLoader)
  3. pro každý .so: ověřit vlastníka a práva, legion_module_info() → ABI, jméno, requiredServices
  4. legion_module_create(host) → přidělit dataType → init() v try/catch (chyba → FAILED)
  5. načíst nastavení podle settingsPolicy (order, customOnly), každý modul v try/catch
appStopImpl:
  uložit nastavení → clean() → legion_module_destroy() všech modulů → až potom modules->unload()
```

---

## 5. Mapování souborů na moduly

**Feature moduly (provider + jeho interní drivery):**

| Modul | Z provideru | Interní drivery | Služby frameworku |
|---|---|---|---|
| `Battery` | SysFsDataProviderBattery | PowerSuplyBattery0 | LegionAttributes |
| `CPUFrequency` | SysFsDataProviderCPUFrequency | – | CpuList |
| `CPUInfo` | SysFsDataProviderCPUInfo | CPUInfo | – |
| `CPUOptions` | SysFsDataProviderCPUOptions | – | CpuList |
| `CPUPower` | SysFsDataProviderCPUPower | – | LegionAttributes |
| `CPUPState` | SysFsDataProviderCPUPState | CPUIntelPState | – |
| `CPUSMT` | SysFsDataProviderCPUSMT | – | CpuList |
| `CPUTopology` | SysFsDataProviderCPUTopology | CPUAtom, CPUCore | CpuList |
| `FanCurve` | SysFsDataProviderFanCurve | LegionFanMode | – |
| `FanOption` | SysFsDataProviderFanOption | – | LegionAttributes |
| `GPUPower` | SysFsDataProviderGPUPower | – | LegionAttributes |
| `HWMon` | SysFsDataProviderHWMon | IntelPowercapRapl, LegionHWMon | CpuList |
| `IntelMSR` | SysFsDataProviderIntelMSR | LegionIntelMSR | – |
| `MachineInformation` | SysFsDataProviderMachineInformation | LegionMachineInformation | – |
| `Other` | SysFsDataProviderOther | – | LegionAttributes, LegionEvents |
| `OtherGpuSwitch` | SysFsDataProviderOtherGpuSwitch | – | LegionAttributes |
| `PowerProfile` | SysFsDataProviderPowerProfile | – | LegionAttributes |
| `NvidiaNvml` | DataProviderNvidiaNvml | – | – (volitelný, kandidát na samostatný proces) |
| `RGBController` | DataProviderRGBController + `RGBControlers/` | – | LegionEvents (volitelný, kandidát na samostatný proces) |

**Framework:**

| Soubory | Kam |
|---|---|
| SysFsDriverCPUXList.*, SysFsDriverCPU.* | služba `ICpuList` |
| SysFsDriverLegionOther.*, SysFsDriverLegionGameZone.* | služba `ILegionAttributes` |
| SysFsDriverLegionEvents.* | služba `ILegionEvents` |
| SysFsDriverLegion.h | SDK (společné definice) |
| DataProviderDaemonSettings.* | framework (nastavení samotného daemona) |
| SysFsDriverACPIPlatformProfile.* | nikdo nepoužívá – odstranit |
| Settings.*, DaemonSettingsManager.* | framework jen úložiště a pořadí, obsah do modulů |
| ProtocolProcessorNotifier.* | framework jen přeposílá `{modul, payload}`, převod do modulů |

Volitelné moduly `NvidiaNvml` a `RGBController` umožní přeložit a nainstalovat daemon bez CUDA (dnes povinná závislost `cuda` v `PKGBUILD`) a bez `hidapi`.

---

## 6. Technické pasti

1. **Logger v každém `.so` zvlášť (ověřeno v kódu).** `LoggerHolder` je šablonový singleton se statickou proměnnou a BJLibs se překládá s `--no-gnu-unique`. Každý modul by tak měl **vlastní, neinicializovaný logger** a první `LOG_*` v modulu by vyhodil výjimku „The log file engine was not initialized“.
   → Řešení: logger předávat přes `IModuleHost::logger()`, nebo přeložit BJLibs jako sdílenou knihovnu.

2. **Protobuf zprávy ve více `.so`.** Pokud by se stejný `*.pb.cc` přilinkoval do více knihoven, protobuf by zprávy registroval v globálním poolu vícekrát a při načtení druhé knihovny by skončil fatální chybou („File already exists in database“).
   → Řešení: zpráva modulu je jen v jeho `.so` (a v GUI, což je jiný proces). Sdílené zprávy frameworku (hlavička, `LIST_MODULES`, obecná notifikace) jsou jen v SDK.

3. **Typy výjimek a RTTI přes hranici `.so`.** `DEFINE_EXCEPTION` vytváří typy, které se chytají přes základní třídu `bj::framework::exception::Exception`. Fungovat to bude jen tehdy, když bude základní třída v jedné sdílené knihovně a moduly se přeloží stejným kompilátorem. Framework navíc chytá i `std::exception` a `...`.

4. **Bezpečnost: root daemon načítá `.so`.** Kdo může zapsat do `modules/`, spustí kód jako root.
   - `modules/` a jeho obsah musí patřit `root:root` s právy `0755`/`0644`, **na rozdíl od** `log/` a `data/`, kam může zapisovat skupina `legion`.
   - Framework před načtením zkontroluje vlastníka a práva adresáře i každého `.so`.
   - Do `tmpfiles.d` a `PKGBUILD` to explicitně uvést.

5. **Uvolnění modulů.** Objekty modulu (včetně `QObject`, timerů a spojení signálů) musí zaniknout dřív než `dlclose`. Moduly načíst jednou při startu a uvolnit až po `legion_module_destroy`. `ApplicationModulesLoader::reload()` za běhu nepoužívat.

6. **Výjimky přes Qt.** Výjimka, která projde kódem Qt (signál/slot, `QTimer`), je nedefinované chování. Framework volá metody modulu přímo a každé volání obalí `try/catch`. Moduly s vlastními timery musí výjimky chytat samy (pomůže helper v SDK).

---

## 7. Doporučený postup

Každý krok jde přeložit a otestovat samostatně. Kroky 1–4 zlepší robustnost i bez modulů.

1. **Izolace chyb ve frameworku:** obalit volání providerů do `try/catch`, chybová odpověď místo odpojení klienta (nahrazuje dnešní chování `Application::notify()`).
2. **Systémové služby:** z `CPUXList`/`CPU`, `LegionOther`/`LegionGameZone` a `LegionEvents` udělat rozhraní `ICpuList`, `ILegionAttributes` a `ILegionEvents` a převést na ně providery.
3. **Nastavení do providerů:** `SettingsPolicy`, `loadSettings`/`saveSettings`, framework jen úložiště a pořadí. Odstraní 27 tříd v `Settings.h`.
4. **Notifikace do providerů:** převod událostí z `ProtocolProcessorNotifier` do providerů.
5. **SDK a logger:** `libLenovoLegionDaemonSDK.so` s `IModule`, `IModuleHost` a službami, logger přes host.
6. **Feature moduly:** převést providery i s interními drivery na `.so`, nahradit registraci v `Application` načítáním z `modules/` včetně kontroly vlastníka a práv. Začít volitelnými `NvidiaNvml` a `RGBController`, které přinesou největší užitek (odstranění povinných závislostí).
7. **Vlastní zprávy a dynamický `dataType`:** `.proto` do modulů, `LIST_MODULES` v protokolu, GUI na přidělená čísla.
8. **Volitelně:** `NvidiaNvml` a `RGBController` jako samostatné procesy.

### 7.1 Navržené pořadí prvních kroků (2026-10-22, zatím neschváleno)

API nenavrhovat celé předem, ale hned ho ověřit na pilotním modulu:

1. **HOTOVO (2026-10-22, necommitnuto):** **Izolace chyb** v `ProtocolProcessor::processRequest` (try/catch, chybová odpověď místo odpojení klienta). Odpovídá kroku 1 výše.
   - Nový typ zprávy `MessageHeader::ERROR_RESPONSE = 5`, payload je UTF-8 popis chyby.
   - Daemon: výjimka z provideru i neznámý `dataType` → `LOG_E` + `ERROR_RESPONSE`, spojení zůstává. Chyby parseru (neplatná délka) dál ukončí spojení.
   - GUI: `ProtocolProcessor::get/setDataRequest` vrací `{false, popis}`, `DataProvider::throwRequestError` vyhodí výjimku s kódem `DAEMON_ERROR` a textem od daemona.
   - Ověřeno testem v `/tmp` (skutečný `ProtocolProcessor`, falešný provider): chyba → typ 5 s textem, neznámý `dataType` → typ 5, další požadavek funguje, klient zůstal připojený.
   - Výjimky z timerů a kernel eventů (ToDo bod 7) zatím beze změny.
2. **Návrh API jen jako hlavičky** v novém adresáři `LenovoLegion-DaemonSDK/`: `ModuleApi.h`, `IModule.h`, `IModuleHost.h`. Bez implementace, k připomínkám.
3. **Pilot `IntelMSR` jako `IModule` uvnitř daemona** (bez `.so`, přes adaptér ve frameworku). Prověří interní driver, udev události a jejich blokování, nastavení i notifikace a nepotřebuje sdílené služby.
4. **Úprava API podle pilotu**, pak zbytek postupu.

---

## 8. Kontext pro navázání konverzace

Tahle kapitola slouží k navázání práce v nové konverzaci (Claude Code). Popisuje stav k **2026-10-22**.

### 8.1 Co už je hotové (commity v `main`)

| Commit | Obsah |
|---|---|
| `a6fcb6c` | daemon: `deleteLater` pro protocol processory, async-signal-safe `bj::framework::Signal` (self-pipe + `QSocketNotifier`), výjimka při selhání `listen()`, `Restart=on-failure` v service |
| `0544c42` | `log/`, `data/` jako `root:legion 2770` přes `tmpfiles.d`, logger s `O_NOFOLLOW`, `umask(0007)` v daemonu i GUI |
| `ca26196` | kernel: chyby zápisu MSR offsetu se vracejí ze `store`; protokol: limit délky zprávy 1 MiB, neblokující `tryParseMessage` v daemonu |
| `e3fdacc` | udev pravidlo RAPL: `root:legion 0440` (dřív `0444`, `energy_uj` byl čitelný pro všechny – CVE-2020-8694) |
| `3c22730` | `processAllUdevEvents` se společným timeoutem, `reconnectUdevMonitor` bez pádu (retry po 5 s) |
| `9aa933d` | RGB: kontrola rozsahu profilu/jasu, timeout čekání v reálném čase; NVML: kontrola rozsahu offsetů, inicializace min/max; GUI: pořadí ukončení; výjimka z `init` driveru při načtení modulu; CPU frequency: polling jen `scaling_min/max`, oprava indexu v `init()` |
| `ddd1667` | GUI: přepínače power profilu po SETu znovu načtou stav; úprava `ToDoList.txt` |

Zbývající známé problémy jsou v `LenovoLegion/ToDoList.txt`.

### 8.2 Rozhodnutí a preference autora

- **Neplatný požadavek od klienta:** daemon ho **jen zaloguje (`LOG_E`) a ignoruje**, nevyhazuje výjimku (výjimka dnes odpojí GUI přes `Application::notify()`). GUI si po SETu hodnotu znovu načte a ukáže skutečný stav.
- **Chyby zápisu do sysfs:** stačí je logovat, bez změny protokolu (bod 3 v `ToDoList.txt`).
- **Blokování vlastních kernel eventů** během SETu (`blockKernelEvent` + `processAllUdevEvents(100)`) zůstává, autor ho nechce měnit.
- **Práva:** `log/`, `data/` zapisovatelné pro skupinu `legion`, ostatní nic. `modules/` bude jen pro roota.
- **`bj::framework::Signal`** zůstává v BJLibs, používá se i v jiných projektech.
- **Commit message anglicky**, styl repozitáře (rozkazovací způsob v předmětu, odrážky v těle). Commity dělá autor sám, Claude jen navrhne zprávu.
- **`ToDoList.txt` anglicky.** Komunikace s autorem česky.
- Před větší změnou nejdřív návrh, implementace až po souhlasu.

### 8.3 Prostředí

- Repozitář: `/home/bojar/Git-projects/lenovo-legion-linux-toolkit`, větev `main`.
- Qt 6.12 (`/home/bojar/Qt/6.12.0/gcc_64`), Qt Creator, build adresář `LenovoLegion/build/Desktop_Qt_6_12_0_Debug`.
- Překlad: `make -j24` v build adresáři (nebo `build` přes Qt Creator MCP). Kernel modul jde přeložit v kopii v `/tmp` (`make -C <kopie> all`), hlavičky běžícího jádra jsou k dispozici.
- Daemon i GUI za běhu Claude netestuje (daemon potřebuje roota, GUI běžící daemon). Ověřuje se překladem a malými testy v `/tmp`.

### 8.4 Otevřené otázky k modulům

- Schválit pořadí v kap. 7.1 (izolace chyb → hlavičky API → pilot `IntelMSR`).
- Logger v modulech: přes `IModuleHost::logger()`, nebo BJLibs jako sdílená knihovna?
- Přechod na dynamický `dataType` (`LIST_MODULES`) hned, nebo až po převodu všech modulů (přechodně čísla podle `MessageRegistry.proto`)?
- Odstranit nepoužívaný `SysFsDriverACPIPlatformProfile`?
