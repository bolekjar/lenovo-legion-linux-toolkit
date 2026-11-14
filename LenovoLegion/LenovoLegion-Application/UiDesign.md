# Generic GUI – design

State: design and the pilot page "Other settings (QML)" (2026-11-02). The existing GUI stays unchanged until the new pages replace it.

## 1. Goal

Today's GUI has a hand-written widget for every data type (`OtherControl`, `PowerControl`, `CPUControl`, …). Each of them repeats the same patterns:

- hide a feature the hardware does not support (`supported`, missing field),
- set the range of the control (`min/max`, `steps`, descriptors by the mode),
- offer only the values that can be set (`settable_*`, `supported_profiles`, `available_governors`),
- send the change (at once, or by Apply/Cancel) and read the real state again.

After the split into modules the daemon is generic, the GUI stayed tied to 12 data types. Goal:

- **the GUI is rendered from the data**: the description of the view is at the data (`.proto`), the pages only compose sections,
- **a new field or module needs no code in the GUI** (an annotation in `.proto` and possibly a line in the layout is enough),
- **special views stay custom components** (RGB keyboard, fan curve, CPU grid, charts), only with the same data source.

It is not a purely generic GUI. Where the pattern is not enough, there is a custom component. The generic renderer should not bend to every exception.

## 2. Architecture

```
 .proto + (legion.ui.*) options          page (QML)
 ┌──────────────────────────────┐        ┌──────────────────────────────┐
 │ CPUPState                    │        │ SettingsPage                 │
 │  status  [label, combo,      │        │  AutoSection CPU_PSTATE      │
 │           choices_from ...]  │        │  Section OTHER               │
 │  max_perf_pct [slider, %]    │        │    Field touch_pad.current   │
 └──────────────┬───────────────┘        │  FanCurveEditor (custom)     │
                │ Descriptor / Reflection└──────────────┬───────────────┘
                ▼                                       │ field(path)
 ┌──────────────────────────────────────────────────────▼───────────────┐
 │ DataSource (C++, one for all types)                                  │
 │  dataType "CPU_PSTATE" -> message by (legion.ui.message)             │
 │  reload() GET, apply() SET of the changed fields only, cancel(),     │
 │  autoApply                                                           │
 │  FieldModel: value, label, unit, widget, min, max, choices, visible  │
 └──────────────────────────────┬───────────────────────────────────────┘
                                │ DataProvider::getData/setData (bytes)
                                ▼
                              daemon
```

Layers:

1. **Metadata** (`UiOptions.proto`): custom options of fields, enum values and data types.
2. **Generic model** (`Ui/DataSource`, `Ui/FieldModel`): message by `dataType`, access to the fields by protobuf reflection, the changes of the user in a separate message.
3. **QML components** (`Ui/qml`): `SettingsPage`, `Section`, `AutoSection`, `Field`, `ApplyBar`, later custom views.
4. **Pages** (`Ui/qml/*Page.qml`): layout, nothing more.
5. **Host** (`ToolBarQmlPageWidget`): QML page in a `QQuickWidget` as one item of the toolbar, forwards the notifications of the daemon.

## 3. Metadata (`LenovoLegion-PrepareBuild/UiOptions.proto`)

### 3.1 Field (`(legion.ui.field)`)

| Option | Meaning |
|-------|--------|
| `label` | Label. When missing at a leaf, the label of the parent field is used (e.g. `touch_pad` for `touch_pad.current`). |
| `unit` | Unit after the label. |
| `widget` | `AUTO` (by the type), `SWITCH`, `SLIDER`, `SPIN`, `COMBO`, `RADIO` (a button for every choice), `LABEL`, `HIDDEN`. |
| `readonly` | Display only. |
| `min`, `max` | A number, or a path to the field with the limit (`"max_perf_pct"`, `"min"`). `-` at the beginning negates the value of the field (`"-max_undervolt"`). |
| `choices_from` | Path to a `repeated` field (strings or enum) with the allowed values (`"settable_statuses"`, `"supported_profiles"`). |
| `supported_from` | Path to a `bool` field, the control is hidden when `false`. Inherited from the parent field. |
| `visible_when` | Simple condition `path == value` or `path != value`. |
| `hint` | Note under the control (e.g. restart needed). Inherited from the parent field. |
| `value_from` | Path to the field whose value is shown, the field itself is written (`PowerProfile`: `thermal_mode` is shown, `current_value` is written). Such a field is always written, even when the written field already has the same value. |
| `scale` | Shown value = stored × `scale`, rounded away from zero as on the widget pages (µV → mV: `0.001`, −39062 µV → −40 mV). Applies to the limits as well. Returning to the shown value of the daemon cancels the change. |
| `step` | Step of `SLIDER`/`SPIN` in the shown units, default 1. `SLIDER`/`SPIN` with a non-empty `choices_from` is shown as a choice (`steps` of the power limits). |
| `tooltip` | Tooltip of the label. Inherited from the parent field. |
| `atomic` | The submessage is sent whole (state of the daemon + changes) when any of its fields changes. The kernel accepts only the whole fan curve (`FanCurve.current_value`). |

Paths may lead into a value of a map by key, read only: `mode_descriptor_map[255].min_value` (descriptor of the custom mode of a power limit), and into an element of a `repeated` message by index: `cpus[3].cpu_online`. Writing an element creates empty elements before it, the daemon skips them, so also for lists only the changes are sent.

`choices_from` pointing to a string splits the value by spaces (`available_governors` = `"performance powersave"`).

The paths in the options are relative to the message containing the field with the option. The options in `CpuIntelMSR.VoltageSettings` (`min: "-max_undervolt"`, `supported_from: "supported"`) therefore apply to every plane (`cpu.offset` → `cpu.max_undervolt`).

`AUTO`: `bool` → switch, enum or `choices_from` → choice, number → number field, string → read-only label.

### 3.2 Enum values (`(legion.ui.value_label)`, `(legion.ui.value_color)`, `(legion.ui.value_tooltip)`)

Label of the value in a choice, e.g. `HYBRID_MODE_ONAUTO` → "Hybrid Mode (Auto)". Without a label the name of the value is shown. `value_color` is the text color of the value of radio buttons (`RADIO`), e.g. the colors of the power profiles as `PowerProfileControl.ui` (QUIET `#55aaff`, BALANCED `#919191`, PERFORMANCE `#ff0000`, EXTREME `#000000`, CUSTOM `#aa55ff`). Without a color the color of the theme is used.
`value_tooltip` is the tooltip of the value of radio buttons and in the list of a combo (GPU modes as `OtherControl.ui`).

### 3.3 Data type (`(legion.ui.message)` on the values of `DataType`)

Full name of the message of the data type, e.g. `CPU_PSTATE` → `legion.messages.CPUPState`. From it `DataSource` creates the message by `DescriptorPool::generated_pool()` and `MessageFactory`. The GUI therefore needs no table of the types.

### 3.3.1 Command (`(legion.ui.command)` on the values of `DataType`)

Data type whose SET sends a different message than GET returns: `PROFILES` sends `ProfileCommand` and the answer is the list `Profiles`. `DataSource.sendCommand({path: value})` builds the command (enum by name, e.g. `"LOAD"`), sends it and reads the answer as the data of the source. `reloadAllSources()` then reads all sources again (a loaded profile changes everything). The panel `SavedProfilesPanel` under the power profiles: saved profiles of the profile management, Load without confirmation.

### 3.3.2 Request (`(legion.ui.request)` on the values of `DataType`)

Data type whose GET needs a request message: `RGB_CONTROLLER` sends `RGBControllerRequest` (`request_flags`). `DataSource.request` (`{"request_flags": 4095}`) is built into this message and sent with every load. Writing RGB is a command (`RGBControllerSetRequest`), the daemon answers empty and `sendCommand` then reads the data again. `sendCommand` also takes whole messages and repeated fields as QML objects and arrays (`{"led_group_effects": [{mode, speed, colors: [...], leds: [{name, value}]}]}`, `Reflection::fromVariant`), `DataSource.data(field, revision)` returns a whole field of the data as QML objects (`Reflection::toVariant`: modes, effects, LEDs).

### 3.3.3 Reload after a write (`(legion.ui.reload_after_set)` on the values of `DataType`)

Data types that change together with a written data type. The daemon blocks the kernel events of its own write (or the event of the firmware may not come), so the GUI does not expect a notification after its own write: after `apply()` and `sendCommand()` it reads the written data type and the data types of the annotation again itself (ToDo 1). The lists are the same as `(legion.ui.reload)` of the notification the write would cause:

| Written | Read again | Why |
|---|---|---|
| `CPU_OPTIONS` | `CPU_FREQUENCY,CPU_TOPOLOGY,CPU_SMT` | enabled and disabled CPUs (`CPU_X_LIST_RELOADED`) |
| `CPU_SMT` | `CPU_OPTIONS,CPU_FREQUENCY,CPU_TOPOLOGY` | SMT switches the sibling threads |
| `CPU_PSTATE` | `CPU_OPTIONS,CPU_FREQUENCY` | the status changes the governors, the performance limits change the frequency limits |
| `POWER_PROFILE` | `CPU_POWER,GPU_POWER,FAN_CURVE,FAN_OPTION` | values of the thermal mode (`THERMAL_MODE_CHANGE`) |
| `CPU_POWER`, `GPU_POWER` | the other one | `POWER_CONTROL_CHANGED` |
| `FAN_CURVE`, `FAN_OPTION` | the other one | `FAN_CURVE_CHANGED` |

`PROFILES` (`LOAD`) reads all data types again (`reloadAllSources`). The other data types change only themselves.

### 3.3.4 Hardware not present (`(legion.ui.available_from)`, `(legion.ui.unavailable_reason)` on the values of `DataType`)

A module that runs but has no hardware answers with data without its main field: `NVIDIA_NWML` without an NVIDIA GPU (or without the NVIDIA driver) has no `name`. Such data are not available: the groups hide their fields and presets and show `unavailable_reason`, the page is grayed out with the same reason (`PageSources.reason`), the Help check reports it. It is a state, not an error of a request: `lastError` is set without `failed` (no notification on every load, the HW monitor polls NVML) and the daemon does not log an error on every poll.

### 3.3.5 Daemon module not running (`(legion.ui.module)` on the values of `DataType`, data type `MODULES`)

On connection `DataSource` reads the list of the data providers of the daemon (`MODULES`, `ModuleList`). A data type without a running provider is not requested; it is not available with `moduleError`, which names the module from `(legion.ui.module)`: "Daemon module NvidiaNvml is not installed (libDaemonModuleNvidiaNvml.so is missing …)", "… is not loaded: <reason of the loader>" or "… failed: <reason of init>". `moduleError` comes before the other reasons (also before "Requires the Lenovo Legion kernel driver") in the groups (`Section.unavailableReason`), on the pages and tabs (`PageSources.reason`), in the power profile panel and in the Help check, which lists all modules after a click on "Daemon modules". A daemon without `MODULES` (older version) is used as before.

### 3.4 Platform (`(legion.ui.platform)` on the values of `DataType`)

`PLATFORM_GENERIC` (default): standard Linux interfaces (cpufreq, intel_pstate, NVML, `legion-intel-msr`), works on any notebook. `PLATFORM_LENOVO`: needs the Lenovo Legion driver (`legion`): power profiles, fans, power limits, touchpad, Win key, GPU switch, RGB. `HW_MONITORING` and `BATTERY` are mixed and stay generic (temperatures and fans are missing without the driver, their groups are hidden).

Tabs of the home page (Home): generic (CPU Control, CPU Freq Control, CPU Power, GPU), a gap, Lenovo (Power, Fan Curve, Device). All tabs have an icon of the feature (custom monochrome SVG in `Ui/icons`, `tabIcon` of the page, `ThemedTabButton.functionIcon`) colored by the text color of the tab (`TintedIcon`, Canvas with the composition "source-in", works with software rendering as well). The Lenovo tabs additionally have the Legion logo as a mark after the text and the power profiles panel in the title (`ThemedTabButton.iconSource`, `Section.titleIcon`), the tooltip of the tab says what controls the page (`PageSources.description`: the Lenovo Legion driver, or the standard interfaces). Width of the tab by the text, more tabs than the width scroll. The group and the state of the tab are given by the data sources of the page (`PageSources`): a page without data is grayed out but can be opened, the reason is on the top ("Requires the Lenovo Legion kernel driver", the tooltip of the tab the same) and the content is disabled. The power profiles panel is grayed out the same way. The state follows the notifications (`LENOVO_DRIVER_ADDED/REMOVED` → `reloadAll()`).

### 3.5 Display rules

- **A field the daemon did not send is not shown** (explicit field presence, `edition = "2024"`). This covers unsupported features, e.g. `hwp_dynamic_boost` outside the mode `active`.
- `supported_from` and `visible_when` hide the control further.
- Ranges and choices come from the data of the daemon, not from the GUI. The daemon checks the values anyway (`settableData`, ModulesDesign.md 9.8).

## 4. Generic model

### 4.1 `DataSource`

- `dataType` (name of the `DataType` value, e.g. `"CPU_PSTATE"`).
- `reload()`: GET, unsaved changes of the user stay (a notification must not overwrite an edit in progress).
- `apply()`: SET of the changed fields only, then `reload()`. An error (ERROR_RESPONSE) is shown in `lastError`.
- `cancel()`: discards the changes.
- `applyValues({path: value})`: preset of the layout, discards the changes and sends the values at once.
- `setValues({path: value})`: preset of the layout, fills the values as changes of the user.
- `count(path)`: number of elements of a `repeated` field (CPUs, ranges of the topology), `daemonValue(path)`: value of the daemon without the changes of the user.
- `revision`: incremented on every change of the message. A binding calling `value()` must depend on it: a call with the revision as an argument (`source.value(path, source.revision)`, also `field`, `daemonValue`, `count`, `repeatedValues`), without comma expressions. Helper JS functions call the source the same way, a binding calling them then depends by itself.
- `autoApply`: a change of a field is sent at once (switches without an Apply button).
- After `apply()` all sources of the same type are read again (power profiles panel and the page Power).
- `interval` (ms): periodic load in a worker thread (monitoring), runs only while the source exists; unsent changes are kept.
- `dirty`: there are unsaved changes.
- `beginChanges()`/`endChanges()`: a batch of changes of many fields (all threads, points of a curve) emits one notification and one `autoApply` at the end; every notification re-evaluates all bindings of the source, without a batch the cost grows with the square of the number of fields. `setValues` batches by itself.
- `failed(message)`: signal on every error (also with the same text), `Section` shows the error for `errorTimeout` ms (default 10 s) and then hides it. On the home page (`isNotificationHost`) the error goes to `Notifications`: a notification in the bottom right corner with an icon, the name of the group and the text, disappears after 10 s or by the cross, the same error only renews the time.
- `available`: the last load from the daemon succeeded (the daemon provides the data type), `platform`: `"generic"` or `"lenovo"` from `(legion.ui.platform)`.
- `fieldPaths()`: leaves of the message in the order of the `.proto` without `HIDDEN` and `repeated` (for `AutoSection`).
- `field(path)`: `FieldModel`, created once for every path.
- Static `reloadAll()`: called by the host on a notification of the daemon.

### 4.2 `FieldModel`

Properties for QML: `value` (writing goes to the changes), `label`, `unit`, `hint`, `widget`, `min`, `max`, `choices` (`[{text, value}]`), `visible`, `readonly`, `modified` (unsaved change of the user). All of them are re-evaluated on every change of the message (limits may depend on other fields, e.g. `min_perf_pct` ≤ `max_perf_pct`).

## 5. QML components

| Component | Purpose |
|------------|------|
| `SettingsPage` | Scrollable page with a title; `available`, `platform`, `reason` from the data sources of the page, without data a disabled content with the reason; a change bar at the bottom (`changeBar`): "N changes not applied · Cancel · Apply" of all sources of the page (`PageSources.changeCount/apply/cancel`, `DataSource.changeCount`), only with changes, stays visible while scrolling; the groups on such a page show only the presets; `columnWidth` (minimal width of a column, 0 = one column): on a wide page the groups in columns of the same width, the groups of a row of the same height, content at the top and presets at the bottom (CPU Power, Power, Device: 540 px). `Section.stretchContent` stretches the content over the whole height (profiles panel, fan curve). |
| `PageSources` (C++) | Finds all `DataSource` of the page: `available` (one of them has data), `platform` (`lenovo` when one of them needs the driver), `reason`. |
| `Section` | Group like the `QGroupBox` of the stylesheet: frame 18 px under the top edge with a background gradient, the title as a tab at the top left as wide as the text (padding 6/16 px, top corners 10 px, horizontal gradient `titleBackground` → `titleBackgroundEnd`), the error `lastError` and an optional `ApplyBar` (`presetLabel`); `titleColor`/`backgroundColor` for a single group (cells of the threads in `CpuGrid` and `FrequencyGrid` as `ThreadControl`/`ThreadFrequencyControl`); the cells of the threads are by columns, the number of columns by the width (up to 4, cell 290 px), every type of threads (P, E, disabled D) in its own columns, never mixed (`CellLayout.js`: the fewest rows, one column per type when the grid is at most 25 % higher), in `CpuGrid` Enabled and Governor in one row, in `FrequencyGrid` Min and Max by one `ThemedRangeSlider` with the text "Min – Max MHz"; `checkable`/`checked` adds a check box to the title (checkable `QGroupBox`). |
| `DetailWindow` | Separate window with the theme of the application, shown while `open`, closing it sets `open` to false. |
| `AutoSection` | `Section` that renders all fields of a `DataSource` (`fieldPaths()`). |
| `Field` | Label, unit, control by `widget`, note `hint`; slider with the value and the unit in a cell of a fixed width (by the longest value); a changed value has the mark "●" besides the color `#995e5e` (also in the custom views); `vertical` (label above the control, radio buttons below each other), `labelVisible` and `labelAfter` (label right of the control, "Custom - Toggle Fn+Q") for narrow panels. |
| `ApplyBar` | Apply/Cancel, enabled when `dirty`. |
| `ThemedButton` | Button in the colors of the theme (`QPushButton` of the stylesheet). |
| `ThemedComboBox`, `ThemedSpinBox` | Input fields in the colors of the theme: accent frame, lighter on hover and focus, rounding 10 px, disabled state (`QComboBox`, `QSpinBox` of the stylesheet). List of a combo: background and frame of the theme, the selected item with a gradient and a left stripe, the item under the mouse (`QComboBox QAbstractItemView` of the stylesheet). |
| `ThemedSwitch` | Switch in the style of the check box of the theme: rounding 6 px, accent frame, checked with an accent fill (`QCheckBox::indicator` of the stylesheet). |
| `ThemedCheckBox`, `ThemedRadioButton` | Check box and radio button of the theme (`QCheckBox::indicator`, `QRadioButton::indicator`). |
| `ThemedSlider` | Slider of the theme, horizontal and vertical: groove, filled part with a gradient, round handle with an accent frame (`QSlider`). |
| `ThemedTextField` | Text field of the theme, text in the accent color (`QLineEdit`). |
| `ThemedTabBar`, `ThemedTabButton` | Tabs of the theme: rounded at the top, the selected one with a gradient, the unselected lower (`QTabBar::tab`); `available`/`reason` (grayed out with a tooltip), `groupStart` (gap before a group). The pages are in a pane (`QTabWidget::pane`: gradient, frame `theme.paneBorder`, rounding 12 px, padding 8 px), `SettingsPage.transparent`. |
| `ThemedScrollBar` | Scroll bar of an area of the theme (`QScrollBar`), used by `SettingsPage`. |
| `ThemedToolTip` | Tooltip of the theme (`QToolTip`). |
| `ThemedListBackground`, `ThemedListDelegate` | List and its items of the theme (`QListWidget`, `QListWidget::item`). |

Tables (`QTableWidget`) and `QLCDNumber` have no equivalent yet, they come with the custom views that need them.

### 5.1 Themes

QML does not use the stylesheets of Qt Widgets (`ToolBarSettingsWidget::getStylesheetTheme`). The main colors of every stylesheet are in `Ui/Theme` (background, text, input fields, group, title bar, accent, button, disabled controls, rounding, font size). The host of the page gives it to QML as the context property `theme` and follows `ApplicationSettings::themeChanged`, so a change of the theme in Settings applies at once.

- `SettingsPage` sets the palette of the page from `theme`, the controls (style `Fusion`) take it over. `palette.button` is the color of the input fields, because Fusion draws switches, choices and the arrows of number fields with it.
- `Section` draws the group and the title bar, `ThemedButton` the buttons like the stylesheet.
- `NoTheme` takes the colors from the system palette, the same as widgets without a stylesheet.
- New theme = a new case in `Theme::setTheme` next to a new stylesheet.

`Field.overrideValue` shows a different value instead of the value of the field (the field is usually disabled).

Custom views (only a wrapper of the generic components with the same `DataSource`):

- **`LimitField`** (done): power limit (`CPUPower.Limit`, `GPUPower.Limit`) as `PowerControl`. In the custom mode `current_value` is edited, otherwise the section is disabled and shows the default value of the current profile (`overrideValue`). Range, steps and support come from the annotation (`mode_descriptor_map[255]`). The presets "Preset" with the default values of the supported profiles are built by `PowerPage.profilePresets()` from the data.
- **`FanCurveEditor`** (done): fan curve as `FanControl`. Own tab "Fan Curve" (`FanCurvePage`, `SettingsPage.fillHeight`), stretched over the whole page: buttons Maximum/Custom over the whole width above the group "Fan curve" (outside the custom mode and with Maximum the whole group is disabled) (full speed, only in the custom mode when supported), 10 vertical sliders over the whole height and width, axis 100–0 % at the levels of the handles (level 1–10, point *n* at least *n*, point 10 at least 9 as `FanControl`, labels and limits from the annotation; a handle under the limit returns to the limit), monotonic curve when a point moves, button Default (curve 1–10, sent at once; the original does not have it), tooltip of a point with the temperatures and speeds of the default curves of the profile. Outside the custom mode 1–10 is shown, with Maximum 10 everywhere. Apply sends the whole curve (`atomic`).
- **`CpuGrid`** (done, page "CPU"): threads as `CPUControl`. Grid of 8 rows, P/E by the topology, D = disabled in the data of the daemon, colors of the cells and titles as `ThreadControl` (fixed, outside the themes, the text in the P/E cells always light). "Show threads" filters the grid and determines what the governor and Enable/Disable apply to. CPU0 cannot be disabled. "Disabled SMP" is written at once (`applyValues`, "on" enables the sibling threads also with `control` "on"). Apply sends only the changed threads (elements of `cpus` by index).
- **`FrequencyGrid`** (done, page "CPU"): frequencies of the threads as `CPUFrequencyControl`. Grid of the active threads (P from `active_cpus_core`, without them all active, E from `active_cpus_atom`), per thread Min/Max in MHz in its range (annotations of `CPUFrequency.proto`, `scale`), Min ≤ Max. The global Min/Max in the range of all threads is applied to the threads by "Show threads" (clamped to their range), the buttons Min/X, X/X and Default set the global Min/Max. After loading data without changes the global values are taken from the first thread.
- Next: `RgbKeyboard`, `HwMonitorCharts`.

## 6. Behavior that is not in the data

| Behavior | Solution |
|---------|--------|
| Immediate write vs. Apply/Cancel | `autoApply` on `DataSource`, `applyBar` on `Section` |
| Unsaved change | label of the field in the color `theme.modified` (`#995e5e`, the same as `MainWindow::VALUE_CHANGE_COLOR`), returning to the value of the daemon cancels the change |
| Restart needed (GPU mode) | `hint` at the field |
| Presets (Default of P-state) | `presets` on `Section` (`[{text, values: {path: shown value}}]`), a button in `ApplyBar` before Cancel/Apply, `DataSource.applyValues()` discards the changes and sends the values at once (the same as Default of the existing pages) |
| Presets that only fill the values (offsets) | a preset with `apply: false` is an item of the choice "Preset" in `ApplyBar`, `DataSource.setValues()` fills the values as changes of the user (visible fields only), Apply sends them |
| Section only in a certain state (fan only in the custom mode) | `visible` of the section by a field of another `DataSource` in the layout (`power.field("thermal_mode").value === 255`) |
| Feature only in the CUSTOM profile | `visible`/`enabled` of the section by another `DataSource` in the layout (`customMode` on the page "Power") |
| Coupling SMT ↔ online CPUs | in the daemon (module CPUSMT), the GUI only shows it |

## 7. Steps

1. **Pilot** (done, waits for a test): `UiOptions.proto`, annotations of `CPUPState`, `OtherSettings`, `GpuSwitchValue`, generic model, components, page "Other settings (QML)" next to the existing one.
2. Compare the pilot with the existing page (behavior, look, style). The look by the themes, the color of unsaved changes and the preset Default are done.
3. Annotate the other messages and data types, convert the simple pages. Done: page "Power" (PowerProfile, FanOption, offsets of the CPU voltage and GPU clocks) in a tab of the home page (`HomePage.qml`, formerly "Preview"). CPUFrequency is per CPU (`repeated`), it belongs to the custom view `CpuGrid` (step 4).
4. Custom views. Done: CPU and GPU power limits (`LimitField`) and the fan curve (`FanCurveEditor`) on the page "Fan Curve", CPU threads (`CpuGrid`) and their frequencies (`FrequencyGrid`) on the page "CPU", HW monitor (`HwMonitorPage`, charts `HistoryChart` with a grid and axis labels in a 9 px font: values of the grid rows on the left, time "-50 s" to "0 s" at the bottom (`sampleInterval`), thresholds `warning`/`critical` (bars and the value orange/red: temperatures 75/90 °C, GPU slowdown − 10 / slowdown, fans 85 % of the maximum orange only), in narrow charts the name and below it the current value in a larger bold font, always visible over the whole width at the top, all groups in one row, chart titles on two lines), below it the power profiles (`PowerProfilesPanel`, always visible: the active profile in a large font, a card for every profile with a stripe in the color of the profile, the active card in the accent of the theme) on the left next to the tabs without page titles with detail windows (`DetailWindow`: `CpuFrequencyWindow` – "CPU Details": live tiles `StatTile` at the top (average of all / P / E threads, the fastest and the slowest thread, CPU power and temperature), below them the thread charts in the groups "Performance threads" / "Efficient threads" with columns by the width of the window (chart 200 px) and next to them in the right column (380 px) the processor and the cache as rows of `InfoSection` (model, generation, threads P + E, frequency ranges with base, set limits, cache in KB/MB); `GpuDetailsWindow` – name with the state Active/Suspended, tiles of the clocks, load, VRAM, temperature (against the slowdown), power (against the enforced limit) and PCIe, below them the charts in the groups Clocks / Load and memory / Thermal, power and PCIe and next to them in the right column (380 px) the specifications (clock ranges, offsets, power limit and default, temperature thresholds, current and maximal PCIe link); without the hybrid topology all active threads are P-threads, an overflow of the RAPL energy counter is skipped), opened by checking the section (`Section.checkable`/`checked`), they take the data of the page and fill the charts only while shown. RGB keyboard (`KeyboardPage.qml`, toolbar page "Keyboard"): step 1 done (group Control: brightness, profile, logo; device information), step 2 done (`KeyboardView`: layout from `KeyboardLayout` in C++. Lenovo Legion keyboard (at least 60 key codes of the template found): three views of the notebook switched by a switch in the style of the notebook (dark chassis, pictogram of the view, the selected view with a light line in the accent, a badge with the number of selected; neither the page nor the group has the title "Keyboard") with the same scale and the same height (no jumping), the drawing centered on the background of the theme (gradient like the tab pane of the home page, no frame, the notebook itself dark), at most 1100 px wide and at most 55 % of the window height high (not huge and without a scroll bar in a large window): "Keyboard" from the top (open lid with the display, keys), "Front" from the front (display, front edge with the front bar), "Back" from the back, lid and body as large as in "Front" (lid with the logo, rear edge with the grilles, rear bar as an outline around the ribs by the photo: the LEDs are in the top row of the matrix in two interleaved rows, the upper row 1001–1010 goes up along the outer left edge, over the top edge of the left grilles, by a slope into the vertical middle of the ribs (no grille in the middle), over the top edge of the right grilles and down along the outer right edge, the lower row 1011–1018 under the left (first half by the columns) and under the right grilles (second half) and up along the inner edges to the middle; segments of the same length in the order of the columns); the selection the same in all themes: outline 3 px in the accent of the theme lightened to a lightness of at least 0.6 (the dark accent of the gray / light theme is not visible on the dark notebook) with a white inside, the color of the other LEDs dimmed while something is selected or highlighted (keys to 45 %, bars and logo only to 65 %, a more dimmed thin light looks off; a selected bar has an outline and a thin white rim, the color of the LED stays visible); every LED has a light housing (edge of the keys, rim of the segments and lines of the rear bar, outline of the logo) which is not dimmed, so even a dark / off LED is visible on the black notebook; a click selects the LED of a key / segment (again: nothing), Ctrl+click adds / removes, the active area of the mouse is the whole group (also around the drawing and the padding of the group, the switch and the buttons above it), a click outside the LEDs clears the selection, dragging with the left button selects everything in the rectangle (continuously, with Ctrl it adds), the segments of the rear bar are hit by the distance from their line, the selection is common for all views, the tabs show the number of selected, "Select all" selects all LEDs of all views, "Clear" nothing; keys at their physical positions by the LED codes (template measured from a photo of the keyboard of the Legion Pro 7i Gen 10, the matrix of the driver only approximates the positions and the names are not unique, twice "\\ (ISO)": 0xA8 is \\ of the ANSI keyboard, 0x4E belongs to the left Shift), LEDs without a key as light bars by the place in the matrix (top row = rear bar around the grilles, codes 1001–1018; perimeter = front bar at the bottom, 501–510, one bar along the front edge in the order around the matrix, width of a segment by the number of its cells), logo 1501 as the text LEGION on the lid, other unknown LEDs in a row under the notebook. Other devices: layout by the former widget `DeviceView` (matrix, large keys over empty cells, touching LEDs with the same name are one key, holes of one cell are filled by the neighboring large key). Everything drawn (the photos did not match the keys), gap between the keys at least 3 px, keys with a plastic bottom edge, long labels shrink, colors of the LEDs from a second source with `REQUEST_STATE_FOR_ALL_LEDS` every 100 ms in a worker thread); steps 3 and 4 done: group "Color" (`ColorPicker`: hue/saturation wheel drawn by gradients, brightness, preview, hex, presets; grayed out for effects without colors), "Effect" (mode, LEDs by the flags of the mode: selected / all / all keys (`KeyboardLayout.keyLeds`) / whole device, speed, direction by the flags, colors per LED / given by the mode (count +/−, a click on a color edits it) / random, "Add Effect", a message at the maximum of effects; memory of the keyboard: the effects go in one report (header 7 + effect 16 + 3 per color + 2 per LED code, every code once), the keyboard keeps only a part and silently drops the rest (whole effects or LEDs at the end; c197 about 481 bytes by the tests), the length byte is 0xC0 as in the factory profiles; the limit is given by the daemon per model in `max_effects_bytes` (c197 481, others 0 = unknown); number of effects `max_effects` 20 (c197 stores more by the test, but only the first 20 light; the daemon refuses more effects by an error), "Effects" shows the number of effects and with a known limit the memory usage, "Add Effect" is blocked with an explanation, the daemon reads the effects back after a write and reports a truncated list by an error), "Effects" (list with the colors, mode and the number of LEDs; hovering highlights the LEDs of the effect in white in the drawing (`KeyboardView.marked`), a click selects them, the cross removes the effect, "Clear All", "Reset To Default"); the daemon always gets the whole list of effects, a new effect with LEDs takes them from the older effects (empty ones disappear), a new effect without LEDs replaces all of them (the same as `RGBController::AddEffect`); step 5 done: the old widgets (`RGBKeyboardDevice`, `DeviceView`, `ColorWheel`, `Swatches`, `RGBController` GUI, `QTooltipedSlider`, `ToolBarKeyboardWidget`) and the photos of the keyboard removed.
5. Replace the existing pages, delete the old widgets. Done: the toolbar page "Home" shows `HomePage.qml`, the old widget pages of the home page and Qt Charts are removed. Done for the RGB keyboard as well (the toolbar page "Keyboard" shows `KeyboardPage.qml`, the old keyboard widgets removed). Done for "Information" as well (`InformationPage.qml`: header with the model, manufacturer, machine type and chips of the main parts, groups `InfoSection` Processor (with cache chips), Graphics, System, BIOS, Board & Chassis; the values can be selected by the mouse and copied by a button in the row, "Copy All" copies a text overview, a confirmation in `Notifications` of the kind "info"; `MACHINE_INFORMATION` has a message annotation). Done for "Help" as well (`HelpPage.qml`: live system check – daemon, drivers lenovo_legion and legion_intel_msr, NVIDIA NVML, RGB keyboard (the same test as `KeyboardPage`: brightness > 0) with instructions on an error, "Check Again"; topics on the left, text on the right, Markdown of the topics converted to HTML with spacing and the colors of the theme, commands in a monospace font, links to the browser; text written again by the current GUI). Done for "Profiles" as well (`ProfilesPage.qml`: on the left the form "Save Current Configuration" (name, description; an existing name fills its description and the button is "Overwrite Profile" with a question), on the right the saved profiles as cards with an initial, description, Load (with a question, then all sources are read again), Edit (to the form) and delete (with a question); questions in `ConfirmDialog` (modal, destructive actions in red), results and errors in `Notifications`). Done for "Settings" as well (`PreferencesPage.qml`, opens also without the daemon: look – theme cards with a preview in the colors of the theme (No Theme from `SystemPalette`); application – switches by `appSettings` (`Ui/AppSettings`, bridge to `ApplicationSettings`, saves at once); daemon – `DAEMON_SETTINGS` has a message and a command annotation, the daemon replaces the settings by the whole message, so all four values are always sent (`sendCommand`), Save Current Configuration sends `save_now` after a question, Reconnect by `AppSettings.reconnectDaemon` (set by `MainWindow`); Reset to Defaults after a question). The stylesheets of the main window are in `Stylesheets` (`Application::applyTheme`). All widget pages are replaced. The main window is done as well: `MainWindow` is only a `QMainWindow` (close to the tray) with one `QQuickWidget` and `Shell.qml` – header with the logo (About) and the navigation of the pages (SVG icons, the current page with a light line of the accent), the page in a `Loader` (pages with the daemon only while connected, created again after a new connection, without a connection a card "Not connected" with Reconnect and Help; Settings and Help also without the daemon), status bar with the state of the daemon (Reconnect), the active power profile, the battery (`BatteryIndicator`: icon with the level, charging / adapter, %, remaining time, on hover a card with the energy, health, power, voltage, cycles, model; the daemon sends from power_supply BAT0 the capacity, energy, power, voltage, cycles, model and the adapter state, the battery also without the Lenovo driver) and the version; `AboutDialog` (logo, version, license, GitHub, also from the tray menu). State for QML in `Ui/AppState` (connection, version, Reconnect, Quit, About). The widgets `ToolBar*Widget`, `BateryStatus`, `AboutWindow`, `TaskList`, `Utils`, the `.ui` files and the stylesheets are removed, only the tray stays in the widgets (`QSystemTrayIcon`).

## 8. Open questions

- Themes: done, they are only in `Ui/Theme` (the stylesheets of the widgets are removed together with the widgets).
- Rust CLI: may use the same metadata (labels, units, choices) for a generic listing and setting.
- Notifications: done. The actions in `Notification.proto` have the annotation `(legion.ui.reload)` with the list of data types the GUI reads again after the notification (`DataSource::reloadForNotification`); actions without an annotation (driver added / removed) and a wrong annotation read everything. Sources with an interval (HW monitor, NVML, battery, keyboard colors) are read in a worker thread on notifications, `reloadAll` (connection) and `reloadAllSources`; the first load stays in the GUI thread (the bindings of the page get the data at once, e.g. the value of a slider is not clamped by an empty range).
