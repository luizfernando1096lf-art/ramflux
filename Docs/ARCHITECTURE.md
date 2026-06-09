# RAMFlux Architecture

---

## High-Level Architecture

```
Telemetry
    ↓
Analytics
    ↓
Heuristics
    ↓
Safety Validation
    ↓
Optimization Orchestration
    ↓
Memory Operations
    ↓
Validation
    ↓
Cooldown
```

## Architecture Principles

The architecture must be:
- modular
- low-overhead
- production-grade
- fault-tolerant
- extensible
- observable

## Module Responsibilities

### `src/core`
Core application infrastructure.

**Responsibilities:** app bootstrap, initialization, lifecycle management, service registration

### `src/ui`
User interface layer.

**Responsibilities:** dashboard, charts, process views, animations, themes, user interaction

**Key Components:** MainWindow, SettingsDialog, DashboardWidget, ConsoleWidget, ThemeManager (3 themes — Mocha/Latte/Nord, QSS generation, runtime re-styling via `applyTo()`)

**Rules:**
- never call NTAPI directly
- no heavy processing
- UI thread safety required

### `src/telemetry`
System telemetry and metrics.

**Responsibilities:** RAM metrics, memory pressure, system statistics, snapshots, historical metrics

### `src/process`
Process observability layer.

**Responsibilities:** enumerate processes, collect process memory usage, process analytics, working set analysis

### `src/benchmark`
Scientific benchmarking infrastructure.

**Responsibilities:** 5-phase benchmark (Warmup, Baseline, Pressure, Optimization, Post-Opt), statistical analysis (mean, median, stddev, percentiles), tri-format report generation (CSV/JSON/Markdown), CLI-driven one-shot execution

**Rules:**
- one-shot only, never continuous
- telemetry wait-loop ensures valid first sample
- independent of IModule system

### `src/automation`
Automation and scheduling.

**Responsibilities:** timers, optimization scheduling, cooldowns, automation rules

**Rules:**
- avoid aggressive polling
- low wakeup rate only

### `src/memory`
Memory orchestration layer.

**Responsibilities:** working set trim, standby list purge, optimization orchestration, compression awareness

**Rules:**
- safety-first
- heuristic-driven only

### `src/stability`
Stability and protection platform.

**Responsibilities:** watchdogs, rollback, validation, protected process filtering, failsafe systems

### `src/ai`
Adaptive heuristics layer.

**Responsibilities:** workload fingerprinting, optimization scoring, behavior analysis, prediction systems, hard fault prediction, auto-tuning feedback loop, effectiveness tracking

**Key Components (v2.7.0):** `HeuristicEngine` with auto-tuning (`storePrediction`/`evaluatePredictionAccuracy`/`tuneFromMetrics`/`adjustModuleParams`), `HardFaultPredictor` class (60-sample regression), `EffectivenessMetrics`, `TuningParams`. AI predictions consumed by `FluxOptimizer::calculatePressureScore()` as continuous boost.

**Rules:**
- lightweight heuristics only
- no heavy AI inference
- predictions influence Optimizer, not Scheduler directly (PreemptiveConfidence)

### `src/ntapi`
Windows NTAPI abstraction layer.

**Responsibilities:** NTAPI wrappers, privilege management, Windows version compatibility, low-level memory operations, NUMA queries (per-process node + migration), page priority (via NtSetInformationProcess), disk performance, SysMain/Superfetch service control, hard fault prediction (quick-check), compression efficiency/store info, working set aging, idle process trimming

**Key Functions (v2.7.0 expansion):** `getProcessNumaNode`, `setProcessNumaAffinityByNode`, `getPreferredNumaForProcess`, `getSysMainServiceState`, `setSysMainServiceEnabled`, `getHardFaultPrediction`, `getCompressionEfficiency`, `getCompressionStoreInfo`, `getProcessWsAge`, `trimIdleProcesses`, `getProcessTotalFaults`

**Rules:**
- isolate NTAPI here only
- use safe wrappers
- validate all operations

### `src/cleaner`
Memory cleaning orchestration.

**Responsibilities:** standby list, modified page list, working set trim (age-aware), file cache, combined list, defragmentation

**Rules:**
- safety-first
- respect Working Set Aging (skip active processes)
- cooldown enforcement

### `src/leakhunter`
Memory leak detection platform.

**Responsibilities:** per-process memory growth tracking, leak alerts, anomaly detection

### `src/gamemode`
Game Mode 3.0 — DirectX-integrated game detection and optimization.

**Responsibilities:** foreground window + known-process game detection, DXGI video memory monitoring (VRAM usage/budget/pressure), game-specific profiles (11 perfis integrados, CPU/IO/page priority por jogo), pre-game memory preparation (`FluxCleaner::prepareForGame()`), VRAM-aware cleaning during gameplay (standby clean if VRAM >85% or throttling), competitive mode (1ms timer resolution, full interference suppression), background process throttling (page/CPU/IO priority reduction), state restoration on game exit

**Key Components (v3.0):** `FluxGameMode` (IModule), `GameProfile` struct, `VramSample` deque (30 samples), `applyVramMonitor()`, `setCompetitiveModeEnabled()`, `setVramAwareCleaningEnabled()`

**Rules:**
- detection loop at 2s interval
- VRAM monitor at ~4s during active gameplay only
- pre-game memory preparation runs once per game session
- competitive mode suppresses ProBalance, WS aging trim, and compression tuning
- all optimizations reverted on game exit or module shutdown

### `src/helper`
Privileged helper process communication.

**Responsibilities:** IPC with RAMFluxHelper.exe, command dispatch, admin-level operations

### `src/analyzer`
Process analysis engine.

**Responsibilities:** CPU usage tracking, process behavior analysis, historical metrics

### `src/scheduler`
Optimization scheduling.

**Responsibilities:** timed optimization triggers, cooldown management, preemptive AI-driven cleaning

### `src/profiles`
Profile management system.

**Responsibilities:** profiles (Economy/Balanced/Performance/Gaming/Custom), settings persistence, user preferences, profile switching, per-profile configuration

### `src/rules`
Process rules engine and watchdog.

**Responsibilities:** persistent process rules (CPU/IO/page priority, CPU affinity, memory priority), wildcard matching, watchdog loop (5s polling), automatic actions (terminate/restart/log/priorities)

**Key Components:** ProcessRulesEngine (IRulesEngine IModule), ProcessRule struct, RuleAction enum, wildcard matching (`*` prefix/suffix, case-insensitive)

## Threading Model

**Main thread:** UI only

**Worker threads:** telemetry, analytics, safe optimization tasks, EventBus dispatch

**Never:** block UI thread, run heavy NTAPI calls in UI thread

## Memory Optimization Flow (v2.7.0)

```
Telemetry (incl. NUMA, Disk Queue, Page Faults)
↓
Pressure Analysis (compression ratio, fault trend, AI boost)
↓
AI Heuristics (workload, anomaly, hard fault prediction)
↓
PreemptiveConfidence (continuous AI boost in pressure score)
↓
Working Set Aging Check (skip active processes <30s)
↓
NUMA Optimization (redistribute processes across nodes if imbalance >20%)
↓
SysMain/Superfetch Check (adjust page priority strategy)
↓
Compression Tuning (warn if ratio <1.2x for 3+ consecutive samples)
↓
WS Aging Trim (trim processes >100MB idle >60s — configurable)
↓
Safety Validation (protected processes, game mode)
↓
Optimization Decision
↓
Working Set Trim (idle processes only)
↓
Standby Purge
↓
Hard Fault Check (disk queue + faults correlation)
↓
VRAM Check (during games: standby clean if VRAM >85%/throttling)
↓
Validation
↓
Cooldown
```

## NUMA Awareness Flow (v2.7.0)

```
Telemetry
↓
GetNumaHighestNodeNumber
↓
For each node: GetNumaAvailableMemoryNodeEx
↓
Display per-node available memory in System Info
↓
Per-process: GetProcessInformation(ProcessProcessorNumber) → node
↓
If NUMA optimization enabled & imbalance >20%:
  Move processes with WS >512MB from most-loaded to least-loaded node
  Via SetProcessGroupAffinity
↓
Per-node standby list optimization
```

## Working Set Aging (v2.7.0)

Each ProcessEntry tracks `lastWsChangeTime`. On each ProcessCache::update(), if the working set delta exceeds 5 MB (`WS_AGE_ACTIVITY_THRESHOLD_BYTES`), the timestamp is refreshed. `FluxCleaner::trimProcesses()` skips any process whose last activity was less than 30 seconds ago.

**NTAPI-level WS Aging (v2.7.0):**
- `getProcessWsAge(pid)` returns seconds since last significant WS change
- `trimIdleProcesses(thresholdBytes, idleSeconds)` trims only processes with WS > threshold AND idle for N+ seconds
- `FluxScheduler::applyWsAgingTrim()` runs every 30s, configurable via `setWsAgingIdleSeconds()` (default 60) and `setWsAgingThresholdBytes()` (default 100MB)

## Hard Fault Prediction (v2.7.0)

The `HardFaultPredictor` (classe completa, anteriormente struct simples) mantém um deque de 60 amostras coleadas a cada 2s de três sinais:
- **Hard faults/s** (from page file delta / 4096)
- **Disk queue length** (via IOCTL_DISK_PERFORMANCE)
- **Standby cache GB** (available pages to repurpose)

Regressão linear calcula slopes individuais para cada sinal via `computeSlope()`, usando ponteiro-para-membro genérico. O método `evaluate()` computa um `severityScore` (0–100) ponderado:
| Fator | Peso máximo | Faixas |
|-------|-------------|--------|
| Faults/s atuais | 40 pts | 50→10, 100→20, 200→30, 500→40 |
| Trend de faults | 25 pts | >1→5, >2→10, >5→18, >10→25 |
| Disk queue | 20 pts | ≥1→7, ≥2→13, ≥3→20 |
| Standby baixo | 15 pts | <2GB→5, <1GB→10, <0.5GB→15 |

5 níveis: **None** (<15), **Low** (≥15), **Medium** (≥30), **High** (≥50), **Critical** (≥70). Storm warning (`stormWarning`) só dispara quando severo + rising + (low cache ou high faults), com cooldown de 30s.

## Auto-tuning (v2.7.0)

The HeuristicEngine now implements a feedback loop that adjusts module parameters based on prediction accuracy:
1. `storePrediction()` — salva predições de pressão a 30/60/120s com timestamp
2. `evaluatePredictionAccuracy()` — quando o horizonte expira, compara predito vs real (tolerância 15% ou 10pts), atualiza `EffectivenessMetrics` (accuracy, recentAccuracy, FP/FN)
3. `tuneFromMetrics()` — mapeia acurácia em 4 faixas (≥80% agressivo, <45% conservador), ajusta confidence threshold por FP/FN, considera trend de pressão e anomalias
4. `adjustModuleParams()` — a cada 30s, aplica `FluxCleaner::setCooldownMs()` e `FluxScheduler::setIntervalMs()` com logging do ajuste

## PreemptiveConfidence (v2.7.0)

The binary heuristic trigger previously in `FluxScheduler::schedulerLoop()` (which overrode `shouldClean = true` when AI predicted high pressure) has been replaced by a continuous AI boost integrated into `FluxOptimizer::calculatePressureScore()`:

```
pressure += clamp(predictedPressure60s * predictionConfidence * 0.35, 0.0, 25.0)
```

This means the AI prediction now naturally elevates the pressure score across all thresholds, causing the optimizer to trigger cleaning earlier when AI confidence is high, without an abrupt override. The weight (0.35) and cap (25pts) ensure the AI can contribute meaningfully (up to 1/4 of the total score) but cannot dominate the decision.

## Memory Compression Tuning (v2.7.0)

`NtApi::getCompressionEfficiency()` computes:
- **ratio**: uncompressed data ÷ compressed memory (how many times compression is working)
- **harmful**: true when ratio < 1.2x (compression overhead negates benefit)
- **savingsPercent**: actual memory saved (e.g. 2.0 ratio = 50% savings)

`FluxScheduler::applyCompressionTuning()` runs every 30s, counting consecutive harmful samples. After 3+ consecutive, logs a persistent warning suggesting the user disable Memory Compression. When efficiency recovers, restores the "healthy" state.

## Hard Fault Prediction — NTAPI Quick-Check (v2.7.0)

Alongside the AI-level `HardFaultPredictor` (60-sample linear regression), a lightweight synchronous `getHardFaultPrediction()` provides a quick-check score:
- Aggregates page faults across all processes + standby list + disk queue
- Score: faults 40pts + disk queue 25pts + standby 20pts + trend 15pts
- Returns `warning` flag when score ≥ 50

This is used by other modules that need an instant fault risk assessment without waiting for the HeuristicEngine's analysis loop.

## Advanced Memory Compression (v2.10.0)

The advanced compression subsystem manages the StoreAPI decoder pool and compression mode.

**StoreAPI Decoder Pool:**
- `getStoreDecoderPoolInfo()` queries extended `SystemMemoryCompressionInformation` (0x54) to extract decoder pool fields: allocated pages, max pages, total requests, cache hits
- `hitRate` = cacheHits ÷ totalRequests (how often decoder context is reused)
- `poolExhausted` = true when free pages = 0 and max pages > 0
- `setStoreDecoderPoolSize(maxPages)` modifies the max pool size via NtSetSystemInformation

**Compression Mode:**
- `CompressionStoreMode` enum: `Auto` (0, system default), `MaxCompression` (1, better ratio/more CPU), `MaxPerformance` (2, lower CPU/maybe less ratio)
- Mode is read from byte offset 24 of the compression info structure
- `setCompressionStoreMode()` writes the mode back via NtSetSystemInformation

**Scheduler Integration:**
- `applyAdvancedCompressionTuning()` runs every 60s and adapts based on workload:
  - When compression is harmful (ratio <1.0): switches to MaxPerformance mode
  - During gaming with low decoder hit rate (<50%): expands pool by 256 pages
  - When healthy (ratio >2.0) with high pool usage: logs status (no action needed)

**Pressure Score Integration:**
- +5ps bonus when decoder hit rate <30% AND compressed memory >1GB

## SysMain / Superfetch Integration (v2.7.0)

`getSysMainServiceState()` queries the SysMain service via SCM API:
- Returns running state (bool), PID, startup type
- `setSysMainServiceEnabled()` starts/stops the service
- Used by `FluxScheduler::applyPagePriorityOrchestration()` to adjust page priority strategy based on whether Superfetch is active

## Memory Heatmap UI (v2.14.0)

The Heatmap tab (`MemoryHeatmapWidget`, `src/ui/MemoryHeatmapWidget.h/cpp`) provides a visual treemap of all running processes:

- **Block sizing**: each process rectangle is sized proportionally to `sqrt(WorkingSet)` so visual area correlates with memory footprint; minimum 30×30, capped at 45% of widget width
- **Block coloring**: gradient from green (0-25% RAM ratio) → yellow (25-50%) → orange (50-75%) → pink (>75%), computed by `memoryColor(ratio)` which linearly interpolates Catppuccin palette values
- **Foreground highlighting**: `GetForegroundWindow()` + `GetWindowThreadProcessId` identifies the foreground process; its block gets a pink border (`#f5c2e7`)
- **Tooltips**: on hover, displays full name, PID, Working Set (GB), % of total RAM, Private memory (GB), CPU%, and thread count
- **Header**: shows overall "Process Heatmap — X% used (Y / Z GB)"
- **Layout**: sorted by area (largest first), packed left-to-right top-to-bottom with 4px gap
- **Update cycle**: refreshed every 2s via `MainWindow::onMemoryUpdated()` → `m_heatmap->updateProcesses(snap, totalRam)`

The widget uses `QPainter` for custom rendering with `setMouseTracking(true)` for hover detection.

## Standby List Inteligente (v2.14.0)

The intelligent standby list subsystem elevates page priority of critical processes before flushing the standby list, preserving their cached pages for faster recall.

**Mechanism:**
- `NtApi::selectiveStandbyClean(maxPriority)` — enumerates all processes, finds those with:
  - Foreground window (via `GetForegroundWindow`)
  - Working set > `SL_FOREGROUND_WS_THRESHOLD` (500MB)
  - Game/miner status (via GameMode/MiningMode queries)
- For each critical process: calls `NtSetInformationProcess(ProcessPagePriority, Foreground)` to elevate page priority to 6
- For non-critical processes: lowers page priority to `SL_STANDBY_PRIORITY_BELOW_NORMAL` (2)
- Finally flushes standby list pages with priority ≤ `maxPriority` via `NtSetSystemInformation(SystemMemoryListInformation, MemoryPurgeStandbyList)`
- `NtApi::standbyPriorityDistribution()` returns histogram of page priorities currently in the standby list for diagnostics

**Scheduler Integration (`FluxScheduler::applyStandbyOrchestration()`):**
- Runs every `SL_ORCHESTRATION_INTERVAL_MS` (30s)
- Integrates with Hard Fault Predictor: if predicted score > warning threshold → selective clean; if critical → deep clean
- Respects disk queue guard (skips if queue > 1.5)
- Configurable via `setStandbyOrchestrationEnabled()` / `isStandbyOrchestrationEnabled()`

**Key Constants:**
| Constant | Value | Purpose |
|----------|-------|---------|
| `SL_STANDBY_PRIORITY_LOWEST` | 1 | Lowest page priority |
| `SL_STANDBY_PRIORITY_BELOW_NORMAL` | 2 | Non-critical processes |
| `SL_STANDBY_PRIORITY_NORMAL` | 5 | Default page priority |
| `SL_STANDBY_PRIORITY_FOREGROUND` | 6 | Foreground/critical processes |
| `SL_FOREGROUND_WS_THRESHOLD` | 500MB | Min WS to qualify as critical |
| `SL_ORCHESTRATION_INTERVAL_MS` | 30000 | Scheduler check interval |

## NUMA Optimization 2.0 (v2.14.0)

Major enhancement over v2.7.0 NUMA awareness:

**Improved `setProcessNumaAffinityByNode()`:**
- Now calls `getNumaNodeCpuMask()` to retrieve the full CPU affinity mask for the target NUMA node
- Applies mask via `SetProcessAffinityMask` (single-group compatible)
- Previously pinned to only 1 core — now pins to ALL cores on the node

**`NtApi::getBestNumaNodeForWorkload()` — Smart Node Selection:**
- Enumerates all NUMA nodes via `GetNumaHighestNodeNumber` + `GetNumaAvailableMemoryNodeEx`
- Selects node with most free memory
- Penalizes nodes where >4 processes already share L3 cache (detected via `getCacheTopology()`)
- Returns the optimal node index (or 0 if single-node)

**`NtApi::pinProcessToBestNumaNode(pid)`:**
- Combines `getBestNumaNodeForWorkload()` + `setProcessNumaAffinityByNode()`
- Returns the node pinned to (or -1 on failure)

**Integration:**
- `FluxGameMode::applyGameOptimizations()` — pins detected game to best NUMA node at game start
- `FluxMiningMode::applyMiningOptimizations()` — pins miner process to best NUMA node
- `FluxScheduler::applyStandbyOrchestration()` — queries NUMA info for priority distribution display

## Hard Fault Predictor 2.0 (v2.14.0)

Enhanced from v2.7.0's `HardFaultPredictor`:

**New Predictive Architecture:**
- `HardFaultHistory` struct: contains `std::deque<double> samples` (max 30), `std::mutex mtx`, `recordSample(value)`, `getSlope()`, `predictFuture(horizonSec)`, `size()`
- Sliding window of 30 samples collected at ~10s intervals (5 min window)
- `trendSlope()` — linear regression via least-squares over the deque
- `predictFuture(30)` — extends current trend 30s ahead: `lastValue + slope * horizon`

**`NtApi::getPredictiveHardFaultInfo()` — Extended Output:**
- `predictedScore30s` — forecasted hard fault score
- `estimatedTimeToThrashingSec` — if slope > 0, estimates seconds until score exceeds critical threshold
- `confidence` — based on sample count (0.0 = no data, 1.0 = ≥15 samples, minimum 6 samples to compute)

**`NtApi::applyPredictiveHardFaultManagement()`:**
- Called from `FluxScheduler::applyPredictiveHardFaultManagement()` every 10s
- If predicted score > `HF_WARNING_THRESHOLD`: triggers `FluxCleaner::selectiveClean()`
- If predicted score > `HF_CRITICAL_THRESHOLD`: triggers `FluxCleaner::deepClean()` + file cache flush
- Logs prediction state, slope, time-to-thrashing for diagnostics

**State Machine:**
```
Normal → (score > warning) → Warning → (cleaning)
       → (score > critical) → Critical → (deep cleaning)
       → (score drops) → Normal
```

## Process Memory Firewall (v2.14.0)

Per-process memory firewall via Windows Job Objects:

**Mechanism:**
- `NtApi::setProcessMemoryLimit(pid, maxBytes, killOnViolation)`:
  - Opens process with `PROCESS_SET_QUOTA | PROCESS_TERMINATE | SYNCHRONIZE`
  - Creates job object via `CreateJobObjectW(NULL, NULL)`
  - Sets `JOB_OBJECT_LIMIT_PROCESS_MEMORY` in `SetInformationJobObject(JobObjectLimitViolationInformation)`
  - If `killOnViolation`: sets `JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE` + `JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION`
  - Assigns process to job via `AssignProcessToJobObject`
  - Returns `MemoryFirewallRule` struct with job handle, PID, limit, kill flag

- `NtApi::quarantineMemoryLeak(pid)`:
  - Sets process memory limit to 1.5× current WS (tight limit)
  - Enables CPU throttle via `JOBOBJECT_CPU_RATE_CONTROL_INFORMATION` (20% hard cap)
  - Enables kill-on-violation

- `NtApi::releaseProcessMemoryLimit(pid)`:
  - Closes job handle → removes limit (process leaves job)
  - Cleans up `MemoryFirewallRule` tracking

- `NtApi::getProcessMemoryFirewallRule(pid)`:
  - Returns current rule for a process (or nullptr)

**Scheduler Integration (`FluxScheduler::applyProcessMemoryFirewall()`):**
- Runs every 30s (`m_lastFirewallCheck`)
- For each process with WS > 50% of total RAM: quarantines immediately
- For each process with WS > `MF_MONITOR_THRESHOLD_BYTES` (1GB):
  - Maintains `m_memoryHistory` (map PID → deque of 10 WS samples)
  - If 10 samples collected and delta between oldest/newest > `MF_GROWTH_THRESHOLD_BYTES` (200MB): applies memory limit at current WS + 25%
- Configurable via `setMemoryFirewallEnabled()` / `isMemoryFirewallEnabled()`

**MinGW Compatibility:**
- `JOB_OBJECT_LIMIT_PROCESS_MEMORY` (0x200), `JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE` (0x8000), `JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION` (0x400) defined manually via `#ifndef` since MinGW headers may lack these constants

## System File Cache Tuner (v2.14.0)

Dynamic system file cache management:

**Mechanism:**
- `NtApi::setSystemFileCacheSize(min, max)` via `NtSetSystemInformation(SystemFileCacheInformation, class 0x15)`:
  - Minimum and maximum file cache in bytes
  - Requires `SE_INCREASE_QUOTA_NAME` privilege (enabled via `adjustPrivilege`)
- `NtApi::getSystemFileCacheLimits()` — returns current `min`/`max` file cache limits via `NtQuerySystemInformation(SystemFileCacheInformation)`
- `NtApi::getFileCacheUsage()` — returns current file cache size in bytes (from `MemorySnapshot.cachedMemory`)

**Integration:**
- **Game Mode**: `FluxGameMode::applyGameOptimizations()` saves current limits, then sets `FC_GAME_CACHE_LIMIT_BYTES` (128MB); `restoreNormalOptimizations()` restores original values
- **Mining Mode**: similar, but uses `FC_MINING_CACHE_LIMIT_BYTES` (64MB)
- **Scheduler**: `applyFileCacheTuning()` runs every `FC_TUNE_INTERVAL_MS` (60s), periodically checks and re-applies the file cache limit if the game/mining mode is active

**Key Constants:**
| Constant | Value | Purpose |
|----------|-------|---------|
| `FC_GAME_CACHE_LIMIT_BYTES` | 128MB | Gaming mode file cache cap |
| `FC_MINING_CACHE_LIMIT_BYTES` | 64MB | Mining mode file cache cap |
| `FC_DEFAULT_CACHE_LIMIT_BYTES` | 512MB | Safe default |
| `FC_MIN_CACHE_BYTES` | 16MB | Absolute minimum |
| `FC_TUNE_INTERVAL_MS` | 60000 | Scheduler check interval |

## Memory Compression Manager (v2.14.0)

Enhanced from v2.7.0's compression tuning:

**Proactive Mode Switching:**
- During gaming/mining workloads: forces `MaxPerformance` mode (via `NtApi::setCompressionStoreMode(CompressionStoreMode::MaxPerformance)`) to minimize CPU overhead
- Tracks previous mode via `m_advancedCompressionMode` to avoid redundant API calls
- When not in performance workload and ratio > 2.0: restores `Auto` mode

**Decoder Pool Management:**
- When gaming/mining active and decoder hit rate < 50%: expands pool by 256 pages via `NtApi::setStoreDecoderPoolSize()`
- When healthy (ratio > 2.0) and pool usage > 80% with hit rate > 60%: expands pool by 128 pages (prepares for growth)

**Harmful Compression Handling:**
- When ratio < 1.0 (compression overhead exceeds benefit): switches to MaxPerformance (same as perf workload)
- Previously required 3 consecutive harmful samples to warn; now immediately switches mode

**Diagnostics:**
- Periodic logging every 10th check: compressed GB, uncompressed GB, savings percentage
- Controlled by `m_compressionLogCounter` modulo 10

## Mining Mode (v2.14.0)

New mode parallel to Game Mode for cryptocurrency mining workloads:

**`FluxMiningMode` (`src/mining/`):**
- `applyMiningOptimizations(pid)`:
  - Pins miner to best NUMA node via `NtApi::pinProcessToBestNumaNode(pid)`
  - Reduces system file cache to 64MB via `NtApi::setSystemFileCacheSize(FC_MIN_CACHE_BYTES)`
  - Applies process memory firewall to prevent miner leaks from crashing system
  - Activates selective standby cleaning to preserve miner pages
- `restoreNormalOptimizations()`:
  - Restores original file cache limits
  - Releases process memory firewall rules
- `isMiningRunning()` — returns whether a miner process is active
- Detection via process name patterns (e.g., containing "miner", "xmrig", etc.)

## Multi-level Cache Pressure (v2.9.0)

The cache pressure subsystem detects CPU cache topology and estimates contention based on active process working sets.

**Topology Detection:**
- `getCacheTopology()` calls `GetLogicalProcessorInformationEx` with `RelationCache`, `RelationProcessorCore`, and `RelationProcessorPackage`
- Returns `CacheTopology` with per-level sizes (KB), associativity, line size, core count, logical processor count, socket count
- Result is cached globally after first call

**Pressure Calculation:**
- `getCachePressure()` iterates `ProcessCache` for active processes (WS > 10MB)
- Per-level pressure = active WS / cache size, adjusted for sharing domain:
  - **L1**: private per logical processor → WS ÷ `processorCount`
  - **L2**: private per physical core → WS ÷ `coreCount`
  - **L3**: shared per socket → WS ÷ `socketsCount`
- `overallCachePressure` = weighted average (L1 20%, L2 30%, L3 50%)
- `l3Contended` = true when WS/socket > 1.5× L3 size

**Integration:**
- `FluxOptimizer::calculatePressureScore()` adds `cacheScore * 0.05` (5% weight) + 10pt bonus when `l3Contended`

## Page File Auto-Tuning (v2.8.0)

The Page File Auto-Tuning subsystem automates page file sizing based on system RAM and observed commit charge patterns.

**NTAPI Layer:**
- `getPageFileInfo()` queries `NtQuerySystemInformation(SystemPageFileInformation)` returning a `PageFileInfo` vector with path, current size (pages * 4096), peak usage, total usage percentage, and system-managed flag
- `getPageFileRecommendation()` computes:
  - `recommendedMinMB` = max(RAM/4, 4096MB) — page file can be smaller
  - `recommendedMaxMB` = min(RAM*3, peakUsage + RAM/2) — cap at 3x RAM
  - `currentPressure` from commit charge ÷ commit limit
  - `shouldResize` = true when pressure >80% AND current < target max
- `setPageFileSize(path, initialMB, maximumMB)` calls `NtSetSystemInformation(SystemPageFileInformation)` with `SE_CREATE_PAGEFILE_NAME` privileged enabled

**Scheduler Integration:**
- `applyPageFileTuning()` runs every 120s (`PF_TUNING_INTERVAL_MS`)
- At `PF_CLEAN_PRESSURE_THRESHOLD` (80%): triggers a quick clean via FluxCleaner
- At `PF_RESIZE_PRESSURE_THRESHOLD` (90%): calls `setPageFileSize()` with recommended min/max
- Suppressed during battery boost mode
- Configurable via `setPageFileAutoTuningEnabled()` / `isPageFileAutoTuningEnabled()`

## CPU Limiting Inteligente — CpuLimiter (v2.12.0)

The CpuLimiter subsystem limits background process CPU usage via Windows Job Objects with hard cap.

**Mechanism:**
- Creates a job object per target process via `CreateJobObjectW`
- Sets `JOBOBJECT_CPU_RATE_CONTROL_INFORMATION` with `JOB_OBJECT_CPU_RATE_CONTROL_HARD_CAP`
- Only limits processes when system CPU >70% (`m_systemCpuThreshold`)
- Only targets processes with WS >512MB (`m_processThresholdMB`)
- Limit percentage defaults to 50% and is configurable
- Removes the job when CPU normalizes below threshold

**Integration:**
- `FluxScheduler::applyResponsiveness()` enables/disables via slider level ≥3
- Auto-capped at 20-80% depending on slider level (10=20%, 0=disabled)
- Independent enable via `setCpuLimiterEnabled()` / `setCpuLimitPercent()`

## Power Plan Automation (v2.12.0)

Dynamically switches Windows power plans based on workload:

**Mechanism:**
- Loads `powrprof.dll` dynamically at runtime (no static link needed)
- `PowerSetActiveScheme(POWER_HIGH_PERFORMANCE)` for gaming/heavy workloads
- `PowerSetActiveScheme(POWER_SAVER)` for battery idle
- Falls back to balanced scheme when no action needed

**Trigger Logic:**
- Gaming → High Performance
- Battery + idle → Power Saver
- Everything else → Balanced (default)

**Integration:**
- `setPowerPlanAutomationEnabled()` / `isPowerPlanAutomationEnabled()`
- Auto-enabled by ResponsivenessSlider at any level >0

## Process Suspension por IA — ProcessSuspender (v2.12.0)

Suspends/resumes processes via NtSuspendProcess/NtResumeProcess (ntdll.dll):

**Mechanism:**
- Loads ntdll.dll dynamically, resolves NtSuspendProcess/NtResumeProcess
- Suspends processes with WS >1GB that have been inactive >120s
- Only triggers when memory pressure is Critical (>75%)
- Restricts max suspended processes to 20
- Automatically resumes after `MIN_SUSPEND_DURATION_MS` (60s) if pressure subsides
- Auto-resumes all on module shutdown

**Integration:**
- `setProcessSuspenderEnabled()` / `isProcessSuspenderEnabled()`
- `setSuspendThresholdMB()` / `setMinIdleSeconds()` configurable
- ResponsivenessSlider: level ≥4 enables, threshold 256MB-3GB, idle 20-300s

## QoS de Rede — NetworkQoS (v2.12.0)

Monitors network connections and applies DSCP-based priority:

**Mechanism:**
- `getProcessNetStats()` — enumerates TCP/UDP connections per PID via `GetExtendedTcpTable`/`GetExtendedUdpTable` (iphlpapi.dll, loaded dynamically)
- `setProcessNetworkPriority()` — sets DSCP value via `SetProcessInformation(ProcessNetQoSPolicy)` (info class 0x23)
- DSCP mapping: Very Low=0 (BE), Low=8 (CS1), Normal=0, High=46 (EF)
- Prioritizes game process as NET_PRIORITY_HIGH
- Deprioritizes processes with >100 connections as NET_PRIORITY_LOW, >200 as VERY_LOW
- Updates every 15s, automatically removes priority when connections normalize

## Responsiveness Slider (v2.12.0)

Central control that unifies all subsystem configurations into a single slider (0–10):

| Level | Label | CPU Cap | Suspend Threshold | Net QoS | Power Plan |
|-------|-------|---------|-------------------|---------|------------|
| 0 | Max Performance | Disabled | 3GB/300s | Off | Always High Perf |
| 3 | Light | 80% | 3GB/300s | Off | Balanced |
| 5 | Balanced | 50% | 1GB/120s | On | Balanced |
| 7 | Responsive | 35% | 512MB/45s | On | Balanced |
| 10 | Max Responsiveness | 20% | 256MB/20s | On | Power Save |

The `applyResponsiveness()` method is called at the top of the scheduler loop, configuring all subsystems before individual apply methods run.

## I/O Dashboard (v2.12.0)

Per-process and system-wide I/O monitoring with UI:

**Mechanism:**
- `getProcessIoStats()` — per-process read/write counters via `GetProcessIoCounters`
- `getSystemIoStats()` — system-wide I/O via `NtQuerySystemInformation(SystemIoInformation, class 21)`
- `IoMonitor` class computes rates (bytes/sec) by comparing deltas between 2s polling intervals

**UI:**
- `IoDashboardWidget` — new "I/O" tab in MainWindow
- System I/O read/write rates displayed at top
- Top 10 Readers and Top 10 Writers tables with PID, name, rate, and total
- Auto-refreshes every 3s

**Security:**
- Uses `verifyProcessName()` from `shared/ProcessUtils.h` (inline, shared across 4 modules) to prevent TOCTOU races on PID reuse

## EventBus com std::jthread (v2.12.0)

The EventBus dispatch thread was modernized from raw `std::thread` to `std::jthread` (C++20):

**Improvements:**
- RAII: `std::jthread` automatically joins on destruction (no explicit join needed)
- Cooperative cancellation via `std::stop_token` passed to `dispatchLoop()`
- `request_stop()` + `notify_all()` for clean shutdown
- Removed `m_running` atomic flag (replaced by stop_token)
- Condition variable wait predicate checks `stopToken.stop_requested()` for wakeup

## Process Memory Classifier (v2.11.0)

The Process Memory Classifier (`FluxClassifier`) categorizes processes by their memory usage pattern, enabling profile-specific optimization strategies.

**Pattern Detection:**
- `classifyPattern()` evaluates a deque of 60 WS samples per process using:
  - **Coefficient of Variation (CV)**: stddev ÷ mean — measures volatility
  - **Peak-to-Mean Ratio**: max(WS) ÷ mean(WS) — measures burstiness
  - **Growth Rate**: linear comparison of first-third vs last-third over time (MB/min)
  - **Zero-Crossings**: count of direction changes in the WS series

| Profile | Criteria | Strategy |
|---------|----------|----------|
| **Steady** | CV <0.15, peak/mean <1.3 | No action — stable usage |
| **Burst** | peak/mean >2.0, CV >0.5 | Trim after burst peak |
| **Periodic** | ≥3 zero-crossings, 0.2<CV<0.8 | Track cycle, prepare trim at peak |
| **Leaky** | growth >50MB/min, ≥20 samples | Trim periodically |
| **Unknown** | <10 samples | Need more data |

**Scheduler Integration:**
- `applyProcessClassification()` runs every 30s in the scheduler loop
- Feeds WS data from ProcessCache into `recordSample()` for each active process (>10MB WS)
- Calls `classifyAll()` to recompute all profiles with confidence scores
- Trims processes classified as Leaky or Burst with WS >500MB
- Logs tracked count, trim count, and monitor count

**Module Registration:**
- Registered as `FluxClassifier` in `ModuleManager` (main.cpp)
- Accessed via `dynamic_cast<Classifier::FluxClassifier*>(moduleManager.getModule("FluxClassifier"))`

## Safety Model

RAMFlux must:
- protect critical processes
- avoid optimization loops
- avoid excessive standby purge
- respect foreground applications
- respect fullscreen workloads

## UI Design Philosophy

Inspired by: Fluent Design, WinUI, telemetry dashboards, observability platforms

**Goals:** lightweight, modern, responsive, professional

## Stability Philosophy

Optimization is secondary to: stability, responsiveness, low overhead, safety

RAMFlux must never:
- destabilize Windows
- cause stuttering aggressively
- degrade responsiveness
- over-optimize

### Code Quality
- **v2.6.0 Full Security Audit**: 11 vulnerabilities/bugs fixed — privilege leak (SeDebugPrivilege nunca desabilitado via `disablePrivilege()`), race condition em ModuleManager, TOCTOU em `TerminateProcess`/`CreateProcessW` no watchdog (verificação `verifyProcessName()` via `QueryFullProcessImageNameW` + `_wcsicmp`), crash handler não signal-safe, HANDLE leak, `GetModuleFileNameW` sem verificação de erro
- **v2.5.2 Full Code Audit**: 18 source files reviewed — zero memory leaks, zero buffer overflows, zero race conditions, zero injection vectors

### Process Rules Engine (`src/rules/`)
A `IRulesEngine` module that provides persistent process rules and a process watchdog:
- **ProcessRulesEngine**: IModule implementation with a background thread (`watchdogLoop`) that polls `ProcessCache` every 5s
- **Rule persistence**: rules saved to QSettings (`processRules` array), survive restarts
- **Process matching**: supports simple wildcard (`*`) prefix/suffix matching (case-insensitive)
- **Rule types**: CPU priority, IO priority, page priority, CPU affinity, memory priority, watchdog
- **Watchdog actions**: terminate, restart (`QueryFullProcessImageNameW` + `CreateProcessW`), set priority, set affinity, log
- **Revert on rule removal**: `removeRule()` reverts applied rules via `revertAppliedRule()`, restoring original process settings
- **TOCTOU protection**: `verifyProcessName()` confirms PID→exe name match via `QueryFullProcessImageNameW` before executing destructive watchdog actions

### CPU Affinity Manager (`src/ui/CpuAffinityDialog`)
A visual dialog for selecting CPU affinity masks:
- **CpuAffinityDialog**: grid of checkboxes (8 per row) representing each logical CPU
- **Select All / Clear All**: convenience buttons for bulk selection
- **Returns `DWORD_PTR`**: affinity mask consumed by `ProcessRulesEngine` for `SetProcessAffinityMask`
- **Backed by**: `NtApi::getSystemCpuCount()` for dynamic CPU enumeration
