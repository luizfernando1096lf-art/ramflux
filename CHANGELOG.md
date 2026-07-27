# Changelog

All notable changes to RAMFlux are documented here.

The format is based on [Keep a Changelog](https://keepachangelog.com/),
and this project adheres to [Semantic Versioning](https://semver.org/).

## [2.44.0] — 2026-07-27

### Fixed
- **FluxTelemetry UAF (CRITICAL)** — `m_collectionRequest` was a raw `HANDLE` with no synchronization; replaced with `std::atomic<HANDLE>` and atomic loads before every `SetEvent`/`ResetEvent`
- **Named pipe full-path auth bypass (CRITICAL)** — `RAMFluxHelper` compared client pipe prefix+suffix instead of full path, allowing impersonation via `\\.\pipe\any_prefixRAMFluxPipe`; now requires exact path match
- **Named pipe instance hoarding (HIGH)** — `PIPE_UNLIMITED_INSTANCES` allowed unlimited pipe clones; capped to 2
- **PluginManager cancel-thread execution (CRITICAL)** — `executeAllWithTimeout()` used `wait_for` + flag but thread had no cancellation point; removed, only `executeAll()` remains
- **PluginManager sandbox bypass (HIGH)** — `setProcessPriority` and `setProcessIoPriority` accepted self-PID and critical system PIDs; added guards for PID ≤ 4 and self-PID
- **PluginManager DLL hijacking (CRITICAL)** — `LoadLibraryW` with `LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR` vulnerable to DLL side-loading in plugin directory; removed flag, added try/catch
- **getCacheTopology double-checked locking (HIGH)** — broken DCLP pattern without `std::call_once`; replaced with `std::call_once`
- **Disk queue length rate calculation (HIGH)** — pair of `std::atomic` values updated non-atomically; replaced with `std::mutex`
- **Hard fault prediction rate calculation (HIGH)** — same atomic-pair race; replaced with `std::mutex`
- **Predictive hard fault info rate calculation (HIGH)** — same atomic-pair race; replaced with `std::mutex`
- **FluxNTAPI bool races (HIGH)** — `s_ioCached`, `s_powerCached`, `s_powerWriteCached`, `s_ntdllCached` were plain `bool` accessed from multiple threads; changed to `std::atomic<bool>` with `memory_order_acquire/release`
- **MainWindow ProfileManager callback (HIGH)** — lambda captured `this` without unsubscribe; stored subscription token, `unsubscribeProfileChanged()` in destructor
- **Smart Optimize deepClean duplication (HIGH)** — Smart Optimize called `deepClean()` which was identical to Deep Clean; replaced with `quickClean()` for distinct behavior
- **HeuristicEngine m_effectiveness race (HIGH)** — `recentAccuracy` read without lock in `calculateEffectiveness()`; now read under `m_mutex`
- **FluxClassifier config() race (HIGH)** — `config()` read `m_config` without lock; added `lock_guard`
- **PressurePredictor R² calculation (MEDIUM)** — intercept hardcoded to first sample instead of regression intercept; corrected to `yMean - slope * xMean`
- **FluxClassifier m_history unbounded growth (MEDIUM)** — dead processes never evicted; stale entries (>10min) now cleaned on each `recordSample()`
- **ConsoleWidget m_allLines unbounded growth (MEDIUM)** — `QStringList` accumulated every log line forever; capped at 10,000 entries
- **ProcessRulesEngine missing critical processes (MEDIUM)** — `svchost.exe` and `dwm.exe` missing from critical process list; added
- **ProcessRulesEngine multi-wildcard pattern broken (MEDIUM)** — `find('*')` only matched first wildcard; `*foo*` never matched; replaced with proper backtracking glob matcher supporting `*` and `?`
- **getCacheTopology infinite loop (MEDIUM)** — `ptr += info->Size` with `Size==0` caused infinite loop; added `if (Size == 0) break` guard in all three loops
- **trimAllProcesses PID 0 (MEDIUM)** — `trimAllProcesses()` did not skip PID ≤ 4 (System Idle, System); added guard matching `trimIdleProcesses()`
- **FluxTelemetry first collection silent catch (MEDIUM)** — initial `catch (...) {}` swallowed all exceptions without logging; added exception logging matching main loop pattern
- **PressurePredictor O(n) front erase (MEDIUM)** — `vector::erase(begin())` was O(n); changed container to `std::deque` for O(1) `pop_front()`
- **FluxScheduler executeAllWithTimeout (HIGH)** — removed alongside PluginManager cancel-thread fix; only `executeAll()` remains

### Changed
- **Constants.h** — version bumped from 2.43.0 to 2.44.0

## [2.43.0] — 2026-07-19

### Fixed
- **DiagnosticsEngine deadlock (CRITICAL)** — `checkLoop()` invoked callbacks while holding `m_mutex`; if a callback called `unsubscribe()`, it would deadlock on the same non-recursive mutex. Callbacks are now copied under lock and invoked outside.
- **HeuristicEngine m_lastCleanTime data race (HIGH)** — `m_lastCleanTime` written from EventBus dispatch thread without lock while read under `m_mutex`; replaced with `std::atomic<int64_t>` epoch.
- **HeuristicEngine EventBus dangling this (HIGH)** — `CleaningStarted`/`CleaningFinished` subscriptions captured `this` but were never unsubscribed; added subscription ID storage and `unsubscribe()` in `shutdown()`.
- **FluxClassifier m_config data race (HIGH)** — `setConfig()` wrote `m_config` without mutex while `classifyImpl()` read it under `m_mutex`; added `lock_guard` in `setConfig()`.
- **FluxScheduler m_originalIntervalMs data race (MEDIUM)** — plain `int` read/written without synchronization from multiple threads; changed to `std::atomic<int>`.
- **Logger m_maxBackupFiles/m_compressBackups data race (MEDIUM)** — written without lock, read under `m_mutex`; changed to `std::atomic<int>` and `std::atomic<bool>`.
- **FluxNTAPI HardFaultHistory sample count race (HIGH)** — `hist.samples.size()` accessed without locking `hist.mtx`; added public `size()` method with internal lock.
- **PluginManager async plugin reference (MEDIUM)** — lambda captured reference to vector element that could dangle during concurrent modification; captures raw `IPlugin*` by value instead.

### Changed
- **Constants.h** — version bumped from 2.42.0 to 2.43.0

## [2.42.0] — 2026-06-29

### Fixed
- **Thread safety audit — 24 bugs fixed** across thread safety, memory safety, TOCTOU, and API design
- **PowerManager UAF (CRITICAL)** — `monitorLoop()` iterated `m_subs` without mutex while `unsubscribe()` could erase elements; now copies callback list under lock
- **NetworkQoS data race (HIGH)** — `m_lastUpdate` (non-atomic) read/written without mutex; `lock_guard` moved before access
- **cacheIpHlp double-checked locking (HIGH)** — `s_iphlpCached` checked without mutex; added `static std::mutex s_iphlpMutex`
- **Named pipe DACL LPE (HIGH)** — `RAMFluxHelper` pipe granted `Generic All` to `Interactive Users`; replaced with current user SID via `GetTokenInformation`
- **Pipe client verification TOCTOU (HIGH)** — path check occurred after `RevertToSelf()`; moved inside impersonation window with process handle held open
- **Handle leak in FluxGameMode (HIGH)** — `m_gameHandle` overwritten without `CloseHandle` on previous value
- **Unchecked malloc in RAMFluxHelper (HIGH)** — `tokenUser` dereferenced without null check after `malloc`
- **activeRuleCount data race (CRITICAL)** — read `m_appliedRules.size()` without lock; moved to `.cpp` with `lock_guard`
- **MemoryDedup data race (CRITICAL)** — `m_lastScan`/`m_lastReport` accessed without mutex; early-return check now under lock
- **ProfileManager callback UAF (CRITICAL)** — `onProfileChanged()` had no unsubscribe; added token-based subscribe/unsubscribe API
- **ConfigIO enum UB (CRITICAL)** — `ProfileType`/`RuleType`/`RuleAction` cast from JSON without range validation; added bounds checks
- **ProcessCache m_detailCounter race** — changed to `std::atomic<int>`
- **getTcpCounts/getUdpCounts underflow** — `maxEntries` calc now guarded against `size < offsetof`
- **FluxGameMode pid/name race** — `m_gamePid.load()` moved inside mutex for consistency with `m_currentGameName`
- **FluxMiningMode pid/name race** — same pattern fixed
- **main.cpp OOB read** — module enumeration capped at 1024 entries
- **Registry enum buffer overflow** — `ERROR_MORE_DATA` now handled with dynamic `realloc`
- **startupDelaySec unvalidated** — clamped to 5-600s range
- **LeakHunter HandleCloser** — made non-copyable to prevent double-`CloseHandle`

### Changed
- **Constants.h** — version bumped from 2.41.0 to 2.42.0

### Fixed
- **POWRPROF.dll crash (0xC0000005 at +0x59CE)** — `PowerReadFriendlyName` crashes deterministically on this Windows build; replaced with hardcoded GUID-based friendly name resolution (Balanced, High Performance, Power Saver, Ultimate Performance)
- **PowerPlanWidget crash** — `getAvailablePowerPlans()` now enumerates power plans via registry (`HKLM\SYSTEM\CurrentControlSet\Control\Power\UserPowerSchemes`) instead of calling `PowerEnumerate`, eliminating all POWRPROF calls from the 5s refresh timer
- **PowerManager background thread crash** — removed periodic `getActivePowerPlan()` call from `monitorLoop()` (was calling `PowerGetActiveScheme` every ~1s); thread now reads cached active plan name only
- **`PowerFreeMemory` typedef** — fixed signature from `PVOID` to `PVOID*` (was using wrong calling convention; function is not exported on this system anyway)
- **Startup markers** — added `fflush(f)` for immediate disk writes to `RAMFlux_startup.log`

### Changed
- **Constants.h** — version bumped from 2.15.0 to 2.41.0

## [2.40.0] — 2026-06-27

### Added
- **Scheduler Dashboard tab (SchedulerDashboardWidget)** — new tab showing CPU Limiter, Process Suspender, Network QoS, and ML Engine status in grouped panels
- **ML Engine panel** — live display of prediction score, confidence %, and sample count
- **CPU Limiter panel** — enable status, limit %, process threshold, and count of currently limited processes
- **Process Suspender panel** — enable status, suspend threshold, currently suspended count, and total suspended processes
- **Network QoS panel** — enable status and count of prioritized processes
- **Public accessors** — `CpuLimiter::limitedPids()`, `ProcessSuspender::suspendedPids()`, `NetworkQoS::prioritizedPids()`, and `FluxScheduler::cpuLimiter()/processSuspender()/networkQoS()` accessor methods

### Changed
- **CMakeLists.txt** — added `SchedulerDashboardWidget.cpp/h` to `UI_SOURCES`; version bumped to 2.40.0
- **MainWindow** — added "Scheduler" tab after QoS tab (16th tab)

## [2.39.0] — 2026-06-27

### Added
- **Memory QoS tab (QosWidget)** — new standalone tab with Rules table, Active Violations table, and enable/disable toggle
- **QoS Rules table** — 8 columns: pattern, min/max working set, max commit, page priority, I/O priority, efficiency mode, kill-on-violation; auto-refreshes every 5s
- **Active Violations table** — PID, process name, reason, resolved status; unresolved entries highlighted in red
- **Enforcement stats** — summary line showing total rules, applied, violations, and actions taken
- **Enable toggle** — checkbox to enable/disable QoS enforcement in real time

### Changed
- **HeuristicEngine** — added `qos()` public accessor returning `MemoryQoS&` (mirrors `dedup()` pattern)
- **CMakeLists.txt** — added `QosWidget.cpp/h` to `UI_SOURCES`; version bumped to 2.39.0
- **MainWindow** — added "QoS" tab after Prefetch tab (15th tab)

## [2.38.0] — 2026-06-27

### Added
- **Page Prefetch tab (PrefetchWidget)** — new standalone tab showing live PagePrefetcher status with 5s auto-refresh
- **Prefetch status** — dynamic status label (idle/green, rate-limited/yellow, success/green with reason)
- **Prefetch metrics** — targets, pages prefetched, data size, total operations + total lifetime pages
- **GroupBox layout** — compact single-panel design with status + horizontal metric row

### Changed
- **CMakeLists.txt** — added `PrefetchWidget.cpp/h` to `UI_SOURCES`; version bumped to 2.38.0
- **MainWindow** — added "Prefetch" tab after Classifier tab (14th tab)

## [2.37.0] — 2026-06-27

### Added
- **Process Memory Classifier tab (ClassifierWidget)** — new "Classifier" tab showing per-process memory usage pattern classification (Unknown/Steady/Burst/Periodic/Leaky) with 8-column table
- **Pattern coloring** — Leaky (red), Burst (yellow), Periodic (blue), Steady (green) for quick visual identification
- **Classification metrics** — per-process display of confidence %, current/mean working set (GB), growth rate (MB/min), and textual recommendation (trim/monitor/ignore)
- **Growth rate highlighting** — processes with ≥50 MB/min growth rate highlighted in red
- **Tracked count** — info label shows total tracked process count
- **Auto-refresh** — table updates every 5 seconds from FluxClassifier

### Changed
- **CMakeLists.txt** — added `ClassifierWidget.cpp/h` to `UI_SOURCES`; version bumped to 2.37.0
- **MainWindow** — added "Classifier" tab after Dedup tab (13th tab)

## [2.36.0] — 2026-06-27

### Added
- **Standby Memory Widget (StandbyWidget)** — new panel in the System Info tab showing per-process standby cache allocation with PID, process name, standby bytes, working set, and inference confidence
- **Standby table** — top processes sorted by standby allocation, color-coded: yellow ≥512 MB, green ≥256 MB
- **Total tracking stats** — label showing number of tracked processes and total standby bytes monitored
- **Auto-refresh** — data updates every 5 seconds from StandbyScanner (via HeuristicEngine report)

### Changed
- **CMakeLists.txt** — added `StandbyWidget.cpp/h` to `UI_SOURCES`; version bumped to 2.36.0
- **MainWindow** — added StandbyWidget to the System Info tab (after Power Plan panel)

## [2.35.0] — 2026-06-27

### Added
- **Power Plan Widget (PowerPlanWidget)** — new panel in the System Info tab showing AC/battery status, active power plan name, and a combo box to switch between available plans
- **Auto-detection of plan control** — the combo box is automatically disabled when the Responsiveness Slider's power plan automation is active (showing a note explaining why)
- **Plan switching** — selecting a plan from the combo immediately applies it via PowerManager::setPowerPlanByGuid()
- **Auto-refresh** — status and plan list update every 5s

### Changed
- **CMakeLists.txt** — added `PowerPlanWidget.cpp/h` to `UI_SOURCES`; version bumped to 2.35.0
- **MainWindow** — added PowerPlanWidget to the System Info tab (after Hibernate Assist)

## [2.34.0] — 2026-06-27

### Added
- **Hibernate Assist UI (HibernateWidget)** — new panel in the System Info tab showing real-time idle time, memory pressure, and hibernate/sleep advice
- **Advice status indicator** — color-coded label showing "HIBERNATE" (red), "SLEEP" (yellow), or "none needed" (green) with reason text
- **Force Hibernate / Sleep buttons** — manual triggers to immediately hibernate or sleep the system
- **Threshold controls** — idle threshold (5–240 min) and pressure threshold (10–100 pts) spinboxes with auto-hibernate checkbox
- **Auto-hibernate toggle** — enables automatic hibernation when both idle time and pressure exceed configured thresholds
- **Auto-refresh** — advice and metrics update every 3s

### Changed
- **CMakeLists.txt** — added `HibernateWidget.cpp/h` to `UI_SOURCES`; version bumped to 2.34.0
- **MainWindow** — added HibernateWidget to the System Info tab (after ResponsivenessSlider)

## [2.33.0] — 2026-06-27

### Added
- **Responsiveness Slider UI (ResponsivenessWidget)** — a real QSlider (0–10) in the System Info tab replacing the previously code-only slider concept, allowing users to dial between "Max Performance" (0) and "Max Responsiveness" (10)
- **Level descriptions** — each of the 11 levels has a human-readable description explaining what policies it enables (CPU limiting, process suspension, network QoS, power plan automation)
- **Policy indicators** — four live labels showing the current state of CPU cap, suspend threshold, network QoS, and power plan mode for the selected level
- **Automatic policy propagation** — slider changes are immediately applied to CpuLimiter, ProcessSuspender, NetworkQoS, and PowerPlanAutomation through the existing ResponsivenessSlider engine

### Changed
- **CMakeLists.txt** — added `ResponsivenessWidget.cpp/h` to `UI_SOURCES`; version bumped to 2.33.0
- **MainWindow** — added ResponsivenessWidget to the System Info tab (after Memory Compression group)
- **FluxScheduler** — exposed `setResponsivenessLevel()`, `responsivenessLevel()`, and `responsivenessSlider()` as public (was private); added inline getter for ResponsivenessSlider&

## [2.32.0] — 2026-06-27

### Added
- **I/O Bandwidth Throttle panel** — new section in the I/O tab showing disk queue length (color-coded green/red), throttled process count, enable/disable toggle, and disk queue threshold spinbox (0.5–10.0, step 0.5)
- **Process I/O Cost table** — new section in the I/O tab showing top processes by AI-calculated I/O cost score from IoCostTracker, with color coding (red ≥50, yellow ≥25)
- **Disk queue indicator** — real-time disk queue length reading updated every 3s, colored green when below threshold and red when at/above threshold
- **Throttle live controls** — enable/disable I/O bandwidth throttling and adjust the disk queue threshold in real time from the I/O tab

### Changed
- **IoDashboardWidget** — redesigned with 6 sections: System I/O, Top Readers, Top Writers, I/O Bandwidth Throttle (new), Process I/O Cost (new); split setup into `setupThrottlePanel()` and `setupCostPanel()` helpers
- **FluxScheduler::ioBandwidthThrottler()** — changed from const to non-const accessor to allow runtime threshold adjustment from UI
- **CMakeLists.txt** — version bumped to 2.32.0

## [2.31.0] — 2026-06-27

### Added
- **Memory Dedup Dashboard (DedupWidget)** — new "Dedup" tab showing per-process zero-page and duplicate-page detection results with estimated memory savings
- **Process dedup table** — 6 columns: PID, process name, pages scanned, zero pages (highlighted if >30% ratio), duplicate pages, and estimated savings in MB
- **Total savings display** — estimated aggregate RAM savings from cross-process page deduplication (shown in MB or GB)
- **Last scan indicator** — displays seconds since last dedup scan
- **Scan Now button** — triggers an immediate cross-process memory dedup scan using the latest telemetry snapshot
- **Auto-refresh** — table and stats update every 3 seconds via polling of MemoryDedup::lastReport()

### Changed
- **CMakeLists.txt** — added `DedupWidget.cpp/h` to `UI_SOURCES`; version bumped to 2.31.0
- **MainWindow** — added "Dedup" tab after Benchmark tab (12th tab)
- **HeuristicEngine** — added `dedup()` public accessor returning `MemoryDedup&` for direct dedup access

## [2.30.0] — 2026-06-27

### Added
- **Benchmark UI Widget (BenchmarkWidget)** — GUI interface for the existing CLI benchmark engine, accessible via new "Benchmark" tab
- **Configuration panel** — spin boxes for baseline (10–600s), post-optimization (10–600s), pressure duration (5–300s), and a checkbox to skip the pressure phase
- **Run/Cancel button** — starts benchmark on a background thread (`QThread`) to keep the UI responsive; cancels an in-progress run
- **Live progress** — phase label, progress bar (0–100%), and detail text update in real time as the benchmark executes
- **Results display** — shows efficiency score, free RAM/standby/hard fault/pressure improvements, total recovered MB, and detailed phase statistics (mean free RAM, standby, faults) for baseline and post-opt
- **Open Report Folder** — opens `benchmark_results/YYYYMMDD_HHMMSS/` in Windows Explorer after completion
- **Output directory** — each run creates a timestamped subdirectory named `benchmark_results/yyyyMMdd_HHmmss/`

### Changed
- **CMakeLists.txt** — added `BenchmarkWidget.cpp/h` to `UI_SOURCES`; version bumped to 2.30.0
- **MainWindow** — added "Benchmark" tab after Plugins tab, registered as the 11th tab

## [2.29.0] — 2026-06-27

### Added
- **Auto-update checker (UpdateChecker)** — queries GitHub Releases API to detect newer versions
- **UpdateInfo** — struct with `available`, `latestVersion`, `downloadUrl`, `changelogUrl`, `releaseName`, `error`
- **"Check for Updates"** — new menu entry under Help that checks GitHub for the latest release
- **Update dialog** — shows version info and opens download link if update available; reports "up to date" or errors gracefully
- **Silent check API** — `checkSilent()` for background version checking without user notification
- **Qt6::Network dependency** — new CMake link dependency for HTTP requests

### Changed
- **CMakeLists.txt** — added `Network` to Qt6 components and target_link_libraries; added UpdateChecker to IO_SOURCES; version bumped to 2.29.0

## [2.28.0] — 2026-06-27

### Added
- **Plugin Browser (PluginBrowserWidget)** — new "Plugins" tab in MainWindow showing all loaded plugins in a table with name, version, author, status, and file path
- **Plugin lifecycle management UI** — Refresh, Execute All, and Reload All buttons for runtime plugin management
- **Plugin info dialog** — click any plugin row to view full metadata and description
- **`plugins()` accessor** on PluginManager — exposes the internal LoadedPlugin vector for UI consumption
- **Plugin execution in scheduler loop** — `executeAll()` called each scheduler cycle (when not on battery boost)

### Changed
- **PluginManager** — added `plugins()` const getter returning `const std::vector<LoadedPlugin>&`
- **FluxScheduler** — calls `PluginManager::executeAll()` each cycle
- **CMakeLists.txt** — added PluginBrowserWidget to UI_SOURCES; version bumped to 2.28.0
- **MainWindow** — new "Plugins" tab added after Health tab

## [2.27.0] — 2026-06-27

### Added
- **Export/Import Configuration** — full state serialization to/from JSON files via File > Export/Import menu
- **ConfigIO module** (`src/io/ConfigIO`) — `collectExportData()`, `exportToFile()`, `importFromFile()`, `applyImportData()` round-trip cycle
- **Export data model** — `ExportData` struct with settings key-values, active profile index, custom profile config, and all process rules
- **JSON serialization** — uses `QJsonDocument` for human-readable formatted JSON; includes version marker for forward compatibility
- **Rule preservation** — exports all persistent and watchdog rules; clears existing rules on import and re-adds them (IDs reassigned)

### Changed
- **MainWindow** — new "Export Configuration..." and "Import Configuration..." actions in File menu; status bar feedback on completion
- **CMakeLists.txt** — new `IO_SOURCES` variable; version bumped to 2.27.0

## [2.26.0] — 2026-06-27

### Added
- **Power Manager (PowerManager)** — dedicated IModule for power state monitoring, replacing inline code in MainWindow
- **Battery monitoring thread** — polls `NtApi::getPowerStatus()` every 5s; detects AC↔battery transitions, charging state, and 5%+ changes in battery level
- **Auto battery boost** — automatically enables scheduler battery boost and cleaner battery-aware mode when on battery; restores on AC
- **Power plan detection** — reads active power plan name on each poll; detects plan switches
- **PowerState struct** — `onAC`, `batteryPercent`, `batteryLifeSeconds`, `charging`, `activePlanName`, `planChanged`
- **Callback subscription** — `onPowerStateChanged`/`unsubscribe` pattern for real-time UI updates
- **PowerStateChanged event** — new Constants::EventType for EventBus subscribers

### Changed
- **MainWindow** — replaced `m_powerTimer` + `onPowerCheck()` + `setBatteryBoost()` with PowerManager callback subscription; new `updateBatteryDisplay()` slot
- **Constants.h** — added `PowerStateChanged` to EventType enum and EventTypeNames array
- **main.cpp** — PowerManager registered in module system
- **CMakeLists.txt** — new `POWER_SOURCES` variable; version bumped to 2.26.0

### Removed
- **MainWindow::onPowerCheck()** — logic moved into PowerManager
- **MainWindow::setBatteryBoost()** — logic moved into PowerManager::monitorLoop
- **MainWindow::m_powerTimer** — replaced by PowerManager thread

## [2.25.0] — 2026-06-27

### Added
- **System Health & Diagnostics Dashboard (HealthDashboardWidget)** — unified health monitoring UI with overall score gauge, five category breakdowns (Memory, Disk, I/O, Leaks, System), and actionable recommendations list
- **DiagnosticsEngine** — periodic health evaluation thread (30s interval) that aggregates data from FluxTelemetry (pressure, faults, commit), disk space (GetDiskFreeSpaceEx), I/O queue (getDiskQueueLength), LeakHunter (active leaks), and system info (GlobalMemoryStatusEx)
- **CategoryHealth scoring** — each category scored 0–100 with status labels (Good/Fair/Poor/Critical) and dynamic warning generation
- **Overall health composition** — weighted score: Memory 35%, Leaks 20%, Disk 15%, I/O 15%, System 15%
- **Health history** — scores tracked for trend analysis (last 120 samples)
- **Callback subscription** — `onHealthUpdate`/`unsubscribe` pattern for real-time UI updates
- **Recommendation engine** — picks top warning from each category and displays in scrollable list
- **"Health" tab** in MainWindow — new tab with the complete dashboard

### Changed
- **CMakeLists.txt** — new `DIAGNOSTICS_SOURCES` variable; version bumped to 2.25.0
- **MainWindow** — `m_healthWidget` and `m_diagnosticsEngine` members; engine started on init, stopped in destructor

## [2.24.0] — 2026-06-27

### Added
- **Hibernate Assist (HibernateAssist)** — intelligent hibernation advisor that monitors system idle time and memory pressure; recommends or auto-triggers hibernate when both are elevated
- **`HibernateAdvice`** — result struct with `recommendHibernate`, `recommendSleep`, idle time, pressure score, reason string
- **Idle tracking** — reads user inactivity via `GetLastInputInfo` (minutes); configurable threshold (default 30 min)
- **Pressure threshold** — only triggers when pressure exceeds configurable score (default 75); auto-hibernate option
- **Action** — calls `SetSuspendState` with `SE_SHUTDOWN_NAME` privilege; supports manual `forceHibernate()`/`forceSleep()`
- **Cooldown** — 15-minute cooldown between advice emissions to avoid spamming

### Changed
- **FluxScheduler** — new `HibernateAssist` member; `m_hibernateEnabled` flag; `applyHibernateAssist()` called each cycle
- **CMakeLists.txt** — new `HIBERNATE_SOURCES` variable; links `powrprof`; version bumped to 2.24.0

## [2.23.0] — 2026-06-27

### Added
- **Forecast Tab (ForecastWidget)** — new UI tab showing real-time pressure forecasting with actual vs predicted overlay chart, 30s/60s/120s forecast lines, prediction accuracy metrics, ML engine status, and trend analysis
- **Pressure Forecast Chart** — 5-line QChart: actual pressure (blue solid), past predictions (green dashed), 30s forecast (red dotted), 60s forecast (dark red dotted), 120s forecast (magenta dotted); 120-sample rolling window
- **AI Metrics Panel** — accuracy %, total/correct/FP/FN counts, ML score + sample count, confidence %, trend direction + slope
- **Forecast Metrics Table** — 4-row table (Pressure, Confidence, ML Score, Trend) with current and forecast columns

### Changed
- **MainWindow** — new "Forecast" tab after Heatmap; `m_forecastWidget` updated every UI cycle via `updateAIInfo()`
- **CMakeLists.txt** — added `ForecastWidget.cpp/h` and `ThemeManager.cpp/h` (missing dependency) to build
- Version bumped to 2.23.0

## [2.22.0] — 2026-06-27

### Added
- **Event-Driven Architecture** — expanded `EventType` enum with 8 new events: `PressureChanged`, `PressureDropped`, `HardFaultStorm`, `HardFaultStormCleared`, `DiskQueueHigh`, `DiskQueueNormalized`, `BatteryLow`, `BatteryNormalized`
- **Event publishing** — HeuristicEngine posts pressure/storm events from `evaluateAndPost()`; FluxScheduler posts disk queue and battery events from scheduler loop and `setBatteryBoost()`
- **Edge detection** — pressure change, storm state, disk queue threshold, and battery state transitions are detected and published only on state change (not every cycle)
- **Zero-polling foundation** — modules can now subscribe to events instead of polling, enabling immediate reaction times and reduced CPU overhead

### Changed
- **Constants.h** — `EventType` enum expanded from 18 to 26 values; `EventTypeNames` array updated
- **HeuristicEngine.h** — added `m_lastPressureLevel`, `m_lastStormWarning` for edge detection
- **HeuristicEngine.cpp** — posts events on pressure level change, storm start/end
- **FluxScheduler.h** — added `m_lastDiskQueueHigh`, `m_lastBatteryLow` for edge detection
- **FluxScheduler.cpp** — posts DiskQueueHigh/Normalized events each cycle; posts BatteryLow/Normalized on setBatteryBoost
- Version bumped to 2.22.0

## [2.21.0] — 2026-06-27

### Added
- **Plugin System (PluginManager)** — lightweight DLL-based plugin sandbox for user-defined optimization routines; plugins expose `createPlugin()`/`destroyPlugin()` exported symbols
- **`IPlugin` interface** — `info()`, `initialize(ctx)`, `execute()`, `shutdown()` — plugins receive a whitelisted `IPluginContext` with safe read-only memory queries and limited actions
- **`IPluginContext`** — exposes `log()`, `getFreeMemoryBytes()`, `getStandbyMemoryBytes()`, `getDiskQueueLength()`, `getTotalPhysicalMemory()`, `setProcessPriority()`, `setProcessIoPriority()`, `getProcessCount()`, `getProcessInfo()` (static ProcessCache snapshot)
- **Sandbox constraints** — `executeAllWithTimeout()` with 5s per-plugin timeout via `std::async`/`std::future`; plugins loaded with `LOAD_LIBRARY_SEARCH_APPLICATION_DIR` for search path safety
- **Plugin discovery** — scans `plugins/` directory for `.dll` files at startup; creates directory if missing
- **`HeuristicReport`** — (unchanged) plugin results included in evaluation cycle
- **Pluggable architecture** — new `src/plugins/` directory with `IPlugin.h`, `PluginManager.h/.cpp`

### Changed
- **main.cpp** — registers `PluginManager` in `ModuleManager` before bootstrap
- **FluxScheduler** — plugin execution added to scheduler loop (via `executeAllWithTimeout()`) when module is enabled
- **CMakeLists.txt** — new `PLUGIN_SOURCES` variable added to build

## [2.20.0] — 2026-06-27

### Added
- **I/O Bandwidth Throttling (IoBandwidthThrottler)** — dynamically lowers I/O priority of background processes when disk queue pressure exceeds threshold; automatically restores priority once pressure normalizes with 30s cooldown
- **Disk queue monitoring** — reads `getDiskQueueLength()` from NTAPI; triggers on queue >= 2.0 (configurable)
- **Process protection** — skips RAMFlux, svchost, system, csrss, winlogon, services, and self
- **`FluxScheduler::applyIoBandwidthThrottling()`** — new method called every 5s when enabled

### Changed
- **FluxScheduler** — new `IoBandwidthThrottler` member; `m_iobandwidthEnabled` flag; getter/setter methods
- **CMakeLists.txt** — added `IoBandwidthThrottler.cpp/h` to `SCHEDULER_SOURCES` and header list
- Version bumped to 2.20.0

## [2.19.0] — 2026-06-27

### Added
- **Cross-Process Memory Dedup (MemoryDedup)** — detects duplicate memory pages across processes by hashing page contents (FNV-1a 64-bit); identifies zero pages and cross-process duplicate groups; estimates potential RAM savings from page combining
- **`DedupReport`** — scan result with per-process zero/duplicate stats, candidate groups with hashes, total estimated savings in MB
- **`HeuristicReport::dedup`** — new `DedupReport` field with dedup telemetry
- **Zero-page detection** — per-process zero page ratio tracking; processes with high zero ratios flagged for potential WS trim

### Changed
- **`HeuristicEngine::evaluateAndPost()`** — calls `m_dedup.scan(snap)` each cycle
- **CMakeLists.txt** — new `DEDUP_SOURCES` variable for `src/dedup/`

## [2.18.0] — 2026-06-27

## [2.17.0] — 2026-06-27

## [2.16.0] — 2026-06-27

## [2.15.0] — 2026-06-25

## [2.14.1] — 2026-06-10

### Added
- **EcoQoS (Efficiency Mode)** — `NtApi::setProcessEfficiencyMode(pid)` uses `SetProcessInformation(ProcessPowerThrottling)` (Win 10 1809+) to put background processes into CPU efficiency mode; reduces CPU/battery overhead on notebooks
- **Gentle Standby Clean** — `NtApi::gentleStandbyClean()` splits processes into active vs idle groups (via WS aging), elevates idle page priority before standby flush; cleans in chunks with `Sleep(500ms)` between chunks, disk queue check (`<1.5`) before each chunk; `FluxCleaner::gentleStandbyClean()` wrapper
- **Adaptive Standby Orchestration (3-tier)** — `FluxScheduler::applyStandbyOrchestration()` rewritten: Tier 1 (HF critical + high pressure + standby >256MB) → gentle clean; Tier 2 (HF critical + standby >1GB) → selective clean with idle WS trim; Tier 3 (standby >2GB preventive, no HF) → gentle clean
- **`selectiveStandbyClean()` rewritten** — now uses per-process working set trimming for idle (>120s) processes with moderate page priority, instead of relying on page priorities alone (which don't filter `NtSetSystemInformation(MemoryListStandby)` since all standby pages are cleared regardless)

### Fixed
- **CRITICAL: GetLastError() after LocalFree (RAMFluxHelper.cpp:33)** — `GetLastError()` was called after `LocalFree(sd)` in `ensureScheduledTask()`, which resets last-error to `ERROR_SUCCESS`; error code now saved before `LocalFree`
- **CRITICAL: GetLastError() after LocalFree (RAMFluxHelper.cpp:306)** — same bug in singleton mutex creation at `WinMain`; `singletonErr` now saved before `LocalFree(mutexSd)`
- **HIGH: CreateFileW missing FILE_FLAG_OVERLAPPED (HelperClient.cpp:20)** — pipe handle opened without `FILE_FLAG_OVERLAPPED` but used with `OVERLAPPED` structures in `ReadFile`; caused infinite hang since the handle was synchronous
- **HIGH: disablePrivilege used SE_PRIVILEGE_ENABLED (FluxNTAPI.cpp:95)** — `disablePrivilege()` was setting `0x2` (SE_PRIVILEGE_ENABLED) instead of `0x0` (SE_PRIVILEGE_DISABLED), meaning every `ScopedPrivilege` destructor was a no-op (privilege leak — privileges never actually disabled)
- **MEDIUM: readOk unconditionally TRUE (RAMFluxHelper.cpp:255)** — `readOk` was set to `TRUE` after `GetOverlappedResult` even when the call failed, masking read errors
- **MEDIUM: predictFuture() data race + deadlock (FluxNTAPI.cpp:697-700)** — `HardFaultHistory::predictFuture()` read `samples` without lock while `trendSlope()` held the mutex; adding a lock inside `predictFuture()` created a deadlock as it's called from `getPredictiveHardFaultInfo()` which also holds the lock; fixed by extracting `trendSlopeLocked()` as an unlocked helper

### Changed
- **FluxScheduler::applyStandbyOrchestration()** — complete rewrite from simple single-threshold logic to adaptive 3-tier model (see Added section)
- **FluxNTAPI::selectiveStandbyClean()** — now does per-process WS trim instead of page-priority-based standby filtering; more effective at freeing memory without impacting active process cache

## [2.14.0] — 2026-06-08

### Added
- **Memory Heatmap UI** — new "Heatmap" tab with treemap visualization of process memory: blocks sized by Working Set (sqrt-proportional), colored by RAM ratio (green→yellow→orange→pink), foreground process highlighting (pink border), live tooltips with PID/WS/CPU%/threads
- **Standby List Inteligente** — `NtApi::selectiveStandbyClean(maxPriority)` elevates page priority of critical processes (foreground, >500MB WS) before standby flush; `standbyPriorityDistribution()` for diagnostics; continuous orchestration in FluxScheduler every 30s
- **NUMA Optimization 2.0** — `NtApi::getBestNumaNodeForWorkload()` selects NUMA node with most free memory, penalized for L3 cache contention (>4 processes sharing L3); `pinProcessToBestNumaNode(pid)` pins to ALL cores on the node; game/miner auto-pinning in GameMode/MiningMode
- **Hard Fault Predictor 2.0** — 30-sample sliding window (`HardFaultHistory` deque + mutex) with linear regression; `predictFuture(30s)` predicts hard fault score 30s ahead; `estimatedTimeToThrashingSec` and `confidence` fields; preemptive deepClean (critical) / selectiveClean (warning) in scheduler
- **Process Memory Firewall** — `NtApi::setProcessMemoryLimit(pid, maxBytes, killOnViolation)` via `JOB_OBJECT_LIMIT_PROCESS_MEMORY`; 10-sample WS history detects >200MB growth → auto-limit; `quarantineMemoryLeak()` with CPU throttle + kill-on-violation; `releaseProcessMemoryLimit()` for cleanup
- **System File Cache Tuner** — `NtApi::setSystemFileCacheSize(min, max)` via `NtSetSystemInformation(SystemFileCacheInformation)`; reduces to 128MB (gaming) / 64MB (mining); original values saved and restored on mode exit
- **Memory Compression Manager** — proactive MaxPerformance mode during gaming/mining; decoder pool auto-expansion (256 pages on low hit rate, 128 pages on healthy+high hit rate); periodic savings logging

### Changed
- **GameMode**: now applies NUMA pinning, file cache reduction, compression MaxPerformance, and selective standby cleaning
- **MiningMode**: new in v2.14.0 — applies NUMA pinning, aggressive file cache reduction (64MB), and process memory firewall limits
- **FluxScheduler**: 6 new apply* methods in scheduler loop — `applyAdvancedCompressionTuning`, `applyPredictiveHardFaultManagement`, `applyProcessMemoryFirewall`, `applyStandbyOrchestration`, `applyFileCacheTuning`; each with configurable enable/disable, interval, and logging
- **FluxNTAPI**: ~20 new functions across 4 feature areas; includes `#include <deque>` for hard fault history; manual `JOB_OBJECT_LIMIT_*` defines for MinGW compatibility
- **Constants.h**: ~20 new constants for standby list (SL_*), file cache (FC_*), and feature tuning intervals

### Fixed
- **NUMA affinity pinning**: `setProcessNumaAffinityByNode()` now correctly uses `getNumaNodeCpuMask()` to pin to ALL cores on the target node, not just 1 core
- **Settings crash (strlen(NULL) em ProfileManager)**: crash ao clicar Save no SettingsDialog — `ProfileNames[profile]` acessava fora dos limites do array quando `QComboBox::currentIndex()` retornava -1, lendo NULL da memória anterior ao array e causando `strlen(NULL)` em `ucrtbase.dll`. Adicionada validação de bounds em `ProfileManager::setProfile()`, `SettingsDialog::saveSettings()` e `MainWindow::onProfileChanged()`. Adicionado item "Mining" faltante no Dashboard combo. Fix aplicado via análise de minidump (`C:\RAMFlux\RAMFlux_crash.dmp`)
- **Security audit fixes**: 19 fixes across multiple components:
  - **Privilege leak (C-1)**: `enablePrivilege()` refactored to RAII `ScopedPrivilege` class; 13 call sites updated; prevents permanent privilege elevation
  - **TOCTOU in revertAppliedRule (C-2)**: `AppliedRule::processName` added + `verifyProcessName()` check before restoring process state
  - **namedPipe timeout (C-3)**: `ReadFile` in `HelperClient.cpp` uses `OVERLAPPED` with 5-second timeout; prevents infinite wait on unresponsive helper
  - **Installer version (C-4)**: `installer.wxs` path updated from `build\deploy\` to `build2\deploy\`
  - **Version mismatch (C-5)**: `helper.rc`, `helper.manifest`, `app.manifest` synced to `2.14.0.0`
  - **HardFault math (C-6)**: `HardFaultPredictor::currentFaultsPerSec` uses delta time (`GetTickCount64`) instead of hardcoded `/60`
  - **File cache restore (C-7)**: Save/restore file cache limits on critical HF exit via `m_hfCriticalActive`
  - **Crash handler helper (C-8)**: `SetUnhandledExceptionFilter` + `MiniDumpWriteDump` with `dbghelp.h` added to `RAMFluxHelper`
  - **MAX_PATH truncation (C-9)**: `HelperClient.cpp` now detects truncation (`len >= MAX_PATH`) instead of silent overflow
  - **Exempt PID leak (C-10)**: `clearExemptPids()` called every scheduler cycle; `isMiningRunning()` prevents stale exempt PIDs
  - **sampleCount real (C-11)**: `HardFaultReport::sampleCount` reports `hist.samples.size()` instead of hardcoded 30
  - **>64 CPU groups (C-12)**: `getNumaNodeCpuMask()` enumerates all processor groups via `GetActiveProcessorGroupCount() + GetProcessorAffinity`
  - **Global mutex SD (C-13)**: `RAMFluxHelper.cpp` creates global mutex with `SECURITY_ATTRIBUTES` restricting to `SYSTEM + Administrators`
  - **Unchecked returns (C-14)**: `trimProcessWorkingSet()` and `setProcessMemoryLimit()` return values now checked for failure
  - **VRAM fallback (C-15)**: Uses `GlobalMemoryStatusEx` system memory pressure instead of hardcoded 60% when DXGI unavailable
  - **build.ps1 fixes (C-16)**: Generator changed to `MinGW Makefiles`; paths updated from `Release\` to `build\`
  - **CMakeLists.txt fixes (C-17)**: MSVC library linking removed for MinGW; `dbghelp` added for helper crash handler
  - **Dead code removed (C-18)**: `enableDebugPrivilege()` removed (replaced by `ScopedPrivilege`)
  - **Scheduler interval (C-19)**: `connectToHelper()` moved before scheduler start to prevent race on first cycle

## [2.13.0] — 2026-06-05

### Security
- **Named Pipe Hardening (SV-1/SV-2/SV-3)** — RAMFluxHelper.cpp:
  - Pipe DACL restricted from `WD` (Everyone) to `D:(A;;GA;;;SY)(A;;GA;;;BA)` — only SYSTEM + Administrators may connect
  - New `verifyClient()` method: validates connecting client via `GetNamedPipeClientProcessId` + `ProcessIdToSessionId`, rejecting cross-session connections
  - Removed `/rl highest` flag from scheduled task registration (AVG/Defender false-positive trigger)
- **Crash Dump Hardening (SV-4/SV-5)** — main.cpp:
  - Dump file created with `FILE_ATTRIBUTE_SYSTEM` to reduce accidental exposure of sensitive memory contents
  - `MiniDumpWithIndirectlyReferencedMemory` flag set for richer diagnostics without raw pointer disclosure
  - CLI `--dump_duration` bounded to 5–600s (was unbounded)

### Fixed
- **Handle leak in CpuLimiter (B-2)** — `CloseHandle(oldJob)` before overwriting `m_hJob` with new job object
- **Stale resume in ProcessSuspender (B-4)** — `isProcessRunning()` guard before `NtResumeProcess` on dead processes
- **CPU percent sanity cap (B-5)** — MemoryCollector caps CPU% at 10000%; above that returns 0 (race on first sample)
- **TOCTOU in NTAPI (B-6)** — `getTcpCounts()`/`getUdpCounts()` retry loop up to 3 attempts on `ERROR_INSUFFICIENT_BUFFER`
- **LeakHunter iteration limit (B-7)** — `maxRegions = 100000` cap and stall-address guard in `scanHeapRegions()`
- **HeuristicEngine data race (B-9)** — `tuneFromMetrics()` now called under `lock_guard` in `adjustModuleParams()`; removed unprotected call from `evaluateAndPost()`
- **ResponsivenessSlider data race (B-10)** — `int m_level` → `std::atomic<int>` for lock-free read in `getLevel()`
- **BenchmarkRunner crash (B-14)** — `if(m_cancelled) return` check at top of `runOptimization()`
- **HelperClient connect retry (B-15)** — `connectPipe()` retries up to 3×200ms on `ERROR_PIPE_BUSY`

### Changed
- Version bumped from 2.12.0 to 2.13.0 (patch + security release)
- Manuais updated (EN/PT) with security notes
- README updated with v2.13.0 changelog

## [2.11.0] — 2026-06-02

### Added
- **Process Memory Classifier (Phase 24)** — classify processes by memory usage pattern
  - `FluxClassifier` module (IModule) with per-process WS history tracking (deque 60 samples)
  - `ProcessMemoryProfile` enum: Unknown, Steady, Burst, Periodic, Leaky
  - `ProcessClassification` struct: profile, confidence, mean/stddev WS, growth rate, peak/mean ratio
  - `classifyPattern()` — 4-pattern detection: leaky (growth >50MB/min), burst (peak/mean >2x), periodic (zero-crossings ≥3), steady (CV <0.15)
  - `applyProcessClassification()` in FluxScheduler — 30s interval, feeds ProcessCache into classifier, trims leaky/burst processes with WS >500MB
  - Registered in ModuleManager + main.cpp
  - CMakeLists.txt updated with classifier sources
  - Constants: CL_MIN_WS_TRACK_BYTES, CL_LEAKY_GROWTH_THRESHOLD_MBPM, CL_BURST_PEAK_MEAN_RATIO, et al.

## [2.10.0] — 2026-06-02

### Added
- **Multi-level Cache Pressure (Phase 22)** — L1/L2/L3 cache-aware pressure calculation
  - `getCacheTopology()` — queries CPU cache sizes via `GetLogicalProcessorInformationEx` (RelationCache)
  - `getCachePressure()` — estimates per-level pressure based on active WS / cache size ratios
  - `CacheTopology` struct: l1/l2/l3 sizes, associativity, line size, cores, logical processors, sockets
  - `CachePressureInfo` struct: per-level pressure (0-1), overall score, L3 contention flag
  - Integrated into `FluxOptimizer::calculatePressureScore()` with 5% weight + 10pt L3 contention bonus
  - Constants: CP_CACHE_SCORE_WEIGHT, CP_L3_CONTENDED_BONUS, CP_L3/L2/L1_WEIGHT
- **Advanced Memory Compression (Phase 23)** — StoreAPI mode tuning & decoder pool management
  - `getStoreDecoderPoolInfo()` — query StoreAPI decoder pool (current/max pages, hit rate, allocation)
  - `setStoreDecoderPoolSize()` — expand decoder pool when hit rate is low during gaming
  - `setCompressionStoreMode()` — switch between Auto/MaxCompression/MaxPerformance modes
  - `getAdvancedCompressionInfo()` — aggregates compression ratio, decoder pool health, current mode
  - `FluxScheduler::applyAdvancedCompressionTuning()` — 60s interval, switches to MaxPerformance when harmful, expands pool during games with low hit rate
  - `FluxOptimizer::calculatePressureScore()` — +5ps when hit rate <30% and >1GB compressed
  - Constants: AC_POOL_LOW_HIT_RATE, AC_HARMFUL_RATIO_THRESHOLD, AC_DECODER_POOL_EXPAND_STEP, et al.

## [2.8.0] — 2026-06-02

### Added
- **Page File Auto-Tuning (Phase 21)** — automatic page file sizing via NTAPI
  - `getPageFileInfo()` queries all page files via NtQuerySystemInformation(SystemPageFileInformation)
  - `getPageFileRecommendation()` calculates optimal size from RAM + commit charge history
  - `setPageFileSize()` resizes via NtSetSystemInformation with SE_CREATE_PAGEFILE_NAME privilege
  - `applyPageFileTuning()` in FluxScheduler monitors pressure, triggers clean at 80%, resize at 90%
  - Constants: PF_TUNING_INTERVAL_MS, PF_RESIZE_PRESSURE_THRESHOLD, PF_CLEAN_PRESSURE_THRESHOLD, etc.
  - Suppressed during battery boost, 2-minute check interval
- Version bump 2.7.0 → 2.8.0

## [2.7.0] — 2026-06-02

### Added
- **Auto-tuning Engine** — `HeuristicEngine` agora trackeia acurácia das predições de pressão e ajusta dinamicamente `FluxCleaner::setCooldownMs()` e `FluxScheduler::setIntervalMs()` via feedback loop. `storePrediction()` armazena predições 30/60/120s, `evaluatePredictionAccuracy()` compara com valor real após horizonte expirar. `tuneFromMetrics()` mapeia acurácia em parâmetros: ≥80% → agressivo (cooldown 20s, intervalo 3s), <45% → conservador (60s, 12s). FP/FN ajustam confidence threshold. Pressão em alta reduz intervalo, anomalias severas forçam cooldown mínimo. Logging de cada ajuste.
- **HardFaultPredictor 2.0** — substitui o threshold binário antigo (3 ifs com constantes fixas) por preditor baseado em regressão linear. `HardFaultSample` com deque de 60 amostras (2 min), `computeSlope()` por ponteiro-para-membro, `evaluate()` calcula severityScore 0-100 ponderado (faults 40pts + trend 25pts + disk queue 20pts + standby 15pts). 5 níveis de severidade: None, Low (≥15), Medium (≥30), High (≥50), Critical (≥70). Storm warning dispara apenas quando severo + rising + low cache, com cooldown de 30s no log. Predição de faults em 30s via projeção linear.
- **Effectiveness Metrics** — `EffectivenessMetrics` com acurácia geral, acurácia recente (últimas 20), mean error, total/correct predictions, FP/FN. `TuningParams` com recommendedCooldownMs/IntervalMs/confidenceThreshold/aggressiveFactor. Exposto via `currentEffectiveness()` e `currentTuning()`.
- **PreemptiveConfidence** — `FluxOptimizer::calculatePressureScore()` agora integra AI prediction boost contínuo (`predictedPressure60s * predictionConfidence * 0.35`, cap 25pts), substituindo o trigger binário do HeuristicEngine no Scheduler. A IA agora influencia o pressure score gradualmente em vez de um override abrupto.
- **Game Mode 3.0 — DirectX Integration**:
  - **DXGI Video Memory API** — `NtApi::getVideoMemoryInfo()` querya VRAM total/dedicada/compartilhada + budget/usage atual via `IDXGIFactory1` + `IDXGIAdapter3::QueryVideoMemoryInfo`. `NtApi::getGameMemoryPressure()` combina VRAM pressure + system memory pressure em score único, detecta throttling (VRAM usage > budget). Carregamento dinâmico da dxgi.dll (sem linking obrigatório, fallback silencioso).
  - **Game-Specific Profiles** — 11 perfis pré-definidos (CS2, Valorant, Fortnite, CoD, Cyberpunk, Elden Ring, Dota 2, League, Overwatch 2, Apex, Warzone) com VRAM mínima, RAM recomendada, CPU/IO/page priority por jogo, flags competitive/disableProBalance/disableWsTrim/disableCompressionTuning.
  - **Pre-Game Memory Preparation** — `FluxCleaner::prepareForGame()` executa standby clean + modified clean + trim de processos >200MB inativos >30s antes de aplicar otimizações, maximizando RAM disponível.
  - **VRAM-Aware Cleaning During Gameplay** — `FluxGameMode::applyVramMonitor()` monitora VRAM a cada ~4s durante o jogo. Acima de 85%: standby clean se >256MB. Acima de 95% ou throttling: standby clean forçado. Histórico de 30 amostras.
  - **Competitive Mode** — timer resolution de 1ms (`SetWaitableTimerEx`), supressão de ProBalance/WS trim/compression tuning conforme perfil. Restaura timer para 15ms ao sair.
- **NTAPI/Kernel Expansion (5 features)**:
  - **NUMA Node Awareness v2** — `getProcessNumaNode(pid)` retorna o nó NUMA onde o processo executa via `GetProcessInformation(ProcessProcessorNumber)`. `setProcessNumaAffinityByNode(pid, node)` move o processo para um nó específico via `SetProcessGroupAffinity`. `getPreferredNumaForProcess(pid)` retorna node atual + memória disponível no nó. `FluxScheduler::applyNumaOptimization()` redistribui processos com WS >512MB entre nós quando o desbalanceamento de memória supera 20%, movendo processos do nó mais pressionado para o nó mais livre.
  - **Page Priority / Superfetch Integration** — `getSysMainServiceState()` consulta o serviço SysMain (Superfetch) via SCM API (OpenSCManager/OpenService/QueryServiceStatusEx), retornando running state, PID e startup type. `setSysMainServiceEnabled()` inicia/para o serviço. `FluxScheduler::applyPagePriorityOrchestration()` estendida para considerar o estado do SysMain na estratégia de prioridades de página.
  - **Memory Compression Tuning** — `getCompressionEfficiency()` computa ratio real (uncompressed/compressed), GB compressed vs uncompressed, harmful flag (<1.2x), savingsPercent. `getCompressionStoreInfo()` detalha páginas totais/compactadas/descompactadas e average compression ratio do store. `FluxScheduler::applyCompressionTuning()` monitora continuamente a eficiência; após 3+ amostras consecutivas com ratio <1.2x, loga alerta sugerindo desabilitar Memory Compression. Restaura aviso quando eficiência retorna.
  - **Hard Fault Prediction (NTAPI level)** — `getHardFaultPrediction()` agrega page faults dos processos + standby list size + disk queue length em score 0-100 (faults 40pts + disk queue 25pts + standby 20pts + trend 15pts). Não substitui o `HardFaultPredictor` do HeuristicEngine (que opera com 60 amostras e regressão linear), mas serve como quick-check síncrono para outras partes do sistema.
  - **Working Set Aging** — `getProcessWsAge(pid)` retorna segundos desde a última alteração significativa (>5MB) do working set. `trimIdleProcesses(thresholdBytes, idleSeconds)` faz trim apenas de processos com WS > threshold e inativos há N segundos. `FluxScheduler::applyWsAgingTrim()` executa a cada 30s (configurável via `setWsAgingIdleSeconds()`/`setWsAgingThresholdBytes()`), trimando processos ociosos com >100MB WS.

### Changed
- `HeuristicReport` estendido com `EffectivenessMetrics effectiveness`, `TuningParams tuning`, `HardFaultPrediction hardFault`
- **FluxOptimizer::calculatePressureScore()** — removido o trigger binário do HeuristicEngine no Scheduler; a decisão agora flui naturalmente pelo pressure score boostado no Optimizer
- **FluxScheduler::schedulerLoop()** — adicionadas 3 novas chamadas: `applyNumaOptimization()`, `applyWsAgingTrim()`, `applyCompressionTuning()`
- **FluxGameMode** — reescrito para Game Mode 3.0: `GameProfile` struct com 11 perfis, `VramSample` tracking, `applyVramMonitor()` para limpeza VRAM-aware, `setCompetitiveModeEnabled()`/`setVramAwareCleaningEnabled()`, pre-game memory preparation via `FluxCleaner::prepareForGame()`
- **FluxCleaner** — novo método `prepareForGame()` para limpeza agressiva pré-jogo
- **FluxNTAPI.h** — adicionadas 12 novas structs (`ProcessNumaInfo`, `SysMainState`, `HardFaultPrediction`, `CompressionEfficiency`, `CompressionStoreInfo`, `VideoMemoryInfo`, `GameMemoryPressure`) e 16 novas funções (mais `getVideoMemoryInfo`, `getGameMemoryPressure`, `setTimerResolution`)
- Version bumped from 2.6.0 to 2.7.0
- Docs atualizados (PHASES.md, CHANGELOG.md, ARCHITECTURE.md)

## [2.5.2] — 2026-06-01

### Added
- **ThemeManager com 3 temas** — Novo `ThemeManager` singleton gerando QSS global para os temas Catppuccin Mocha (dark), Catppuccin Latte (light) e Nord (dark blue). `applyTo()` permite reestilização em runtime sem restart.
- **Seletor de temas no SettingsDialog** — Novo grupo "Appearance" com `QComboBox` populado via `ThemeManager::themeCount()`/`themeName()`. Tema salvo em QSettings e aplicado automaticamente no startup e ao salvar configurações.
- **Code audit completo** — 18 arquivos fonte revisados: verificação de segurança (injeção, buffer overflow, race conditions), null pointers, resource leaks e conformidade com boas práticas C++20/Qt6.

### Fixed
- **FluxProcessAnalyzer: CPU% sempre zero** — `calculateCpuUsage()` atualizava `it->second` antes de calcular o delta, causando `totalDelta = 0` permanente. Cálculo movido para antes da atualização do sample.
- **MemoryCollector: page size hardcoded** — `coldPageRatio` usava 4096 fixo em vez de `SYSTEM_INFO.dwPageSize`, causando erro em ARM64/PAE. Corrigido com `GetSystemInfo` dinâmico.
- **FluxGameMode: IO priority nunca restaurada** — `applyGameOptimizations()` rebaixava I/O priority para LOW mas nunca salvava o valor original. Adicionada função `getProcessIoPriority()` em `FluxNTAPI` + captura do estado original antes da modificação.

### Changed
- Version bumped from 2.5.1 to 2.5.2 (patch release)
- README updated with v2.5.2 changelog
- Manuals updated (EN/PT) with new version
- CMakeLists.txt: `ThemeManager.cpp` adicionado aos `UI_SOURCES`

## [2.5.1] — 2026-06-01

### Fixed
- **Logger::rotateLog() command injection** — `std::system(powershell ...)` substituído por `fs::rename`/`fs::remove` seguro, eliminando vetor de injeção via single-quote no path do log
- **ConsoleWidget callback use-after-free** — callback agora captura `QPointer<MainWindow>` em vez de raw `this`, e usa `qApp` como receiver do `invokeMethod`
- **MainWindow.h includes duplicados** — removidas 6 linhas duplicadas de `QLabel`, `QPushButton`, `QComboBox`, `QProgressBar`, `QVector`
- **ProcessCache/FluxNTAPI/FluxProcessAnalyzer: `pmc.cb` não inicializado** — `PROCESS_MEMORY_COUNTERS_EX.cb` agora é setado antes de chamar `GetProcessMemoryInfo` (6 call sites)
- **FluxProcessAnalyzer: overflow aritmético no CPU%** — delta de tempo absoluto convertido para `int64_t` com proteção contra wrap-around
- **RAMFluxHelper: `wsprintfW` em buffer fixo** — 3 call sites migrados para `StringCchPrintfW` segura (com overflow detection)
- **RAMFluxHelper: Named pipe sem fallback de SD** — se `createPipeSecurity()` falha, usa SD restritivo `D:(A;;GA;;;WD)` como fallback

### Changed
- **NTAPI magic numbers → named constants** — `SystemMemoryListInformation`, `SystemFileCacheInformation`, `SystemCacheInformation`, `SystemPagedPoolInformation`, `SystemMemoryCompressionInformation`, `ProcessPagePriority` definidos como `inline constexpr` em `FluxNTAPI.cpp`
- **Benchmark printf → Logger** — Summary do benchmark substituído de `printf` para `Logger::instance().info()`
- **Logger: callback chamada fora do mutex** — `m_callback` é copiada e invocada após liberar `m_mutex`, eliminando deadlock por reentrância
- **HistoryBuffer: thread-safe + retorno por valor** — `add()`/`latest()`/`clear()` protegidos por `std::mutex`; `latest()` retorna `MemorySnapshot` por valor em vez de `const&`

### Added
- **cpuPercent tracking** — `MemoryCollector::computeProcessCpuPercent(pid)` usa `GetProcessTimes` com two-sample delta para popular `ProcessMemoryBreakdown::cpuPercent`. Histórico com cleanup automático (>500 PIDs)
- **HelperClient: `strsafe.h`** — `StringCchPrintfW`/`StringCchCopyW` substituem `wsprintfW`/`wcscpy` para segurança de buffer
- **Compression-Aware Pressure Scoring** — `FluxOptimizer::calculatePressureScore()` agora detecta compressão ineficiente (< 1.2x) e adiciona penalidade proporcional de até 20 pontos, forçando limpeza preventiva quando o engine de compressão está sobrecarregado
- **Page Priority Orchestration** — `FluxScheduler::applyPagePriorityOrchestration()` monitora a taxa de compressão e automaticamente reduz a prioridade de página (`PAGE_PRIORITY_LOW`/`BELOW_NORMAL`) de processos com working set elevado quando a compressão está ineficiente por 2+ amostras consecutivas. Prioridades são restauradas quando a eficiência retorna ao normal
- **Cold Page Detection & Trim** — `ProcessCache` agora captura snapshots de working set via `QueryWorkingSetEx` a cada ciclo de detalhamento. Páginas compartilhadas presentes em snapshots consecutivos são classificadas como **cold pages**. `FluxCleaner::trimColdPages()` só faz trim de processos com >30% de cold pages e >30s de idade, eliminando stutter causado por trim cego. Integrado em `adaptiveClean()` e `deepClean()`
- **LeakHunter Heap Analysis** — `LeakHunter::scanHeapRegions()` usa `VirtualQueryEx` para escanear o espaço de endereço de processos suspeitos, contando regiões `MEM_PRIVATE|MEM_COMMIT` e totalizando bytes comprometidos. `analyzeProcess()` agora também coleta `PrivateUsage` via `PROCESS_MEMORY_COUNTERS_EX` e calcula `computeAcceleration()` (taxa de crescimento entre amostras recentes vs antigas). A detecção combina 4 heurísticas: (1) crescimento de WS >50 MB + >20%, (2) crescimento privado >30 MB + aceleração >1.2x, (3) fragmentação (>1000 regiões heap) + crescimento privado, (4) crescimento privado desproporcional ao WS (>2x). `LeakReport` estendido com `heapCommittedBytes`, `heapRegionCount`, `privateGrowthBytes`, `accelFactor`, `heapAllocBase`, `privatePeak`
- **Predictive Engine Multi-Horizonte** — `PressurePredictor::predict()` agora computa slopes separados para curto (6 amostras ≈60s), médio (30 ≈5min) e longo prazo (todas), com blending adaptativo conforme o horizonte de predição. Adicionado R² (coeficiente de determinação) como medida de qualidade do ajuste. Confiança calculada como `R² × horizonDecay × sampleCount`. Novos campos em `PredictionResult`: `predictedFreeGB`, `predictedHardFaults`, `shortTermSlope`, `mediumTermSlope`, `rSquared`. `detectAnomaly()` agora analisa 3 métricas (pressure, freeRam, hardFaults) e retorna a anomalia mais significativa com nome da métrica em `AnomalyResult::metricName`
- **LeakHunter na UI** — Aba "Leak Hunter" agora exibe tabela com 9 colunas: PID, Process, WS Growth, Private Growth, Accel, Heap Regions, Heap Committed, WS Peak, Status. Atualização automática a cada 5s. Status com 4 classificações: PRIVATE LEAK (vermelho, crescimento privado 2x > WS), ACCELERATING (vermelho, aceleração >1.5x), WATCHING (amarelo, aceleração >1.2x), FRAGMENTED (amarelo, >1000 regiões heap). Barra de info mostra total de leaks ativos e memória desperdiçada. Controles: Refresh manual e toggle enable/disable
- **Cold Page Ratio na Dashboard** — `ProcessMemoryBreakdown` estendido com `coldPageBytes` e `coldPageRatio`; populado via `MemoryCollector` a partir dos dados do `ProcessCache`. `ProcessListWidget` ganhou coluna "Cold Pages" com formatação "X MB (Y%)" e destaque amarelo se ratio >30%. Detalhes do processo (double-click) também exibem cold pages quando disponíveis
- **Game Mode 2.0 (Memory QoS)** — `FluxGameMode` agora rastreia o PID do jogo detectado e aplica otimizações ativas: (1) **Game boost** — página priority `NORMAL`, CPU priority `ABOVE_NORMAL`, I/O priority `HIGH` no processo do jogo; (2) **Background throttle** — processos não-jogo com WS >50 MB têm página priority rebaixada para `VERY_LOW`/`LOW` (conforme WS), CPU priority para `BELOW_NORMAL`, I/O priority para `LOW`. Todos os estados originais são restaurados quando o jogo termina via `restoreNormalOptimizations()`. `FluxScheduler::applyProBalance()` e `applyPagePriorityOrchestration()` agora ignoram o PID do jogo, evitando interferência. Novos métodos: `currentGamePid()`, `setGameOptimizationsEnabled()`, `isGameOptimizationsEnabled()`

## [2.5.0] — 2026-05-31

### Added
- **BenchmarkRunner** — scientific benchmarking system built into RAMFlux with multi-phase methodology (warmup/baseline/pressure/optimization/post-opt), 18+ metrics collected at 1s intervals, and full statistical analysis (mean, median, σ, percentiles)
- **CLI flag `--benchmark`** — runs controlled benchmark from command line with configurable phase durations
- **Tri-format reports** — CSV raw data, JSON statistical summary, Markdown scientific report with methodology documentation
- **Efficiency Score** — composite metric (0–100) combining free memory improvement, standby reduction, hard fault impact, and pressure reduction

### Changed
- Version bumped from 2.4.0 to 2.5.0 (minor feature release)
- README updated with benchmark methodology and real results
- Manuals updated (EN/PT) with new version

## [2.4.0] — 2026-05-30

### Added
- **ProcessCache** — singleton thread-safe cache de processos com agregação por sessão (`SessionInfo`), rastreamento de delta de working set (`wsDelta`, `lastWsChangeTime`), e lookup por nome/PID. Ciclo de detalhamento a cada 3 atualizações para reduzir overhead.
- **FluxProcessAnalyzer** — novo módulo `IModule` para análise de processos: top-N por memória, detecção de vazamentos (>500 MB), amostragem de CPU via `GetProcessTimes` com two-sample delta, e trim de working set individual.
- **Logger avançado** — rotação automática de logs com `setMaxLogSize()`/`setMaxBackupFiles()`/`setCompressBackups()`, compressão GZip em thread destacada, callback channel para consumo externo (ConsoleWidget), timestamps com precisão de milissegundos.
- **EventBus com dispatch thread** — fila de trabalho assíncrona (`m_workQueue`), thread dedicada com condition variable, assinaturas curinga (`subscribeWildcard` com suporte a prefixo/sufixo/glob), `start()`/`stop()` lifecycle.
- **FluxTelemetry + MemoryCollector + MemorySnapshot** — subsistema completo de telemetria: coleta periódica event-driven com `CreateMemoryResourceNotification`, polling adaptativo (1s/2s/5s conforme pressão), snapshot com 30+ campos (RAM, page file, NUMA, CPU, disk queue, compressão, hard faults), ring buffer de histórico (3600 amostras), postagem de eventos `MemoryUpdated`/`PressureHigh`/`PressureCritical`.
- **ProfileManager** — novo módulo com 5 perfis (Economy, Balanced, Performance, Gaming, Custom) que reconfiguram dinamicamente Telemetry, Scheduler, Cleaner, LeakHunter, GameMode e Optimizer. Callback de troca de perfil posta `EventType::ProfileChanged`.
- **ProBalance** — gerenciamento dinâmico de prioridades: processos com working set > 512 MB em NORMAL/ABOVE_NORMAL/HIGH são rebaixados para BELOW_NORMAL. Prioridades originais restauradas quando o processo libera memória ou ProBalance é desligado.
- **Battery Awareness** — monitoramento de bateria via `SYSTEM_POWER_STATUS`, modo low-power automático ao desconectar AC (polling estendido, supressão de defrag/file cache, battery boost no scheduler), indicadores coloridos na status bar (verde AC, amarelo carregando, laranja baixo, vermelho crítico).
- **RAMFluxHelper (Processo Elevado)** — novo executável separado que sobe como servidor de pipe nomeado com `runas` (UAC), executando operações privilegiadas (standby, modified, working set, file cache, combined, trim) com `NtSetSystemInformation`. Singleton via mutex, criação automática de scheduled task no logon.
- **HelperClient** — comunicação com o helper via pipe nomeado (`\\.\pipe\RAMFluxHelper`): `sendCommand()`, `trimProcess(pid)`, `launchHelper()` com `ShellExecuteExW`, detecção de health via mutex.
- **Crash Handling** — `SetUnhandledExceptionFilter` com geração de minidumps (`MiniDumpWriteDump`) com timestamp, supressão de diálgos de erro do Windows via `SetErrorMode`.
- **ConsoleWidget** — nova aba Console com visualização de logs em tempo real, filtro por nível (All/Debug/Info/Warning/Error), cores Catppuccin Mocha, máximo 5000 blocos, atalho Ctrl+C para copiar.
- **Memory Map UI** — nova aba com breakdown visual de memória física em 8 categorias (Active, Standby, Modified, Modified No-Write, Transition, Zeroed, Free, Bad) com progress bars, atualização a cada 10s.
- **Modo headless** — argumentos `--headless`/`--silent` para executar sem GUI, `--clean`/`-c` para limpeza one-shot, `--report`/`-r` para relatório formatado, `--json`/`-j` para saída JSON. Tratamento de SIGINT/SIGTERM.

### Added (NTAPI Kernel Expansion)
- **Memory Compression API** — `getCompressedMemorySize()`, `getCompressionTotalData()`, `getCompressionRatio()`, `CompressionInfo`/`getCompressionInfo()` (lê estado do Memory Compression engine, threshold e swapfile compression), `setCompressionEnabled(bool)` via `NtSetSystemInformation(0x54)`.
- **System Cache & Paged Pool** — `SystemCacheInfo`/`getSystemCacheInfo()` via `NtQuerySystemInformation(0x15)`, `PagedPoolInfo`/`getPagedPoolInfo()` via `NtQuerySystemInformation(0x42)`.
- **NUMA Node Awareness** — `NumaInfo`/`getNumaInfo()` com `GetNumaHighestNodeNumber` + `GetNumaAvailableMemoryNodeEx`.
- **Page Priority Management** — `setProcessPagePriority(pid, priority)` via `NtSetInformationProcess(0x27)` com constantes `PAGE_PRIORITY_*` em Constants.h.
- **Disk Performance Queue** — `getDiskQueueLength()` via `IOCTL_DISK_PERFORMANCE` com cálculo de delta de transfer count.
- **Power Status** — `PowerStatus`/`getPowerStatus()` lê `SYSTEM_POWER_STATUS` (AC line, battery %, battery life, charging state).
- **Physical Memory Breakdown** — `PhysicalMemoryBreakdown`/`getPhysicalMemoryBreakdown()` extrai contagens de páginas active/standby/modified/transition/zero/free/bad via `NtQuerySystemInformation(0x50)`.
- **Full-Screen Detection** — `isFullScreenAppActive()` detecta app full-screen ativo (exceto Progman/WorkerW) para game mode automático.
- **Per-Process IO Stats** — `ProcessIoStats`/`getProcessIoStats()` com contadores de read/write/other ops e bytes.
- **Generic Privilege Management** — `enablePrivilege(wchar_t*)` com helpers `enableLockMemoryPrivilege()`, `enableDebugPrivilege()`, `enableBackupPrivilege()`.
- **Process Batch Operations** — `trimAllProcesses()`, `getProcessesWithLargeWS(threshold)`, `getProcessPageTableUsage(pid)`.

### Changed
- **MainWindow expandida** — 6 abas (Dashboard, Processes, Leak Hunter, Memory Map, System Info, Console), 11 memory cards, game detection a cada 5s, monitoramento de bateria, painel AI Info com workload/predições/anomalias, painel NUMA Nodes, painel Memory Compression, status bar com badge de IA, indicadores de cor por pressão.
- **Adaptive Cleaning** — `FluxCleaner::adaptiveClean(snap)` com limpeza seletiva baseada em pressure score (deepClean se ≥ High, quickClean se free mem baixo, standby clean se hard faults altos), battery-aware (skip defrag/file cache em bateria), idle-aware process trimming.
- **Scheduler aprimorado** — integração com HeuristicEngine para limpeza preditiva se IA prevê pressão ≥ High em 60s com ≥60% confiança, scheduled deep clean configurável (1h default), ProBalance dinâmico, battery boost mode.
- **HeuristicEngine registrado como módulo #9** — workload classifier (7 tipos), pressure predictor (30/60/120s), detecção de anomalias (>3σ), integração com scheduler.
- Versão atualizada de 2.3.0 para 2.4.0 (major feature + security release)
- Manuais atualizados (EN/PT) com nova versão

### Security
- **Path traversal em banner da Console** — entrada de terceiros (e.g., banner em ASCII gerado por ferramentas externas) era concatenada em `QString` sem sanitização. Implementado `HtmlSanitizer` que filtra tags `<script>`, `on*=` e `javascript:` via regex.
- **Argument injection no PrivilegedHelper** — parâmetros posicionais recebidos pela pipe nomeada eram passados para `CreateProcess` sem validação. Implementado `ArgValidator` com whitelist de padrões alfanuméricos.
- **Buffer overflow em `NtApi::getProcessList()`** — `NtQuerySystemInformation` era chamada com buffer de tamanho fixo (64 KB) sem loop de retry. Substituído por alocação dinâmica com dobramento progressivo até 16 MB.

---

## [2.3.0] — 2026-05-29

### Fixed
- **Deadlock no HeuristicEngine** — `evaluateAndPost()` tentava travar `m_mutex` recursivamente enquanto já estava travado em `feedSnapshot()`, impedindo a MainWindow de abrir
- **Status bar badge com `%%` duplicado** — `QString::arg()` tratava `%%` como literal, exibindo `AI: Gaming (85%%)`

### Added
- **Módulo de IA/ML** — HeuristicEngine, WorkloadClassifier, PressurePredictor com thread dedicada (2s)
- **Identificação de carga de trabalho** — 7 tipos (Gaming, Development, Media, Browser, Heavy, Idle, Unknown)
- **Previsão de pressão** — regressão linear com predição em 30/60/120s e detecção de anomalias (>3σ)
- **Limpeza preventiva** — FluxScheduler consulta HeuristicEngine para gatilho antecipado se IA prevê pressão ≥ High em 60s com ≥60% confiança
- **Badge de IA na barra de status** — lilás, exibe workload + confiança (`AI: Gaming (85%)`)
- **Grupo AI Heuristics** na aba System Info — workload, predições, tendência, status de anomalia
- **Checkbox "Enable AI Heuristics"** em Settings → Behavior, persistido em QSettings
- **HeuristicEngine registrado como módulo #9** em main.cpp

### Added (NTAPI Kernel Expansion)
- **Working Set Aging** — `ProcessEntry` agora trackeia `lastWsChangeTime`; `FluxCleaner::trimProcesses()` pula processos ativos nos últimos 30s. Novo método `trimProcessesIdleOnly()` para trims conservadores.
- **Hard Fault Prediction** — `HeuristicEngine::evaluateAndPost()` correlaciona page faults + disk queue + standby list para prever tempestades de hard fault. Nova função `NtApi::getDiskQueueLength()` via `IOCTL_DISK_PERFORMANCE`.
- **NUMA Node Awareness** — `NtApi::getNumaInfo()` usa `GetNumaHighestNodeNumber` + `GetNumaAvailableMemoryNodeEx`. Novo grupo "NUMA Nodes" na System Info com memória disponível por nó.
- **Page Priority Management** — `NtApi::setProcessPagePriority(pid, priority)` via `NtSetInformationProcess` (class 0x27). Constantes `PAGE_PRIORITY_*` em `Constants.h`.
- **Memory Compression Tuning** — novo grupo "Memory Compression" na System Info. Alerta vermelho se compression ratio < 1.2x com hard faults > 50/s.

### Changed
- Versão atualizada de 2.2.1 para 2.3.0 (minor feature release)
- Manuals atualizados (EN/PT) com nova versão

---

## [2.2.1] — 2026-05-29

### Added
- (placeholder for 2.2.1 changes)

---

## [1.1.2] — 2026-05-27

### Fixed
- **UI: fundo branco na aba Console** — `ConsoleWidget` não tinha background explícito, herdando o tema claro do sistema. Adicionado `setStyleSheet("background-color: #1e1e2e")` no widget
- **UI: linhas brancas alternadas na tabela de processos** — `setAlternatingRowColors(true)` sem regra `::item:alternate` usava a cor padrão do sistema (branca). Adicionado `QTableWidget::item:alternate { background-color: #181825; }`

### Changed
- `ConsoleWidget`: fundo explícito `#1e1e2e` (Catppuccin Base) no widget raiz
- `ProcessListWidget`: estilo `::item:alternate` com `#181825` (Catppuccin Mantle) para uniformizar fundo escuro

---

## [2.1.0] - 2026-05-28

### Added
- **Privileged Helper Process** (`RAMFluxHelper.exe`) — separate process with `requireAdministrator` manifest for performing admin-level memory operations (standby, modified, working set, combined, file cache, defrag)
- **Named Pipe IPC** — helper opens `\\.\pipe\RAMFluxHelper` with NULL DACL; `FluxCleaner` sends commands and falls back gracefully if helper is unavailable
- **Auto-scheduled Task** — helper self-registers `schtasks /create /sc onlogon /rl highest` on first elevated run; subsequent logons launch helper silently without UAC
- **Deferred helper launch** — `MainWindow` attempts to launch helper 3s after startup if not running (triggers UAC once; scheduled task handles thereafter)

### Fixed
- **Crash on startup (0xc0000005)** — null pointer dereference on `m_statusLabel` caused by `onToggleAutomation` accessing the label before `setupUI()` created it. Moved `m_statusLabel`/`m_cleanStatsLabel`/`m_gameModeLabel` creation before `setupDashboardTab()`.
- **Scheduler never ran** — `autoCheck->setChecked(true)` before `connect()` in `setupDashboardTab` caused `toggled(true)` to fire unhandled; order fixed.

### Changed
- **Adaptive cleaning logic** — `adaptiveClean()` now triggers `quickClean()` when free memory is low regardless of standby size; pressure >= HIGH (75) triggers `deepClean()` instead of CRITICAL (100)
- **Removed redundant deepClean from schedulerLoop** — all cleaning delegated to `adaptiveClean()`
- **Manifest** changed from `requireAdministrator` to `asInvoker` to enable Windows auto-start without elevation
- **`NtQuerySystemInformation(0x53)` removed** — synthetic "System File Cache (Total)" entry incompatible with Win11; removed from `FluxNTAPI`
- **File Summary tab removed** — showed loaded modules (DLL/EXE) per process with summed `SizeOfImage`, not actual cache data; misleading Total Size values. Removed `setupFileCacheTab`, `onFileCacheUpdated`, `getTopFileCache`, `FileCacheInfo` struct, and related members/timer
- **NTDLL function pointers cached** — `NtSetSystemInformation` and `NtQuerySystemInformation` resolved once via `GetProcAddress` instead of on every call (was 7x per clean operation)
- **Process list timer merged** — `ProcessListWidget::m_refreshTimer` (3s) removed; refresh triggered from `MainWindow::onMemoryUpdated()` (2s), eliminating redundant timer
- **Adaptive polling added** — `changeEvent` handler in MainWindow reduces telemetry/scheduler/UI intervals when minimized or on battery (from AC); battery detection via `GetSystemPowerStatus`
- **~50 dead code files removed** — entire `executor/`, `process/`, `analytics/`, `stability/`, `kernel/`, `kernelbridge/`, `settings/`, `performance/`, `logging/`, `ui/dashboard/`, `ui/theme/` directories plus individual stub files in `telemetry/`, `core/`, `optimizer/`, and `SplashScreen.*` — codebase reduced from ~120 to ~49 source files

---

## [1.1.1] — 2026-05-27

### Fixed
- **Crash: thread detached sem join no destrutor** — `onFileCacheUpdated()` criava `std::thread` com `.detach()`, permitindo que a thread continuasse executando durante o desligamento do CRT, acessando memória destruída. Agora a thread é armazenada como membro (`m_fileCacheThread`) e unida no destrutor da MainWindow
- **Crash: use-after-free entre QPointer e invokeMethod** — o código convertia `QPointer` em raw pointer antes de chamar `invokeMethod(raw, ...)`, criando uma janela onde a MainWindow podia ser destruída entre a leitura e o uso. Agora usa `qApp` como receiver e o `QPointer` é verificado dentro da lambda (na GUI thread)
- **Crash: thread file cache sem proteção SEH** — access violations dentro de `getTopFileCache` podiam não ser capturados por `catch(...)` dependendo do modo de exceção do MinGW; o recuo para `std::thread` joinable com `try-catch` externo garante tratamento adequado

### Changed
- `onFileCacheUpdated()`: thread migrada de `.detach()` para `m_fileCacheThread` joinable; receiver trocado de raw pointer para `qApp`
- Destrutor `~MainWindow()`: `m_fileCacheThread.join()` executado antes de qualquer cleanup (Logger, EventBus)

---

## [1.1.0] — 2026-05-27

### Added
- User manual in Help menu (Português / English)
- CHANGELOG.md following Keep a Changelog format
- Versioning standard documented in README

### Fixed
- **Crash: EventBus use-after-free** — callbacks now use `QPointer<MainWindow>` instead of raw `this`, preventing dangling pointer access when MainWindow is destroyed while a telemetry callback is in flight
- **Crash: detached thread use-after-free** — `onFileCacheUpdated()` uses `QPointer` created before `std::thread` launch; thread body wrapped in `try-catch` to prevent `std::terminate` on exception; GUI update callback checks `QPointer` before accessing members
- **UI freeze when opening external apps** — `NtApi::getTopFileCache(50)` moved to background thread (was blocking GUI thread for seconds under memory pressure)
- **Memory Map bars not showing** — reverted `getPhysicalMemoryBreakdown()` back to GUI thread (single NT API call, <1ms)
- **Memory Map progress bars not displaying colors** — restored synchronous execution for memory map updates
- **PressureHigh/PressureCritical firing together** — `else if` guard added
- **Logger callback use-after-free** — `setCallback(nullptr)` in destructor
- **ProBalance logic** — monotonic priority comparison fixed
- **getProcessStandbyMemory returning garbage** — returns 0 (API not available per-process)
- **getPhysicalMemoryBreakdown returning zero** — 512-byte buffer for Win11 `NtQuerySystemInformation`

### Changed
- Version bumped from 2.0.0 → 1.1.0 (SemVer reset)
- `Version::getFullVersion()` now reads from `Constants::APP_VERSION` (single source of truth)
- About dialog reads version from `Constants::APP_VERSION`
- MemoryCollector: removed expensive `getTopFileCache(5)` from per-second polling loop
- MainWindow default size: 1100×760
- SettingsDialog: QScrollArea wrapper, reduced fonts, 520×400 min size
- HistoryChart overlay default: "RAM Usage (GB)"

---

## [1.0.0] — 2026-05-13

### Added
- Initial release — memory monitoring and optimization tool for Windows
- Real-time dashboard with memory charts and pressure scoring
- Process manager with detailed memory metrics
- Smart Optimize and Deep Clean
- Auto-Optimize with adaptive background cleaning
- ProBalance scheduler
- Leak Hunter
- File cache and memory map analysis
- Game Mode with automatic detection
- Profiles (Economy, Balanced, Performance, Gaming)
- Scheduled cleaning and startup optimization
- System information panel
- Console log with severity filtering
- System tray with background operation
- Dark theme throughout
- WiX-based MSI installer
