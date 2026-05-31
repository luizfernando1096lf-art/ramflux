# RAMFlux

**Intelligent Memory Orchestrator for Windows**

[![Version](https://img.shields.io/badge/version-2.4.0-blue.svg)](https://github.com/luizfernando1096lf-art/ramflux/releases)
[![Platform](https://img.shields.io/badge/platform-Windows%2010%2F11-0078D4.svg)](https://github.com/luizfernando1096lf-art/ramflux)
[![C++](https://img.shields.io/badge/C%2B%2B-20-00599C.svg)](https://en.cppreference.com/w/cpp/20)
[![Qt](https://img.shields.io/badge/Qt-6.11-41CD52.svg)](https://www.qt.io)
[![License](https://img.shields.io/badge/license-Proprietary-red.svg)](LICENSE)

RAMFlux is a modern, low-overhead memory optimization platform for Windows 10 and 11. It combines real-time telemetry, adaptive AI/ML heuristics, and safe NTAPI-level memory operations to improve system responsiveness without compromising stability.

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
- **Modern UI** — dark theme inspired by modern design principles

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

This project follows [Semantic Versioning](https://semver.org/). The current version is **2.4.0**.

| Stream | Version |
|--------|---------|
| Latest Release | [v2.4.0](https://github.com/luizfernando1096lf-art/ramflux/releases/tag/v2.4.0) |

See [VERSIONING.md](Docs/VERSIONING.md) for the version reference guide.

---

## Security

See [SECURITY.md](SECURITY.md) for the security policy and vulnerability reporting process.

---

## License

Proprietary. All rights reserved. Source code is not published.
