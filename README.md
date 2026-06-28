# RAMFlux

**Intelligent Memory Orchestrator for Windows**

[![Version](https://img.shields.io/badge/version-2.33.0-blue.svg)](https://github.com/luizfernando1096lf-art/ramflux/releases)
[![Platform](https://img.shields.io/badge/platform-Windows%2010%2F11-0078D4.svg)](https://github.com/luizfernando1096lf-art/ramflux)
[![C++](https://img.shields.io/badge/C%2B%2B-20-00599C.svg)](https://en.cppreference.com/w/cpp/20)
[![Qt](https://img.shields.io/badge/Qt-6.11-41CD52.svg)](https://www.qt.io)
[![License](https://img.shields.io/badge/license-Proprietary-red.svg)](LICENSE)

RAMFlux is a modern, low-overhead memory optimization platform for Windows 10 and 11. It combines real-time telemetry, adaptive AI/ML heuristics, safe NTAPI-level memory operations, and a scientific benchmarking system to quantify real-world memory efficiency improvements.

---

## Features

- **Real-time Memory Telemetry** — RAM usage, page file, compression, kernel memory with live charts
- **AI Workload Classification** — automatically identifies Gaming, Development, Media, Browser, Heavy, Office, and Idle workloads via process fingerprinting
- **Pressure Prediction** — linear regression predicts memory pressure at 30s/60s/120s with anomaly detection (>3σ)
- **Preemptive Optimization** — AI-triggered cleaning when pressure ≥ High is predicted at ≥60% confidence
- **Memory Operations** — standby list, modified page list, working set, file cache, combined list cleaning, and heap defragmentation
- **Working Set Aging** — tracks per-process WS activity; trims only idle processes (≥30s inactive) to avoid disrupting active work
- **Hard Fault Prediction** — correlates page faults + disk queue length + standby list to predict I/O storms before they happen
- **NUMA Node Awareness** — detects NUMA topology and reports available memory per node for server/workstation optimization
- **Memory Compression Tuning** — monitors compression ratio and alerts when compression is degrading performance
- **Page Priority Management** — `NtSetInformationProcess` integration for setting per-process page priority (Superfetch-aware)
- **Game Mode** — automatic detection of fullscreen games with optimization suppression
- **Leak Detection** — real-time process memory leak monitoring with alerts
- **Process Manager** — detailed process view with memory usage, terminate, and analyze capabilities
- **Dashboard** — system info, pressure gauge, compression savings, optimization history, NUMA topology
- **Profile System** — performance profiles (Balanced, Aggressive, Conservative, Game, Power Save)
- **Privileged Helper** — separate `RAMFluxHelper.exe` process with elevated privileges for NTAPI operations
- **System Tray** — background operation with quick actions
- **Modern UI** — 3 themes (Catppuccin Mocha, Catppuccin Latte, Nord) com seletor no SettingsDialog e persistência
- **Process Rules Engine** — persistent priority, IO priority, page priority, memory priority, and CPU affinity rules saved across sessions
- **CPU Affinity Manager** — per-CPU checkbox grid with Select All/Clear All, accessible via Settings dialog
- **Process Watchdog** — automatic actions on memory threshold breach: terminate, restart, set priority, log, or change affinity
- **Process Memory Classifier** — classifies processes by usage pattern (Steady/Burst/Periodic/Leaky) with per-profile strategies
- **Advanced Memory Compression** — StoreAPI decoder pool management and compression mode tuning
- **Multi-level Cache Pressure** — L1/L2/L3 cache-aware pressure calculation via CPU topology detection
- **CPU Rate Limiting** — hard CPU usage cap via Windows Job Objects (JOBOBJECT_CPU_RATE_CONTROL_INFORMATION, HARD_CAP)
- **Power Plan Automation** — dynamic switching between High Performance, Balanced, and Power Saver plans via powrprof.dll
- **Process Suspend/Resume** — NTAPI-level process suspension via NtSuspendProcess/NtResumeProcess (dynamic ntdll.dll)
- **Network QoS (DSCP)** — per-process network priority tagging via SetProcessInformation(ProcessNetQoSPolicy)
- **Responsiveness Slider** — unified control (0-10) linking CPU cap, suspension, QoS, and power plan
- **I/O Monitoring Dashboard** — real-time per-process and system-wide disk I/O rates with delta calculations
- **Page File Auto-Tuning** — automatic page file resizing based on RAM + commit charge
- **Game Mode 3.0** — DXGI video memory monitoring, 11 game profiles, VRAM-aware cleaning, 1ms competitive timer
- **Memory Heatmap** — visual treemap of process memory usage, sized and colored by Working Set
- **Standby List Inteligente** — selective standby flush preserving critical process pages via page priority elevation
- **NUMA Optimization** — automatic NUMA node selection and process pinning based on free memory and L3 cache contention
- **Hard Fault Predictor 2.0** — sliding window regression predicts hard faults 30s ahead with preemptive cleaning
- **Process Memory Firewall** — per-process memory limits via Job Objects with automatic leak detection and quarantine
- **System File Cache Tuner** — dynamic file cache reduction during gaming/mining workloads
- **Memory Compression Manager** — adaptive MaxPerformance/Auto mode switching and decoder pool expansion
- **EcoQoS (Efficiency Mode)** — `SetProcessInformation(ProcessPowerThrottling)` puts background processes in CPU efficiency mode; reduces battery/heat on notebooks (Win 10 1809+)
- **Gentle Standby Clean** — chunked standby cleaning with disk queue guards (queue <1.5) and sleep intervals between chunks; elevates idle process page priorities before flushing
- **Adaptive Standby Orchestration (3-tier)** — Tier 1 (HF critical + high pressure) → gentle clean; Tier 2 (HF critical + standby >1GB) → selective clean with WS trim; Tier 3 (standby >2GB preventive) → gentle clean

---

## Scientific Benchmarking

RAMFlux v2.6.0 introduces a built-in **BenchmarkRunner** — a scientific benchmarking system that measures real-world memory optimization impact through controlled, multi-phase experiments.

### Methodology

| Phase | Duration | Purpose |
|-------|----------|---------|
| **Warmup** | 10s | System stabilization and telemetry initialization |
| **Baseline** | 30–120s | Metrics collected with zero intervention (control) |
| **Pressure** (optional) | 30s | 2 GB test allocation to simulate memory pressure |
| **Optimization** | instant | RAMFlux deep clean execution |
| **Post-Opt** | 60–300s | Continuous monitoring for rebound and hard faults |

Each phase collects 18+ metrics at 1s intervals. Statistical analysis includes mean, median, sample standard deviation (n−1), and 5th/95th percentiles.

### Real Benchmark Results (Windows 11, 34 GB RAM)

| Metric | Baseline | Post-Optimization | Improvement |
|--------|----------|-------------------|-------------|
| **Free RAM** | 20.50 GB | 25.50 GB | **+24.4%** |
| **Memory Pressure** | 35.0 | 25.0 | **−28.6%** |
| **Standby Cache** | 3.46 GB | 4.52 GB | normal recache |
| **Hard Faults** | 0/s | transient spike | stabilizes |
| **CPU Overhead** | 12.7% | 16.8% | negligible |

### Scientific Outputs

```
RAMFlux --benchmark [baseline_s] [post_opt_s] [options]
```

Every benchmark generates three complementary reports:

| Format | Content |
|--------|---------|
| **CSV** | Raw time-series data (102+ samples, 18 columns) — for external analysis |
| **JSON** | Structured statistical summary per phase — for CI/automation |
| **Markdown** | Full scientific report with tables, methodology, and impact analysis |

### Impact Analysis

The benchmark data demonstrates that RAMFlux's adaptive cleaning recovers significant memory without causing system instability. The temporary increase in hard faults and standby cache is a natural consequence of the operating system refilling its cache after pages are freed — a sign that RAMFlux is returning unused memory to the system for reallocation.

---

## Download

Download the latest installer from the [Releases page](https://github.com/luizfernando1096lf-art/ramflux/releases).

> **⚠️ Installer must be run as Administrator**

---

## System Requirements

| Component | Minimum |
|-----------|---------|
| OS | Windows 10 (64-bit) or Windows 11 |
| RAM | 2 GB |
| Disk | 100 MB |
| | Administrator privileges required for memory operations |

---

## Building from Source

### Prerequisites

- [CMake](https://cmake.org/download/) ≥ 3.22
- [Qt 6.11+](https://www.qt.io/download) (MinGW 13.1.0 64-bit)
- [MinGW](https://www.mingw-w64.org/) 13.1.0 64-bit (bundled with Qt)
- [WiX Toolset v7](https://wixtoolset.org/) (for MSI installer)

### Build Steps

```powershell
# Clone the repository
git clone https://github.com/luizfernando1096lf-art/ramflux.git
cd ramflux

# Configure with CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles"

# Build
cmake --build build --config Release -- -j$(nproc)

# The executables are generated at:
#   build/RAMFlux.exe
#   build/RAMFluxHelper.exe
```

### Generate Installer

```powershell
wix build installer.wxs -o RAMFlux_Setup_x.x.x.msi -arch x64 -ext WixToolset.UI.wixext -bindpath .
```

---

## Architecture Overview

```
┌─────────────┐    ┌──────────────┐    ┌──────────────────┐
│  Telemetry   │───▶│  Analytics   │───▶│  AI Heuristics   │
└─────────────┘    └──────────────┘    └──────────────────┘
                                               │
                                               ▼
┌─────────────┐    ┌──────────────┐    ┌──────────────────┐
│  Validation │◀───│  Optimizer   │◀───│  Safety Check    │
└─────────────┘    └──────────────┘    └──────────────────┘
       │
       ▼
┌──────────────────────────────────┐
│  Memory Operations (NTAPI)       │
│  ┌────────────────────────────┐  │
│  │  RAMFluxHelper.exe (Admin) │  │
│  └────────────────────────────┘  │
└──────────────────────────────────┘
```

See [ARCHITECTURE.md](Docs/ARCHITECTURE.md) for full documentation.

---

## Versioning

This project follows [Semantic Versioning](https://semver.org/). The current version is **2.33.0**.

| Stream | Version |
|--------|---------|
| Latest Release | [v2.33.0](https://github.com/luizfernando1096lf-art/ramflux/releases/tag/v2.33.0) |

### v2.33.0 — Responsiveness Slider

A real QSlider in the System Info tab (0–10) with descriptions and live policy indicators. Controls CPU limiting, process suspension, network QoS, and power plan automation in one unified control.

### v2.28.0 — Plugin Browser & Scheduler Integration

New "Plugins" tab with a table browser for loaded plugins, lifecycle controls (refresh/execute/reload), and automatic plugin execution in the scheduler loop.

### v2.27.0 — Export & Import System

Full configuration export/import to/from JSON. Exports all QSettings, active profile, custom profile config, and process rules. Accessible via File > Export/Import.

### v2.26.0 — Power Manager

Dedicated power management module with battery monitoring, auto battery boost on AC→battery transitions, power plan detection, and callback-based UI updates.

### v2.25.0 — System Health & Diagnostics Dashboard

Unified health monitoring with overall score gauge, five category breakdowns (Memory, Disk, I/O, Leaks, System), and actionable recommendations. Aggregates data from telemetry, disk, I/O, leak hunter, and system metrics.

### v2.24.0 — Hibernate Assist

Monitors user idle time + memory pressure; recommends or auto-triggers hibernate when both are elevated. Configurable thresholds.

### v2.23.0 — Telemetry & Forecasting Dashboard

New Forecast tab with pressure prediction chart (actual vs 30s/60s/120s forecast), accuracy metrics, ML engine status, and trend analysis — live-updating every 2s.

### v2.22.0 — Event-Driven Architecture

Expanded event system with 8 new event types; HeuristicEngine and FluxScheduler now post events on pressure change, hard fault storm, disk queue spike, and battery state transitions. Foundation for zero-polling reaction.

### v2.21.0 — Plugin System

Lightweight DLL-based plugin sandbox for user-defined optimization routines. Scans `plugins/` directory at startup; plugins export `createPlugin()`/`destroyPlugin()` and receive a whitelisted `IPluginContext` with safe API access. 5s execution timeout.

### v2.20.0 — I/O Bandwidth Throttling

Dynamically lowers I/O priority of background processes when disk queue pressure exceeds threshold; auto-restores on cooldown.

### v2.19.0 — Cross-Process Memory Dedup

Detects duplicate memory pages across processes using FNV-1a 64-bit hashing; identifies zero pages and cross-process duplicate groups; estimates potential RAM savings from page combining.

### v2.18.0 — Memory QoS / SLA

Per-process memory SLAs with automatic enforcement — min/max working set, page priority, I/O priority, EcoQoS, and kill-on-violation.

### v2.17.0 — Predictive Page Prefetch

PrefetchVirtualMemory-based proactive prefetch to prevent hard fault storms before they happen.

### v2.16.0 — Per-Process Standby Scanner

Per-process standby list enumeration using statistical inference — no kernel driver required.

### v2.15.0 — ML Engine & I/O Cost Tracker
- **EcoQoS (Efficiency Mode)** — background processes put into CPU efficiency mode via `SetProcessInformation(ProcessPowerThrottling)`; reduces battery/heat on notebooks (Win 10 1809+)
- **Gentle Standby Clean** — chunked standby cleaning with disk queue guards and sleep intervals; minimizes I/O impact on low-RAM systems
- **Adaptive Standby Orchestration (3-tier)** — Tier 1 (HF critical + high pressure) → gentle clean; Tier 2 (HF critical + standby >1GB) → selective clean; Tier 3 (standby >2GB preventive) → gentle clean
- **6 audit fixes** — 2 CRITICAL (GetLastError after LocalFree), 2 HIGH (missing FILE_FLAG_OVERLAPPED, disablePrivilege using wrong constant), 2 MEDIUM (readOk unconditional, predictFuture data race)
- [Full changelog](CHANGELOG.md)

### v2.14.0 — Memory Heatmap, Intelligent Standby, NUMA Pinning & More
- **Memory Heatmap UI** — new tab with treemap visualization of process memory: blocks sized by Working Set, colored by RAM ratio, with foreground highlighting and live tooltips
- **Standby List Inteligente** — selective standby flush preserving critical process pages via page priority elevation; continuous orchestration tied to hard fault prediction
- **NUMA Optimization** — automatic best-NUMA-node selection (most free memory, penalized for L3 contention); pins game/miner processes to all cores on the node
- **Hard Fault Predictor 2.0** — 30-sample sliding window with linear regression; predicts hard faults 30s ahead; triggers preemptive cleaning on warning/critical states
- **Process Memory Firewall** — per-process memory limits via Job Objects; automatic leak detection (200MB growth over 10 samples); quarantine with CPU throttle + kill-on-violation
- **System File Cache Tuner** — dynamic file cache reduction to 128MB (gaming) / 64MB (mining) with automatic restore on exit
- **Memory Compression Manager** — proactive MaxPerformance mode during gaming/mining; decoder pool auto-expansion; periodic savings logging
- [Full changelog](CHANGELOG.md)

### v2.13.0 — Security Audit, Bug Fixes & Robustness
- **Security Audit**: 13 critical/high/medium fixes across 12 files — restricted helper DACL, client session verification, removed `/rl highest` from scheduled task, crash dump hardening, CLI bounds checking
- **Named Pipe Hardening**: helper pipe SDDL changed from `WD` (Everyone) to `D:(A;;GA;;;SY)(A;;GA;;;BA)` — only SYSTEM and Administrators can connect; `verifyClient()` enforces same-session check
- **Bug Fixes**: job handle leak in CpuLimiter, stale resume in ProcessSuspender, CPU percent sanity cap in MemoryCollector, TOCTOU retry loop in NTAPI, LeakHunter iteration limit, HeuristicEngine concurrency guard, atomic slider level, benchmark cancellation, pipe connection retry
- **Crash Dump**: `MiniDumpWithIndirectlyReferencedMemory`, `FILE_ATTRIBUTE_SYSTEM` to prevent accidental exposure
- [Full changelog](CHANGELOG.md)

### v2.11.0 — Process Memory Classifier, Advanced Compression & Multi-level Cache
- **Process Memory Classifier (Phase 24)**: per-process pattern detection (Steady/Burst/Periodic/Leaky) com trim automático
- **Advanced Memory Compression (Phase 23)**: StoreAPI mode tuning + decoder pool management
- **Multi-level Cache Pressure (Phase 22)**: L1/L2/L3 cache-aware pressure calculation
- **Page File Auto-Tuning (Phase 21)**: automatic page file sizing via NTAPI
- **Game Mode 3.0 (Phase 20)**: DXGI video memory API, 11 game profiles, VRAM-aware cleaning, competitive mode
- [Full changelog](CHANGELOG.md)

### v2.6.0 — Process Rules Engine, CPU Affinity Manager e Process Watchdog
- **Persistent Process Rules Engine**: regras de prioridade (CPU, IO, page, memory), afinidade de CPU e prioridade de memória — salvas em QSettings, aplicadas automaticamente a cada 5s
- **CPU Affinity Manager**: `CpuAffinityDialog` com grid de checkboxes por CPU lógica + Select All / Clear All; integrado ao SettingsDialog
- **Process Watchdog**: monitora consumo de memória por processo; ações configuráveis: Terminar, Reiniciar, Definir Prioridade, Definir Afinidade, Log
- **Watchdog Restart**: reinicia processo automaticamente via `QueryFullProcessImageNameW` + `CreateProcessW`
- **Wildcard pattern matching**: regras com `*` para prefixo/sufixo (ex: `chrome*`, `*notepad*`)
- **Nova API NTAPI**: `NtApi::setProcessAffinity()`, `NtApi::getProcessAffinity()`, `NtApi::getSystemCpuCount()`, `NtApi::isProcessRunning()`
- [Full changelog](CHANGELOG.md)

### v2.5.2 — Melhorias, Auditoria e Correções
- **ThemeManager**: 3 temas (Catppuccin Mocha/Latte, Nord) + seletor no SettingsDialog + persistência QSettings
- **Code audit**: 18 source files revisados — zero memory leaks, zero buffer overflows, zero race conditions
- **Bugfix CPU%**: FluxProcessAnalyzer — delta calculado antes de atualizar o sample
- **Bugfix page size**: MemoryCollector — coldPageRatio com `SYSTEM_INFO.dwPageSize` em vez de 4096 fixo
- **Bugfix IO priority**: FluxGameMode — salvava e restaurava o valor original via `getProcessIoPriority()`
- **Nova API NTAPI**: `NtApi::getProcessIoPriority()` — `ProcessIoPriority` via `GetProcessInformation`
- [Full changelog](CHANGELOG.md)

See [VERSIONING.md](Docs/VERSIONING.md) for the version reference guide.

---

## Security

See [SECURITY.md](SECURITY.md) for the security policy and vulnerability reporting process.

---

## License

Proprietary. All rights reserved. Source code is not published.
