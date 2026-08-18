# RAMFlux
## Intelligent Memory Orchestrator for Windows

---

# PROJECT OVERVIEW

RAMFlux is a modern low-overhead Windows memory optimization platform focused on:

- intelligent memory orchestration
- telemetry
- observability
- adaptive heuristics
- process analytics
- stability-first optimization
- premium Windows 11 UX

The software is inspired by tools like:
- Memory Reduct
- Process Explorer
- telemetry dashboards
- observability platforms

---

# PRIMARY GOALS

RAMFlux aims to:

- improve system responsiveness
- reduce memory pressure intelligently
- avoid destructive optimizations
- provide advanced telemetry
- maintain extremely low overhead
- deliver a premium user experience

---

# CORE PHILOSOPHY

RAMFlux DOES NOT aim to:
- free RAM visually only
- aggressively purge memory constantly
- fake optimization results

RAMFlux DOES aim to:
- improve responsiveness
- optimize contextually
- operate safely
- adapt to workload behavior
- remain stable and lightweight

---

# TECHNOLOGY STACK

- C++20
- Qt6
- CMake
- Windows 10/11
- NTAPI
- MinGW 13.1.0 64-bit
- WiX Toolset v7

---

# PROJECT STRUCTURE

```text
RAMFlux/
│
├── src/
│   ├── ai/              (HeuristicEngine, WorkloadClassifier, PressurePredictor, MLEngine, IoCostTracker, StandbyScanner, PagePrefetcher)
│   ├── analyzer/        (FluxProcessAnalyzer — CPU sampling, leak detection)
│   ├── benchmark/       (BenchmarkRunner — 5-phase scientific benchmarking, tri-format reports)
│   ├── classifier/      (FluxClassifier — per-process memory pattern classification)
│   ├── cleaner/         (FluxCleaner — adaptive, battery-aware, idle-aware WS trim)
│   ├── core/            (EventBus c/ dispatch thread, FluxCore, ModuleManager, Logger c/ rotação)
│   ├── dedup/           (MemoryDedup — cross-process page dedup detection via FNV-1a hashing)
│   ├── diagnostics/     (DiagnosticsEngine — system diagnostics and health reporting)
│   ├── gamemode/        (FluxGameMode — fullscreen detection, VRAM monitoring, competitive mode)
│   ├── helper/          (HelperClient — IPC com RAMFluxHelper.exe elevado)
│   ├── hibernate/       (HibernateAssist — hibernation and fast startup management)
│   ├── io/              (ConfigIO — import/export config; UpdateChecker — version update check)
│   ├── leakhunter/      (LeakHunter — memory leak detection)
│   ├── mining/          (FluxMiningMode — CPU miner detection, priority boost, background throttle)
│   ├── ntapi/           (FluxNTAPI — compression, NUMA, page priority, disk queue, power, pool, QoS, I/O stats)
│   ├── optimizer/       (FluxOptimizer — pressure scoring)
│   ├── plugins/         (PluginManager — DLL-based plugin system with sandboxed execution)
│   ├── power/           (PowerManager — dynamic power plan switching)
│   ├── process/         (ProcessCache — WS aging tracking, session aggregation)
│   ├── profiles/        (ProfileManager — 5 perfis, reconfiguração dinâmica)
│   ├── qos/             (MemoryQoS — per-process memory SLAs with automatic enforcement)
│   ├── rules/           (ProcessRulesEngine — regras persistentes, wildcards, watchdog)
│   ├── scheduler/       (FluxScheduler — AI-driven, ProBalance, battery boost, CpuLimiter, ProcessSuspender, NetworkQoS, ResponsivenessSlider, IoMonitor, IoBandwidthThrottler)
│   ├── shared/          (Constants, ProcessUtils — verifyProcessName compartilhado)
│   ├── telemetry/       (FluxTelemetry, MemoryCollector, MemorySnapshot, HistoryBuffer)
│   └── ui/              (MainWindow, ConsoleWidget, IoDashboardWidget, Memory Map, ForecastWidget, AI Heuristics, NUMA, compression, CpuAffinityDialog, ThemeManager)
│
├── resources/           (manuals, icons, manifests, RC files)
├── Docs/                (architecture, roadmap, versioning, code style, phases)
├── build/               (CMake build output, MSI, deploy/)
├── installer/           (EULA, WiX assets)
├── CMakeLists.txt
├── installer.wxs        (WiX v4 → v7)
├── README.md
├── SECURITY.md
├── CHANGELOG.md
└── .gitignore