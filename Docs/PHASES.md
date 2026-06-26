
# docs/PHASES.md

# RAMFlux Development Phases

> **Status v2.15.0**: Phases 1-30 concluídos.
> - Auto-tuning Engine: feedback loop que ajusta cooldown/intervalo do Scheduler e Cleaner conforme acurácia das predições (v2.7.0)
> - HardFaultPredictor 2.0: predição por regressão linear com severidade graduada (5 níveis: None/Critical), substituindo threshold fixo binário (v2.7.0)
> - Effectiveness Tracking: storePrediction/evaluatePredictionAccuracy — acurácia geral + recente, FP/FN tracking (v2.7.0)
> - Segurança crítica: auditoria completa — 11 vulnerabilidades/bugs corrigidos (v2.6.0)
> - Process Rules Engine: regras persistentes de prioridade/afinidade com QSettings (v2.6.0)
> - CPU Affinity Manager: CpuAffinityDialog com grid de checkboxes por CPU (v2.6.0)
> - Process Watchdog: monitoramento de memória com ações automáticas (v2.6.0)
> - Tema: Catppuccin Latte corrigido — surfaces mais claras, texto mais escuro, contraste 6.5:1 (v2.6.0)
> - Tema: ThemeManager com 3 temas (Mocha, Latte, Nord) + seletor no SettingsDialog (v2.5.2)
> - Segurança: code audit completo — 18 arquivos revisados (v2.5.2)
> - Bugfix: CPU% sempre zero no FluxProcessAnalyzer (v2.5.2)
> - Bugfix: coldPageRatio com page size hardcoded 4096 (v2.5.2)
> - Bugfix: IO priority nunca restaurada no FluxGameMode (v2.5.2)
> - Segurança: injeção em `rotateLog()` corrigida (v2.5.1)
> - I/O-Aware Memory Cleaning: guardas de fila de disco em todas as operações de limpeza, standby só em Critical, perfis reajustados (v2.12.0)
> - CPU Mining Mode: detecção de ~35 mineradores, boost CPU/IO/page, background throttle (v2.12.0)

---

# PHASE 1 — BASE PROJECT ✓

Goals:
- setup Qt6
- setup CMake
- create MainWindow
- create build pipeline

Deliverables:
- CMakeLists.txt
- main.cpp
- MainWindow

---

# PHASE 2 — DASHBOARD PLATFORM ✓

Goals:
- modern dashboard
- charts
- telemetry cards
- live metrics

Deliverables:
- DashboardWidget
- MemoryChartWidget
- SystemInfoCard

---

# PHASE 3 — TELEMETRY ENGINE ✓

Goals:
- RAM monitoring
- memory pressure
- telemetry snapshots

Deliverables:
- FluxTelemetry
- MemoryCollector
- MemorySnapshot

---

# PHASE 4 — PROCESS OBSERVABILITY ✓

Goals:
- process enumeration
- process analytics
- working set analysis

Deliverables:
- ProcessCache (WS aging, session aggregation)
- FluxProcessAnalyzer
- ProcessInfo

---

# PHASE 5 — AUTOMATION ENGINE ✓

Goals:
- automation
- scheduling
- cooldowns
- idle optimization

Deliverables:
- FluxScheduler (AI-driven, ProBalance)
- FluxOptimizer
- FluxCleaner (adaptive, battery-aware)

---

# PHASE 6 — SETTINGS PLATFORM ✓

Goals:
- persistent settings
- optimization profiles
- profile management

Deliverables:
- SettingsDialog (QSettings)
- ProfileManager (5 perfis)
- ProfileConfig

---

# PHASE 7 — TRAY PLATFORM ✓

Goals:
- tray integration
- notifications
- quick actions

Deliverables:
- SystemTrayManager
- Tray notifications
- Quick actions menu

---

# PHASE 8 — ADVANCED MEMORY ENGINE ✓

Goals:
- orchestration engine
- standby management
- memory awareness

Deliverables:
- FluxCleaner (adaptive, age-aware)
- NtApi (standby, modified, combined, file cache)
- HelperClient + RAMFluxHelper (elevated ops)

---

# PHASE 9 — STABILITY PLATFORM ✓

Goals:
- watchdog
- rollback
- failsafe systems
- safety validation

Deliverables:
- Crash handling (minidumps)
- Protected process filtering
- Cooldown enforcement
- Safety validation

---

# PHASE 10 — AI HEURISTICS ✓

Goals:
- adaptive heuristics
- workload fingerprinting
- scoring
- contextual optimization

Deliverables:
- HeuristicEngine
- WorkloadClassifier (7 tipos)
- PressurePredictor (30/60/120s)

---

# PHASE 11 — DESIGN SYSTEM ✓

Goals:
- Fluent UI
- animations
- themes
- premium UX

Deliverables:
- ThemeManager (3 temas: Catppuccin Mocha, Catppuccin Latte, Nord) + seletor no SettingsDialog + persistência QSettings
- MemoryCard (Fluent-style cards)
- Memory Map visual breakdown

---

# PHASE 12 — REAL NTAPI OPERATIONS ✓

Goals:
- real memory optimization
- working set trimming
- standby purge
- NTAPI integration

Deliverables:
- NtApi (standby, modified, combined, file cache, trim)
- Privilege management (SE_LOCK_MEMORY, SE_DEBUG, SE_BACKUP)
- RAMFluxHelper elevated process
- Compression, NUMA, page priority, disk queue, power status

---

# PHASE 13 — DISTRIBUTION PLATFORM (parcial)

Goals:
- installer
- portable mode
- release pipeline

Deliverables:
- WiX MSI installer (v7)
- GitHub Releases (MSI asset)
- windeployqt deployment
- Pendente: portable mode, signed installer

---

# PHASE 14 — BENCHMARK SYSTEM ✓

Goals:
- scientific benchmarking infrastructure
- multi-phase measurement methodology
- automated report generation
- CLI integration

Deliverables:
- BenchmarkRunner (5-phase methodology: Warmup, Baseline, Pressure, Optimization, Post-Opt)
- BenchmarkTypes (config, samples, stats, reports)
- tri-format reports (CSV, JSON, Markdown)
- CLI flags: --benchmark, --bench-no-pressure, --bench-output

---

# PHASE 15 — PROCESS RULES ENGINE ✓

Goals:
- persistent process rules
- priority/affinity automation
- rule matching with wildcards
- QSettings persistence

Deliverables:
- ProcessRulesEngine (IModule)
- RuleTypes (enums, structs)
- ProcessRule (persistent, watchdog, wildcards)
- Persistência via QSettings (beginWriteArray/endArray)

---

# PHASE 16 — CPU AFFINITY MANAGER ✓

Goals:
- GUI for CPU affinity management
- per-CPU selection
- Select All / Clear All

Deliverables:
- CpuAffinityDialog (checkboxes por CPU lógica)
- NtApi::setProcessAffinity / getProcessAffinity
- NtApi::getSystemCpuCount

---

# PHASE 17 — PROCESS WATCHDOG ✓

Goals:
- automatic process monitoring
- memory threshold triggers
- configurable actions

Deliverables:
- Watchdog rules (RuleType::WatchdogMemory)
- RuleAction: Terminate, Restart, SetCpuPriority, SetIoPriority, SetCpuAffinity, Log
- Watchdog restart via QueryFullProcessImageNameW + CreateProcessW
- Polling a cada 5s na watchdogLoop

---

# PHASE 18 — AUTO-TUNING & HARD FAULT PREDICTOR 2.0 ✓

Goals:
- feedback loop de auto-tuning baseado em acurácia das predições
- HardFaultPredictor com regressão linear e severidade graduada
- effectiveness tracking (storePrediction + evaluatePredictionAccuracy)
- integração com Scheduler (intervalo) e Cleaner (cooldown)

Deliverables:
- HeuristicEngine::storePrediction / evaluatePredictionAccuracy / tuneFromMetrics / adjustModuleParams
- EffectivenessSample / EffectivenessMetrics / TuningParams structs
- HardFaultPredictor classe completa (feed, computeSlope, evaluate, reset)
- HardFaultSeverity (None/Critical) + HardFaultPrediction com severityScore 0-100
- Integração via ModuleManager: ajusta FluxScheduler::setIntervalMs e FluxCleaner::setCooldownMs

---

# PHASE 19 — NTAPI/KERNEL EXPANSION ✓

Goals:
- NUMA node awareness com per-process node query e migração automática
- Page priority / Superfetch (SysMain) integration
- Memory compression tuning com detecção de ineficiência
- Hard fault prediction direto via NTAPI (quick-check síncrono)
- Working set aging com trim de processos ociosos

Deliverables:
- FluxNTAPI: `getProcessNumaNode`, `setProcessNumaAffinityByNode`, `getPreferredNumaForProcess`
- FluxScheduler::applyNumaOptimization (redistribui processos entre nós quando imbalance >20%)
- FluxNTAPI: `getSysMainServiceState`, `setSysMainServiceEnabled`
- FluxNTAPI: `getCompressionEfficiency`, `getCompressionStoreInfo`
- FluxScheduler::applyCompressionTuning (alerta após 3 amostras ruins consecutivas)
- FluxNTAPI: `getHardFaultPrediction` (score 0-100: faults + disk queue + standby + trend)
- FluxNTAPI: `getProcessWsAge`, `trimIdleProcesses`
- FluxScheduler::applyWsAgingTrim (trim de processos >100MB inativos >60s)
- PreemptiveConfidence: AI boost contínuo no `calculatePressureScore()` substitui trigger binário

---

# PHASE 20 — GAME MODE 3.0 ✓

Goals:
- DirectX video memory integration (DXGI)
- Game-specific profiles com VRAM/RAM requirements
- Pre-game memory preparation
- VRAM-aware cleaning during gameplay
- Competitive mode with timer resolution optimization

Deliverables:
- FluxNTAPI: `getVideoMemoryInfo`, `getGameMemoryPressure`, `setTimerResolution`
- FluxGameMode: 11 game profiles (CS2, Valorant, Fortnite, CoD, etc.)
- FluxGameMode::applyVramMonitor (VRAM tracking every ~4s during gameplay)
- FluxCleaner::prepareForGame (standby + modified + idle process trim)
- Competitive mode: 1ms timer, ProBalance/WS trim suppression

---

# PHASE 21 — PAGE FILE AUTO-TUNING ✓

Goals:
- Query current page file configuration and usage via NTAPI
- Optimal page file size recommendation based on RAM + commit charge
- Automatic resize via NtSetSystemInformation with SE_CREATE_PAGEFILE_NAME
- Scheduler integration for pressure monitoring and proactive resizing

Deliverables:
- FluxNTAPI: `PageFileInfo` struct (path, currentSize, minSize, maxSize, usage, peak)
- FluxNTAPI: `PageFileRecommendation` struct (recommendedMinMB, MaxMB, pressure, shouldResize)
- FluxNTAPI: `getPageFileInfo()` — NtQuerySystemInformation(SystemPageFileInformation)
- FluxNTAPI: `getPageFileRecommendation()` — RAM * 1/4 min, RAM * 3 max, headroom from peak
- FluxNTAPI: `setPageFileSize()` — NtSetSystemInformation with privilege enable/disable
- FluxScheduler: `applyPageFileTuning()` — 2min check, clean trigger at 80%, resize at 90%
- FluxScheduler: `setPageFileAutoTuningEnabled()` / `isPageFileAutoTuningEnabled()`
- Constants.h: PF_TUNING_INTERVAL_MS, PF_RESIZE_PRESSURE_THRESHOLD, PF_CLEAN_PRESSURE_THRESHOLD, etc.
- Suppressed during battery boost
---

# PHASE 22 — MULTI-LEVEL CACHE PRESSURE ✓

Goals:
- Detect CPU cache topology (L1/L2/L3 sizes) via GetLogicalProcessorInformationEx
- Estimate per-level cache pressure from active process working sets
- Integrate cache pressure into the optimization pressure score
- Flag L3 contention for cache-aware resource management

Deliverables:
- FluxNTAPI: `CacheTopology` struct (L1/L2/L3 sizes, associativity, line size, cores, sockets)
- FluxNTAPI: `CachePressureInfo` struct (per-level pressure, overall score, contention flag)
- FluxNTAPI: `getCacheTopology()` — RelationCache + RelationProcessorCore + RelationProcessorPackage
- FluxNTAPI: `getCachePressure()` — WS/cache ratios per core/socket level
- FluxOptimizer: cache pressure weight (5%) + L3 contention bonus (10pt) in calculatePressureScore()
- Constants.h: CP_CACHE_SCORE_WEIGHT, CP_L3_CONTENDED_BONUS, CP_L3/L2/L1_WEIGHT
- Topology cached globally after first call

---

# PHASE 23 — ADVANCED MEMORY COMPRESSION ✓

Goals:
- StoreAPI decoder pool query and management
- Compression mode tuning (Auto/MaxCompression/MaxPerformance)
- Integration with scheduler for adaptive compression strategy
- Game-aware decoder pool expansion

Deliverables:
- FluxNTAPI: `CompressionStoreMode` enum (Auto=0, MaxCompression=1, MaxPerformance=2)
- FluxNTAPI: `StoreDecoderPoolInfo` struct (currentPages, maxPages, allocated/free, hitRate)
- FluxNTAPI: `AdvancedCompressionInfo` struct (mode, ratio, decoder pool stats, harmful flag)
- FluxNTAPI: `getStoreDecoderPoolInfo()` — extended SystemMemoryCompressionInformation query
- FluxNTAPI: `setStoreDecoderPoolSize(maxPages)` — resize decoder pool via NtSetSystemInformation
- FluxNTAPI: `setCompressionStoreMode(mode)` — switch compression behavior
- FluxNTAPI: `getAdvancedCompressionInfo()` — aggregate all compression/decoder metrics
- FluxScheduler: `applyAdvancedCompressionTuning()` — 60s interval:
  - Ratio <1.0 harmful → MaxPerformance mode
  - Gaming + hit rate <50% → expand decoder pool +256 pages
  - Healthy ratio >2.0 + pool >80% → log status
- FluxOptimizer: +5ps penalty when decoder hit rate <30% and compressed >1GB
- Constants: AC_POOL_LOW_HIT_RATE, AC_HARMFUL_RATIO_THRESHOLD, AC_DECODER_POOL_EXPAND_STEP, et al.

---

# PHASE 24 — PROCESS MEMORY CLASSIFIER ✓

Goals:
- Classify processes by memory usage pattern (steady, burst, periodic, leaky)
- Track per-process WS history over time
- Apply different strategies per classification profile
- Reduce unnecessary trimming of steady processes

Deliverables:
- `ProcessMemoryProfile` enum: Unknown=0, Steady, Burst, Periodic, Leaky
- `ProcessClassification` struct: pid, name, profile, confidence, mean/stddev WS, growth rate, peak/mean, recommendation
- `ClassifierConfig` struct: maxSamples, leakyGrowthThreshold, burstPeakRatio, periodicCycles, etc.
- `FluxClassifier` class (IModule): per-process deque tracking, classifyPattern(), classifyAll()
  - Steady: CV <0.15, peak/mean <1.3 — no action needed
  - Burst: peak/mean >2.0, CV >0.5 — trim after peak
  - Periodic: ≥3 zero-crossings, 0.2 < CV <0.8 — track cycle, prepare trim at peak
  - Leaky: growth >50MB/min, ≥20 samples — trim periodically
- `FluxScheduler::applyProcessClassification()` — 30s interval:
  - Feeds ProcessCache entries (WS >10MB) into FluxClassifier::recordSample()
  - Runs classifyAll(), trims leaky/burst procs with WS >500MB
  - Logs tracked count, trimmed count, monitored count
- Module registered in main.cpp, added to CMakeLists.txt
- Constants.h: CL_MIN_WS_TRACK_BYTES, CL_LEAKY_GROWTH_THRESHOLD_MBPM, CL_BURST_PEAK_MEAN_RATIO, et al.

---

# PHASE 25 — COMPETITIVE FEATURES (v2.12.0) ✓

A six-feature expansion to surpass Process Lasso and competing optimizers:

**Feature A — CPU Limiting Inteligente:**
- `CpuLimiter` class: Windows Job Objects with hard cap
- Only limits background processes when system CPU >70%
- Configurable cap percentage (default 50%)
- Automatically removes limits when CPU normalizes

**Feature B — Power Plan Automation:**
- Dynamic powrprof.dll loading (no static link)
- Switches plans: Gaming→High Performance, Battery Idle→Power Saver
- Integrates with workload classifier

**Feature C — Suspensão de Processos por IA:**
- `ProcessSuspender` class via NtSuspendProcess/NtResumeProcess (ntdll.dll)
- Suspends processes >1GB WS inactive >120s under critical pressure
- Auto-resumes after pressure subsides or on shutdown

**Feature D — QoS de Rede:**
- `NetworkQoS` class + NTAPI network monitoring
- Connection counting via GetExtendedTcpTable/GetExtendedUdpTable (iphlpapi.dll)
- DSCP priority tagging via SetProcessInformation(ProcessNetQoSPolicy)
- Game process gets NET_PRIORITY_HIGH (DSCP 46), heavy network processes get LOW/VERY_LOW

**Feature E — Responsiveness Slider:**
- `ResponsivenessSlider` class: unified control (0-10) for all subsystems
- Level 0 (Max Performance) through Level 10 (Max Responsiveness)
- Each level maps CPU cap, suspend threshold, QoS, and power plan
- `applyResponsiveness()` runs at scheduler loop top, configuring all subsystems

**Feature F — Dashboard de I/O:**
- `IoMonitor` class: per-process I/O rates via GetProcessIoCounters
- System-wide I/O via NtQuerySystemInformation(SystemIoInformation)
- `IoDashboardWidget`: new "I/O" tab with top readers/writers tables
- 3s auto-refresh with formatted rates (B/s, KB/s, MB/s, GB/s)

**Code Quality:**
- `verifyProcessName()` extraído para `shared/ProcessUtils.h` (inline, elimina duplicação em 4 módulos)
- EventBus modernizado: `std::thread` → `std::jthread` com `std::stop_token`
- Versão bumpada para 2.12.0

---

# PHASE 26 — CPU MINING OPTIMIZATION MODE ✓

A specialized mode for optimizing system performance during CPU-based cryptocurrency mining.

**Detection:**
- `KNOWN_MINERS` array (~35 executáveis conhecidos como XMRig, lolMiner, TeamRedMiner, etc.)
- `hasKnownMiner()` no WorkloadClassifier — detectado antes de Gaming (mineração tem prioridade)
- Detection loop a cada 2s no FluxMiningMode

**Mining Profile (ProfileManager):**
- `cooldownMs=60000`, `autoCleanEnabled=true`, `aggressiveTrim=false`
- `gameMode=false`, `leakDetection=false`, `pollingIntervalMs=15000`
- `pressureThreshold=60`, `standbyThresholdMB=4096`, `freeMemThresholdMB=1024`

**FluxMiningMode (src/mining/):**
- Boost de prioridade do minerador: CPU HIGH, IO HIGH, page NORMAL
- Throttling de processos background (>100MB WS): prioridade BELOW_NORMAL, page VERY_LOW/LOW, IO 0
- Timer resolution 1ms via NtApi::setTimerResolution
- Restauração completa de todos os estados na saída

**Integração no FluxScheduler:**
- ProBalance: ignora minerPid na classificação de processos
- Power Plan: mining ativo → High Performance
- WS trim (applyWsAgingTrim): pula completamente se mining ativo
- ProcessSuspender: minerPid adicionado como exempt (nunca suspenso)
- NetworkQoS: minerPid recebe prioridade HIGH se mining ativo
- Page priority orchestration: pula minerPid
- NUMA optimization: pula minerPid
- Process classification: pula minerPid

**UI (SettingsDialog):**
- Checkbox "Enable Mining Mode (CPU miner optimization)"
- Item "Mining - CPU miner optimization" no profileCombo (index 4, Custom=5)
- Save/load do estado miningMode via QSettings

**Constants:**
- `WorkloadType::Mining`, `EventType::MiningDetected/MiningEnded`, `ProfileType::Mining`
- `MM_DETECTION_INTERVAL_MS=2000`, `MM_BG_PROCESS_THRESHOLD_BYTES=104857600`
- `MM_MINER_CPU_PRIORITY=0x80` (HIGH), `MM_MINER_IO_PRIORITY=2` (HIGH)
- `MM_MINER_PAGE_PRIORITY=5` (NORMAL), `MM_TIMER_RESOLUTION_MS=1`

---

# PHASE 27 — I/O-AWARE MEMORY CLEANING ✓

A systematic rework to prevent disk thrashing caused by aggressive memory cache cleanup.

**Problem:**
- The standby list IS Windows' disk cache; flushing it forces every subsequent file read to hit physical disk
- Aggressive `cleanStandbyList()` + `cleanFileCache()` caused high disk queue, making the system feel slower despite "freeing" RAM
- High memory usage is often preferable to high disk I/O (RAM is 1000x faster than disk)

**I/O-Aware Guards (FluxCleaner):**
- `cleanStandbyList()`: skips if disk queue > 1.5 OR standby < 2GB (prevents thrashing small caches)
- `cleanModifiedPageList()`: skips if disk queue > 1.5 OR modified < 1GB
- `cleanFileCache()`: skips if disk queue > 0.75 (most destructive operation, most sensitive guard)
- `deepClean()`: if disk busy, falls back to cold page trim only (near-zero I/O)
- `quickClean()`: skips standby if disk busy, does process trim only

**Pressure Escalation Rework (adaptiveClean):**
- Critical pressure → `deepClean()` (standby + modified + file cache + trim)
- High pressure → `quickClean()` (standby + trim, with I/O guards)
- Normal pressure + low free mem → `quickClean()`
- High hard faults + large standby → `cleanStandbyList()` (the one case where cleaning helps I/O)

**Conservative Profile Tuning (ProfileManager):**
- Economy: cooldown 60→90s, standbyThreshold 2→4GB, freeMemThreshold 1GB→512MB, pressureThreshold 60→65
- Balanced: cooldown 30→45s, standbyThreshold 1→2GB, freeMemThreshold 2→1GB, pressureThreshold 50→55
- Performance: cooldown 15→30s, standbyThreshold 512MB→2GB, freeMemThreshold 3→2GB, pressureThreshold 35→45, aggressiveTrim disabled
- Mining: cooldown 60→90s, standbyThreshold 4→8GB, freeMemThreshold 1GB→512MB, pressureThreshold 60→65

**Idle Time Sensitivity:**
- `WS_AGE_IDLE_SECONDS`: 30→60s (processes must be idle longer before WS trim)
- `IO_TRIM_IDLE_SECONDS`: 120s for background/cleaning-related trims
- Prevents unnecessary page-out/page-in cycles from aggressive working set trimming

**New Constants:**
- `IO_DISK_QUEUE_SKIP_THRESHOLD=1.5`, `IO_STANDBY_MIN_CLEAN_BYTES=2GB`, `IO_MODIFIED_MIN_CLEAN_BYTES=1GB`
- `IO_TRIM_IDLE_SECONDS=120`, `IO_FILE_CACHE_HIGH_FAULT_THRESHOLD=150`, `IO_CLEAN_COOLDOWN_AFTER_WRITE_MS=45000`

---

# PHASE 28 — CRASH DEBUG & STABILITY FIX (v2.14.0)

**Bug:** SettingsDialog crash ao clicar Save — `strlen(NULL)` em `ucrtbase.dll` chamado de `ProfileManager::setProfile()`.

**Debug methodology:**
- Minidump analysis via `dbgeng.h` + custom parser (`C:\RAMFlux\RAMFlux_crash.dmp`)
- Identified exception: `0xC0000005` (ACCESS_VIOLATION) reading address `0x0`
- Crash at `ucrtbase.dll!strlen+0x31` — `mov rdx,[rax]` with `RAX=0` → `RCX=0` (NULL string argument)
- Return address traced via stack: `RAMFlux.exe+0x71216` → `ProfileManager::setProfile+0x96`
- `addr2line` + `nm -C` confirmou função exata
- Disassembly (`objdump -d`) revelou: `mov (%rax,%rdx,8),%r13` carregando `ProfileNames[rdx]` onde `rdx=sign-extend(profile)`
- `ProfileNames` array localizado em `.rdata` via `readelf`; 8 bytes antes do array são zero (NULL ptr)
- Causa raiz: `currentIndex()` retornando -1 → `static_cast<ProfileType>(-1)` → `ProfileNames[-1]` → `NULL` → `strlen(NULL)`

**Fix aplicado em 3 arquivos:**
1. `src/profiles/ProfileManager.cpp:22` — bounds check em `setProfile()`: `pidx<0||pidx>5` → default p/ Balanced (1)
2. `src/ui/SettingsDialog.cpp:257` — bounds check em `saveSettings()` + handler aceita idx=5 (Custom estava excluído)
3. `src/ui/MainWindow.cpp:775,335-340` — bounds check em `onProfileChanged()` + item "Mining" adicionado ao Dashboard combo (estava faltando, causando mismatch com sistema de 6 perfis)

**Build:** MinGW 13.1.0, Qt 6.8.0, deployed to `C:\RAMFlux`

---

# PHASE 29 — AUDIT FIXES & GENTLE OPERATIONS (v2.14.1)

A critical bugfix and feature release focused on eliminating hidden bugs and adding gentle, battery-aware operations.

**6 Bugs Found & Fixed in Final Audit:**

*CRITICAL (2x):*
- `RAMFluxHelper.cpp:33` — `GetLastError()` after `LocalFree(sd)` in `ensureScheduledTask()` — saved error code before freeing
- `RAMFluxHelper.cpp:306` — same bug in singleton mutex at `WinMain` — saved `singletonErr` before `LocalFree(mutexSd)`

*HIGH (2x):*
- `HelperClient.cpp:20` — `CreateFileW` without `FILE_FLAG_OVERLAPPED` (synchronous pipe handle used with Overlapped I/O → infinite hang)
- `FluxNTAPI.cpp:95` — `disablePrivilege` used `0x2` (SE_PRIVILEGE_ENABLED) instead of `0x0` (disabled) — privileges never actually disabled

*MEDIUM (2x):*
- `RAMFluxHelper.cpp:255` — `readOk` unconditionally TRUE after `GetOverlappedResult` failure
- `FluxNTAPI.cpp:697-700` — `predictFuture()` data race on `samples` + deadlock from nested lock — refactored with `trendSlopeLocked()` unlocked helper

**3 New Features:**

*EcoQoS (Efficiency Mode):*
- `NtApi::setProcessEfficiencyMode(pid)` uses `SetProcessInformation(ProcessPowerThrottling)` (Win 10 1809+)
- Puts background processes in CPU efficiency mode — reduces power/heat on notebooks
- Integrated into FluxScheduler (30s loop, targets idle background WS >100MB)

*Gentle Standby Clean:*
- `NtApi::gentleStandbyClean()` — splits processes by WS age (active vs idle), elevates idle page priority
- Cleans standby in 3 chunks with `Sleep(500ms)` + disk queue guard (`<1.5`) between chunks
- `FluxCleaner::gentleStandbyClean()` wrapper for scheduler access

*Adaptive Standby Orchestration (3-tier):*
- Replaces single-threshold logic in `FluxScheduler::applyStandbyOrchestration()`
- Tier 1: HF critical + high pressure + standby >256MB → gentle clean
- Tier 2: HF critical + standby >1GB → selective clean (WS trim idle >120s)
- Tier 3: Standby >2GB preventive → gentle clean
- All tiers gated by disk queue <1.5

**Fixes:**
- `selectiveStandbyClean()` rewritten — now uses per-process WS trim for idle processes instead of page priorities alone (page priorities don't filter `NtSetSystemInformation(MemoryListStandby)` which clears ALL standby pages)

**Build:** MinGW 13.1.0, Qt 6.11.0, deployed to `C:\RAMFlux`

---

# PHASE 30 — ML ENGINE & I/O COST TRACKER (v2.15.0)

A feature release introducing machine learning prediction and I/O cost measurement.

### ML Engine — MLEngine

**`src/ai/MLEngine.h/.cpp`** — Multi-variable linear regression with online SGD:
- 10 features: hard faults/sec, 5-avg, slope, disk queue + slope, standby GB + slope, memory pressure, total WS, time since last clean
- Running z-score normalization (α=1e-4 for mean/std)
- SGD learning rate: 0.01
- Prediction horizon: 30s, auto-trained via T vs T+30s comparison
- Prediction clamped to [0, 100], confidence based on sample count
- Thread-safe via `std::mutex`

**Integration:**
- Owned by `HeuristicEngine` as `m_mlEngine`
- `extractFeatures()` + `predict()` + `processTraining()` called every `evaluateAndPost()` cycle
- `MLPrediction` stored in `HeuristicReport.mlScore/mlConfidence/mlSampleCount`
- Subscribes to `CleaningFinished` for `m_lastCleanTime` tracking

### I/O Cost Tracker — IoCostTracker

**`src/ai/IoCostTracker.h/.cpp`** — Per-process page fault attribution after cleaning:
- `beforeClean()` snapshots all processes' cumulative page faults
- After 30s, `evaluateCosts()` computes delta per process
- Cost score via EMA (α=0.3): `newCost = min(100, delta/100)`
- `ProcessIoCost` per PID with 0-100 score
- `IoCostReport.systemIoCost` = mean of all process scores

**Integration:**
- Owned by `HeuristicEngine` as `m_ioCostTracker`
- Subscribes to `CleaningStarted`/`CleaningFinished` events
- `FluxScheduler::applyStandbyOrchestration()` skips non-critical clean when `systemIoCost ≥ 50`

### Files Modified
| File | Change |
|------|--------|
| `src/ai/MLEngine.h` | New — ML engine class + data structs |
| `src/ai/MLEngine.cpp` | New — SGD, feature extraction, training |
| `src/ai/IoCostTracker.h` | New — I/O cost tracker |
| `src/ai/IoCostTracker.cpp` | New — fault delta measurement + scoring |
| `src/ai/HeuristicEngine.h` | Added MLEngine + IoCostTracker members + report fields |
| `src/ai/HeuristicEngine.cpp` | MLEngine predict/train + IoCostTracker integration |
| `src/scheduler/FluxScheduler.cpp` | I/O cost gate in standby orchestration |
| `CMakeLists.txt` | Added new .cpp files to AI_SOURCES |

**Build:** MinGW 13.1.0, Qt 6.11.0, deployed to `C:\RAMFlux`

---

# FINAL TARGET

RAMFlux should become:

- lightweight
- intelligent
- modern
- stable
- modular
- production-grade
- enterprise-inspired

Windows memory orchestration platform.