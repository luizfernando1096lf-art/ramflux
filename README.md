# RAMFlux

**Intelligent Memory Orchestrator for Windows**

[![Version](https://img.shields.io/badge/version-2.6.0-blue.svg)](https://github.com/luizfernando1096lf-art/ramflux/releases)
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

This project follows [Semantic Versioning](https://semver.org/). The current version is **2.6.0**.

| Stream | Version |
|--------|---------|
| Latest Release | [v2.6.0](https://github.com/luizfernando1096lf-art/ramflux/releases/tag/v2.6.0) |

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
