# Generické GUI – návrh

Stav: návrh a pilotní stránka „Other settings (QML)“ (2026-11-02). Stávající GUI zůstává beze změny, dokud ho nové stránky nenahradí.

## 1. Cíl

Dnešní GUI má pro každý typ dat ručně psaný widget (`OtherControl`, `PowerControl`, `CPUControl`, …). Každý z nich opakuje stejné vzory:

- skrýt funkci, kterou hardware nepodporuje (`supported`, chybějící pole),
- nastavit rozsah ovládacího prvku (`min/max`, `steps`, deskriptory podle režimu),
- nabídnout jen hodnoty, které lze nastavit (`settable_*`, `supported_profiles`, `available_governors`),
- poslat změnu (hned, nebo přes Apply/Cancel) a znovu načíst skutečný stav.

Daemon je po rozdělení na moduly obecný, GUI zůstalo pevně svázané s 12 typy dat. Cíl:

- **GUI se vykreslí podle dat**: popis zobrazení je u dat (`.proto`), stránky jen skládají sekce,
- **nové pole nebo modul nevyžaduje kód v GUI** (stačí anotace v `.proto` a případně řádek v layoutu),
- **speciální pohledy zůstávají vlastní komponenty** (RGB klávesnice, křivka ventilátoru, mřížka CPU, grafy), jen se stejným zdrojem dat.

Nejde o čistě generické GUI. Kde vzor nestačí, je vlastní komponenta. Generický renderer se nemá ohýbat na každou výjimku.

## 2. Architektura

```
 .proto + (legion.ui.*) options          stránka (QML)
 ┌──────────────────────────────┐        ┌──────────────────────────────┐
 │ CPUPState                    │        │ SettingsPage                 │
 │  status  [label, combo,      │        │  AutoSection CPU_PSTATE      │
 │           choices_from ...]  │        │  Section OTHER               │
 │  max_perf_pct [slider, %]    │        │    Field touch_pad.current   │
 └──────────────┬───────────────┘        │  FanCurveEditor (vlastní)    │
                │ Descriptor / Reflection└──────────────┬───────────────┘
                ▼                                       │ field(path)
 ┌──────────────────────────────────────────────────────▼───────────────┐
 │ DataSource (C++, jeden pro všechny typy)                             │
 │  dataType "CPU_PSTATE" -> zpráva podle (legion.ui.message)           │
 │  reload() GET, apply() SET jen změněných polí, cancel(), autoApply   │
 │  FieldModel: value, label, unit, widget, min, max, choices, visible  │
 └──────────────────────────────┬───────────────────────────────────────┘
                                │ DataProvider::getData/setData (bajty)
                                ▼
                              daemon
```

Vrstvy:

1. **Metadata** (`UiOptions.proto`): vlastní volby polí, hodnot enumu a typů dat.
2. **Generický model** (`Ui/DataSource`, `Ui/FieldModel`): zpráva podle `dataType`, přístup k polím přes reflexi protobufu, změny uživatele v samostatné zprávě.
3. **Komponenty QML** (`Ui/qml`): `SettingsPage`, `Section`, `AutoSection`, `Field`, `ApplyBar`, později vlastní pohledy.
4. **Stránky** (`Ui/qml/*Page.qml`): layout, nic víc.
5. **Hostitel** (`ToolBarQmlPageWidget`): stránka QML v `QQuickWidget` jako jedna položka panelu nástrojů, předává notifikace daemonu.

## 3. Metadata (`LenovoLegion-PrepareBuild/UiOptions.proto`)

### 3.1 Pole (`(legion.ui.field)`)

| Volba | Význam |
|-------|--------|
| `label` | Popisek. Chybí-li u listu, použije se popisek nadřazeného pole (např. `touch_pad` pro `touch_pad.current`). |
| `unit` | Jednotka za popiskem. |
| `widget` | `AUTO` (podle typu), `SWITCH`, `SLIDER`, `SPIN`, `COMBO`, `RADIO` (tlačítko pro každou možnost), `LABEL`, `HIDDEN`. |
| `readonly` | Jen zobrazení. |
| `min`, `max` | Číslo, nebo cesta k poli se mezí (`"max_perf_pct"`, `"min"`). `-` na začátku hodnotu pole neguje (`"-max_undervolt"`). |
| `choices_from` | Cesta k `repeated` poli (řetězce nebo enum) s povolenými hodnotami (`"settable_statuses"`, `"supported_profiles"`). |
| `supported_from` | Cesta k `bool` poli, při `false` se prvek skryje. Dědí se z nadřazeného pole. |
| `visible_when` | Jednoduchá podmínka `cesta == hodnota` nebo `cesta != hodnota`. |
| `hint` | Poznámka pod prvkem (např. nutný restart). Dědí se z nadřazeného pole. |
| `value_from` | Cesta k poli, jehož hodnota se zobrazuje, zapisuje se pole samotné (`PowerProfile`: zobrazuje se `thermal_mode`, zapisuje `current_value`). Takové pole se zapíše vždy, i když má zapisované pole už stejnou hodnotu. |
| `scale` | Zobrazená hodnota = uložená × `scale`, zaokrouhleno od nuly jako na stránkách widgetů (µV → mV: `0.001`, −39062 µV → −40 mV). Platí i pro meze. Návrat na zobrazenou hodnotu z daemonu změnu zruší. |
| `step` | Krok `SLIDER`/`SPIN` v zobrazených jednotkách, výchozí 1. `SLIDER`/`SPIN` s neprázdným `choices_from` se zobrazí jako výběr (`steps` výkonových limitů). |
| `tooltip` | Tooltip popisku. Dědí se z nadřazeného pole. |
| `atomic` | Podzpráva se při změně kteréhokoli jejího pole odešle celá (stav z daemonu + změny). Jádro přijme jen celou křivku ventilátoru (`FanCurve.current_value`). |

Cesty mohou vést do hodnoty mapy podle klíče, jen pro čtení: `mode_descriptor_map[255].min_value` (deskriptor custom režimu výkonového limitu), a do prvku `repeated` zprávy podle indexu: `cpus[3].cpu_online`. Zápis do prvku vytvoří prázdné prvky před ním, daemon je přeskočí, takže i u seznamů se posílají jen změny.

`choices_from` ukazující na řetězec rozdělí hodnotu podle mezer (`available_governors` = `"performance powersave"`).

Cesty ve volbách jsou relativní ke zprávě, která obsahuje pole s volbou. Volby v `CpuIntelMSR.VoltageSettings` (`min: "-max_undervolt"`, `supported_from: "supported"`) tak platí pro každou rovinu (`cpu.offset` → `cpu.max_undervolt`).

`AUTO`: `bool` → přepínač, enum nebo `choices_from` → výběr, číslo → číselné pole, řetězec → popisek jen pro čtení.

### 3.2 Hodnoty enumu (`(legion.ui.value_label)`, `(legion.ui.value_color)`, `(legion.ui.value_tooltip)`)

Popisek hodnoty ve výběru, např. `HYBRID_MODE_ONAUTO` → „Hybrid Mode (Auto)“. Bez popisku se zobrazí jméno hodnoty. `value_color` je barva textu hodnoty u přepínačů (`RADIO`), např. barvy výkonových profilů jako `PowerProfileControl.ui` (QUIET `#55aaff`, BALANCED `#919191`, PERFORMANCE `#ff0000`, EXTREME `#000000`, CUSTOM `#aa55ff`). Bez barvy se použije barva tématu.
`value_tooltip` je tooltip hodnoty u přepínačů a v seznamu comba (režimy GPU jako `OtherControl.ui`).

### 3.3 Typ dat (`(legion.ui.message)` na hodnotách `DataType`)

Plné jméno zprávy typu dat, např. `CPU_PSTATE` → `legion.messages.CPUPState`. Z něj `DataSource` vytvoří zprávu přes `DescriptorPool::generated_pool()` a `MessageFactory`. GUI tak nepotřebuje tabulku typů.

### 3.3.1 Příkaz (`(legion.ui.command)` na hodnotách `DataType`)

Typ dat, jehož SET posílá jinou zprávu než GET vrací: `PROFILES` posílá `ProfileCommand` a odpověď je seznam `Profiles`. `DataSource.sendCommand({cesta: hodnota})` sestaví příkaz (enum jménem, např. `"LOAD"`), odešle ho a odpověď načte jako data zdroje. `reloadAllSources()` pak znovu načte všechny zdroje (načtený profil mění všechno). Panel `SavedProfilesPanel` pod výkonovými profily: uložené profily ze správy profilů, Load bez potvrzení.

### 3.4 Platforma (`(legion.ui.platform)` na hodnotách `DataType`)

`PLATFORM_GENERIC` (výchozí): standardní rozhraní Linuxu (cpufreq, intel_pstate, NVML, `legion-intel-msr`), funguje na jakémkoli notebooku. `PLATFORM_LENOVO`: potřebuje ovladač Lenovo Legion (`legion`): výkonové profily, ventilátory, výkonové limity, touchpad, Win klávesa, přepínač GPU, RGB. `HW_MONITORING` a `BATTERY` jsou smíšené a zůstávají obecné (teploty a ventilátory bez ovladače chybí, jejich skupiny se skryjí).

Taby Preview: obecné (CPU Control, CPU Freq Control, CPU Power, GPU), mezera, Lenovo (Power, Fan Curve, Device). Lenovo taby a panel výkonových profilů mají logo Legion (`ThemedTabButton.iconSource`, `Section.titleIcon`), tooltip tabu říká, co stránku ovládá (`PageSources.description`: ovladač Lenovo Legion, nebo standardní rozhraní). Šířka tabu podle textu, víc tabů než šířka se posouvá. Skupinu a stav tabu určují zdroje dat stránky (`PageSources`): stránka bez dat je zašedlá, ale jde otevřít, nahoře je důvod („Requires the Lenovo Legion kernel driver“, tooltip tabu totéž) a obsah je zakázaný. Panel výkonových profilů se zašedne stejně. Stav sleduje notifikace (`LENOVO_DRIVER_ADDED/REMOVED` → `reloadAll()`).

### 3.5 Pravidla zobrazení

- **Pole, které daemon neposlal, se nezobrazí** (explicitní přítomnost polí, `edition = "2024"`). To pokrývá nepodporované funkce, např. `hwp_dynamic_boost` mimo režim `active`.
- `supported_from` a `visible_when` prvek dále skryjí.
- Rozsahy a výběry se berou z dat daemonu, ne z GUI. Daemon hodnoty stejně kontroluje (`settableData`, ModulesDesign.md 9.8).

## 4. Generický model

### 4.1 `DataSource`

- `dataType` (jméno hodnoty `DataType`, např. `"CPU_PSTATE"`).
- `reload()`: GET, neuložené změny uživatele zůstanou (notifikace nesmí přepsat rozpracovanou úpravu).
- `apply()`: SET jen změněných polí, pak `reload()`. Chyba (ERROR_RESPONSE) se zobrazí v `lastError`.
- `cancel()`: zahodí změny.
- `applyValues({cesta: hodnota})`: předvolba z layoutu, zahodí změny a hodnoty hned odešle.
- `setValues({cesta: hodnota})`: předvolba z layoutu, vyplní hodnoty jako změny uživatele.
- `count(cesta)`: počet prvků `repeated` pole (CPU, rozsahy topologie), `daemonValue(cesta)`: hodnota z daemonu bez změn uživatele.
- `revision`: zvýší se při každé změně zprávy. Vazba, která volá `value()`, na něm musí záviset (`source.revision, source.value(...)`).
- `autoApply`: změna pole se pošle hned (přepínače bez tlačítka Apply).
- Po `apply()` se znovu načtou všechny zdroje stejného typu (panel výkonových profilů a stránka Power).
- `interval` (ms): periodické načítání v pracovním vlákně (monitorování), běží jen dokud zdroj existuje; neodeslané změny se zachovají.
- `dirty`: jsou neuložené změny.
- `beginChanges()`/`endChanges()`: dávka změn mnoha polí (všechna vlákna, body křivky) vyšle jednu notifikaci a jeden `autoApply` na konci; každá notifikace přepočítá všechny vazby zdroje, bez dávky roste cena s druhou mocninou počtu polí. `setValues` dávkuje sám.
- `available`: poslední načtení z daemonu proběhlo (daemon typ dat poskytuje), `platform`: `"generic"` nebo `"lenovo"` z `(legion.ui.platform)`.
- `fieldPaths()`: listy zprávy v pořadí z `.proto` bez `HIDDEN` a `repeated` (pro `AutoSection`).
- `field(path)`: `FieldModel`, vytvoří se jednou pro každou cestu.
- Statický `reloadAll()`: volá hostitel při notifikaci daemonu.

### 4.2 `FieldModel`

Vlastnosti pro QML: `value` (zápis jde do změn), `label`, `unit`, `hint`, `widget`, `min`, `max`, `choices` (`[{text, value}]`), `visible`, `readonly`, `modified` (neuložená změna uživatele). Při každé změně zprávy se přepočítají všechny (meze mohou záviset na jiných polích, např. `min_perf_pct` ≤ `max_perf_pct`).

## 5. Komponenty QML

| Komponenta | Účel |
|------------|------|
| `SettingsPage` | Posuvná stránka s titulkem; `available`, `platform`, `reason` ze zdrojů dat stránky, bez dat zakázaný obsah s důvodem. |
| `PageSources` (C++) | Najde všechny `DataSource` stránky: `available` (některý má data), `platform` (`lenovo`, pokud některý potřebuje ovladač), `reason`. |
| `Section` | Skupina jako `QGroupBox` stylesheetu: rámeček 18 px pod horním okrajem s přechodem pozadí, titulek jako záložka vlevo nahoře široká podle textu (odsazení 6/16 px, horní rohy 10 px, vodorovný přechod `titleBackground` → `titleBackgroundEnd`), chyba `lastError` a volitelný `ApplyBar` (`presetLabel`); `titleColor`/`backgroundColor` pro jednotlivou skupinu (buňky vláken v `CpuGrid` a `FrequencyGrid` jako `ThreadControl`/`ThreadFrequencyControl`); `checkable`/`checked` přidá zaškrtávátko do titulku (zaškrtávací `QGroupBox`). |
| `DetailWindow` | Samostatné okno s tématem aplikace, zobrazené dokud `open`, zavření nastaví `open` na false. |
| `AutoSection` | `Section`, která vykreslí všechny pole `DataSource` (`fieldPaths()`). |
| `Field` | Popisek, jednotka, ovládací prvek podle `widget`, poznámka `hint`; `vertical` (popisek nad prvkem, přepínače pod sebou), `labelVisible` a `labelAfter` (popisek vpravo od prvku, „Custom - Toggle Fn+Q“) pro úzké panely. |
| `ApplyBar` | Apply/Cancel, aktivní při `dirty`. |
| `ThemedButton` | Tlačítko v barvách tématu (`QPushButton` stylesheetu). |
| `ThemedComboBox`, `ThemedSpinBox` | Vstupní pole v barvách tématu: akcentový rámeček, světlejší při najetí a fokusu, zaoblení 10 px, zakázaný stav (`QComboBox`, `QSpinBox` stylesheetu). Seznam comba: pozadí a rámeček tématu, vybraná položka s přechodem a levým proužkem, položka pod myší (`QComboBox QAbstractItemView` stylesheetu). |
| `ThemedSwitch` | Přepínač ve stylu checkboxu tématu: zaoblení 6 px, akcentový rámeček, zapnutý s akcentovou výplní (`QCheckBox::indicator` stylesheetu). |
| `ThemedCheckBox`, `ThemedRadioButton` | Checkbox a přepínací tlačítko tématu (`QCheckBox::indicator`, `QRadioButton::indicator`). |
| `ThemedSlider` | Posuvník tématu, vodorovný i svislý: dráha, vyplněná část s přechodem, kulatý úchyt s akcentovým rámečkem (`QSlider`). |
| `ThemedTextField` | Textové pole tématu, text v akcentové barvě (`QLineEdit`). |
| `ThemedTabBar`, `ThemedTabButton` | Záložky tématu: zaoblené nahoře, vybraná s přechodem, nevybraná nižší (`QTabBar::tab`); `available`/`reason` (zašedlá s tooltipem), `groupStart` (mezera před skupinou). Stránky jsou v panelu (`QTabWidget::pane`: přechod, rámeček `theme.paneBorder`, zaoblení 12 px, odsazení 8 px), `SettingsPage.transparent`. |
| `ThemedScrollBar` | Posuvník oblasti tématu (`QScrollBar`), používá ho `SettingsPage`. |
| `ThemedToolTip` | Tooltip tématu (`QToolTip`). |
| `ThemedListBackground`, `ThemedListDelegate` | Seznam a jeho položky tématu (`QListWidget`, `QListWidget::item`). |

Tabulky (`QTableWidget`) a `QLCDNumber` zatím nemají ekvivalent, přijdou s vlastními pohledy, které je potřebují.

### 5.1 Témata

QML nepoužívá stylesheety Qt Widgets (`ToolBarSettingsWidget::getStylesheetTheme`). Hlavní barvy každého stylesheetu jsou v `Ui/Theme` (pozadí, text, vstupní pole, skupina, pruh titulku, akcent, tlačítko, zakázané prvky, zaoblení, velikost písma). Hostitel stránky ho dává QML jako kontextovou vlastnost `theme` a sleduje `ApplicationSettings::themeChanged`, takže změna tématu v Settings se projeví hned.

- `SettingsPage` nastaví paletu stránky z `theme`, ovládací prvky (styl `Fusion`) ji převezmou. `palette.button` je barva vstupních polí, protože jí Fusion kreslí přepínače, výběry a šipky číselných polí.
- `Section` kreslí skupinu a pruh titulku, `ThemedButton` tlačítka jako stylesheet.
- `NoTheme` bere barvy ze systémové palety, stejně jako widgety bez stylesheetu.
- Nové téma = nový případ v `Theme::setTheme` vedle nového stylesheetu.

`Field.overrideValue` zobrazí místo hodnoty pole jinou hodnotu (pole bývá zakázané).

Vlastní pohledy (jen obal generických komponent se stejným `DataSource`):

- **`LimitField`** (hotovo): výkonový limit (`CPUPower.Limit`, `GPUPower.Limit`) jako `PowerControl`. V custom režimu se upravuje `current_value`, jinak je sekce zakázaná a ukazuje výchozí hodnotu aktuálního profilu (`overrideValue`). Rozsah, kroky a podpora jsou z anotace (`mode_descriptor_map[255]`). Předvolby „Preset“ s výchozími hodnotami podporovaných profilů skládá `PowerPage.profilePresets()` z dat.
- **`FanCurveEditor`** (hotovo): křivka ventilátoru jako `FanControl`. Vlastní tab „Fan Curve“ (`FanCurvePage`, `SettingsPage.fillHeight`), roztažený přes celou stránku: tlačítka Maximum/Custom přes celou šířku nad skupinou „Fan curve“ (mimo custom režim a při Maximum je celá skupina zakázaná) (plná rychlost, jen v custom režimu, když je podporovaná), 10 svislých posuvníků přes celou výšku a šířku, osa 100–0 % na úrovních táhel (úroveň 1–10, bod *n* nejméně *n*, bod 10 nejméně 9 jako `FanControl`, popisky a meze z anotace; táhlo pod mezí se vrátí na mez), monotónní křivka při posunu bodu, tlačítko Default (křivka 1–10, odešle se hned; originál ho nemá), tooltip bodu s teplotami a otáčkami výchozích křivek profilu. Mimo custom režim se ukazuje 1–10, při Maximum všude 10. Apply pošle celou křivku (`atomic`).
- **`CpuGrid`** (hotovo, stránka „CPU“): vlákna jako `CPUControl`. Mřížka po 8 řádcích, P/E podle topologie, D = vypnutá v datech z daemonu, barvy buněk a titulků jako `ThreadControl` (pevné, mimo témata, text v P/E buňkách vždy světlý). „Show threads“ filtruje mřížku a určuje, na co se použije governor a Enable/Disable. CPU0 nejde vypnout. „Disabled SMP“ se zapíše hned (`applyValues`, „on“ zapne sourozenecká vlákna i při `control` „on“). Apply pošle jen změněná vlákna (prvky `cpus` podle indexu).
- **`FrequencyGrid`** (hotovo, stránka „CPU“): frekvence vláken jako `CPUFrequencyControl`. Mřížka aktivních vláken (P z `active_cpus_core`, bez nich všechna aktivní, E z `active_cpus_atom`), u vlákna Min/Max v MHz v jeho rozsahu (anotace `CPUFrequency.proto`, `scale`), Min ≤ Max. Globální Min/Max v rozsahu všech vláken se použije na vlákna podle „Show threads“ (oříznuté na jejich rozsah), tlačítka Min/X, X/X a Default nastaví globální Min/Max. Po načtení dat bez změn se globální hodnoty vezmou z prvního vlákna.
- Další: `RgbKeyboard`, `HwMonitorCharts`.

## 6. Chování, které není v datech

| Chování | Řešení |
|---------|--------|
| Okamžitý zápis vs. Apply/Cancel | `autoApply` na `DataSource`, `applyBar` na `Section` |
| Neuložená změna | popisek pole v barvě `theme.modified` (`#995e5e`, stejně jako `MainWindow::VALUE_CHANGE_COLOR`), návrat na hodnotu z daemonu změnu zruší |
| Nutný restart (režim GPU) | `hint` u pole |
| Předvolby (Default u P-state) | `presets` na `Section` (`[{text, values: {cesta: zobrazená hodnota}}]`), tlačítko v `ApplyBar` před Cancel/Apply, `DataSource.applyValues()` zahodí změny a hodnoty hned odešle (stejně jako Default stávajících stránek) |
| Předvolby, které jen vyplní hodnoty (offsety) | předvolba s `apply: false` je položka výběru „Preset“ v `ApplyBar`, `DataSource.setValues()` vyplní hodnoty jako změny uživatele (jen viditelná pole), odešle je Apply |
| Sekce jen v určitém stavu (ventilátor jen v custom režimu) | `visible` sekce podle pole jiného `DataSource` v layoutu (`power.field("thermal_mode").value === 255`) |
| Funkce jen v CUSTOM profilu | `visible`/`enabled` sekce podle jiného `DataSource` v layoutu (`customMode` na stránce „Power“) |
| Vazba SMT ↔ online CPU | v daemonu (modul CPUSMT), GUI jen zobrazuje |

## 7. Postup

1. **Pilot** (hotovo, čeká na test): `UiOptions.proto`, anotace `CPUPState`, `OtherSettings`, `GpuSwitchValue`, generický model, komponenty, stránka „Other settings (QML)“ vedle stávající.
2. Porovnat pilot se stávající stránkou (chování, vzhled, styl). Vzhled podle témat, barva neuložených změn a předvolba Default jsou hotové.
3. Anotovat ostatní zprávy a typy dat, převést jednoduché stránky. Hotovo: stránka „Power“ (PowerProfile, FanOption, offsety napětí CPU a taktů GPU) v záložce stránky „Preview“ (`PreviewPage.qml`). CPUFrequency je po jednotlivých CPU (`repeated`), patří k vlastnímu pohledu `CpuGrid` (krok 4).
4. Vlastní pohledy. Hotovo: výkonové limity CPU a GPU (`LimitField`) a křivka ventilátoru (`FanCurveEditor`) na stránce „Fan Curve“, vlákna CPU (`CpuGrid`) a jejich frekvence (`FrequencyGrid`) na stránce „CPU“, HW monitor (`HwMonitorPage`, grafy `HistoryChart` s mřížkou a popisky os písmem 9 px: hodnoty řádků mřížky vlevo, čas „-50 s“ až „0 s“ dole (`sampleInterval`), vždy viditelný přes celou šířku nahoře, všechny skupiny v jedné řadě, titulky grafů na dva řádky), pod ním výkonové profily (`PowerProfilesPanel`, vždy viditelné) vlevo vedle záložek bez nadpisů stránek s okny podrobností (`DetailWindow`: `CpuFrequencyWindow` frekvence po vláknech, `GpuDetailsWindow`), která se otevírají zaškrtnutím sekce (`Section.checkable`/`checked`), berou data stránky a grafy plní jen když jsou zobrazená. Zbývá RGB.
5. Nahradit stávající stránky, smazat staré widgety.

## 8. Otevřené otázky

- Témata jsou dvakrát: stylesheet pro widgety a barvy v `Ui/Theme` pro QML. Po převodu všech stránek zůstane jen `Ui/Theme`.
- Rust CLI: může použít stejná metadata (popisky, jednotky, výběry) pro generický výpis a nastavení.
- Notifikace: dnes `reloadAll()` při každé notifikaci, později jen pro typ dat z notifikace (notifikace by nesla `dataType`).
