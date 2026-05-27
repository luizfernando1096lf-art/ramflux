# RAMFlux — Intelligent Memory Orchestrator for Windows

**RAMFlux** is a real-time memory monitoring and optimization tool for Windows 10/11, built with **Qt 6** and **C++20**. It provides deep system memory insights, automatic cache cleaning, process management, and leak detection — all through a modern, dark-themed interface.

## Features

- **Real-time Dashboard** — Memory charts, pressure scoring, and live telemetry at a glance
- **Process Manager** — View, analyze, and manage running processes with detailed memory metrics
- **Smart Optimization** — One-click "Smart Optimize" and "Deep Clean" to reclaim memory
- **Auto-Optimize** — Adaptive background cleaning based on system pressure and profiles
- **ProBalance** — Reduces priority of memory-heavy processes to keep the system responsive
- **Leak Hunter** — Detects processes with abnormal memory growth over time
- **File Cache Analyzer** — Shows which files are consuming the most system cache
- **Memory Map** — Visual breakdown of physical memory page distribution (Active, Standby, Modified, etc.)
- **Game Mode** — Automatically detects games and adjusts cleaning behavior for a smoother experience
- **Profiles** — Economy, Balanced, Performance, and Gaming presets
- **System Info** — Detailed OS, RAM, kernel, and page file information
- **Scheduled Cleaning** — Regular automated deep cleaning
- **Custom Settings** — Fine-tune polling intervals, clean areas, thresholds, and more
- **Console Log** — Real-time event log with severity filtering
- **System Tray** — Minimizes to tray with background operation

## Screenshots

*(Screenshots coming soon)*

## Download

| Version | Link |
|---------|------|
| **v1.1.0** (latest) | [RAMFlux_Setup_1.1.0.msi](https://github.com/luizfernando1096lf-art/ramflux/releases/latest) |

### Requirements

- Windows 10 or 11 (64-bit)
- 4 GB RAM (minimum)
- 200 MB disk space
- Administrator privileges for full functionality

## Building from Source

### Prerequisites

- **Windows 10/11** (64-bit)
- **Qt 6.11.0+** (MinGW or MSVC)
- **CMake 3.20+**
- **MinGW** (13.1.0+) or **Visual Studio 2022**
- **WiX Toolset v7.0** (optional, for MSI installer)

### Quick Build (PowerShell)

```powershell
.\build.ps1
```

### Manual Build

```cmd
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Build Installer

```cmd
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
cd ..
wix build installer.wxs -ext WixToolset.UI.wixext -out RAMFlux_Setup_x.x.x.msi
```

### Deploy

```powershell
# Copy built exe + dependencies to deployment folder:
Copy-Item build/RAMFlux.exe C:\ramflux\
# (Qt DLLs and platforms/qwindows.dll must also be copied)
```

## Project Structure

```
RAMFlux/
├── CMakeLists.txt
├── build.ps1 / build.bat          # Build scripts
├── installer.wxs                  # WiX installer definition
├── installer/                     # Installer assets (EULA)
├── resources/                     # Icons, RC files, Qt resources
├── src/
│   ├── main.cpp                   # Entry point
│   ├── core/                      # EventBus, Logger, ModuleManager
│   ├── ntapi/                     # Windows NT API wrappers (native)
│   ├── telemetry/                 # Memory collection, pressure analysis
│   ├── cleaner/                   # Memory cleaning routines (standby, modified, etc.)
│   ├── optimizer/                 # Optimization logic, foreground protection
│   ├── scheduler/                 # ProBalance scheduler
│   ├── profiles/                  # Profile management (Economy, Balanced, etc.)
│   ├── leakhunter/                # Memory leak detection
│   ├── gamemode/                  # Game detection and mode switching
│   ├── ui/                        # Qt widgets (MainWindow, settings, charts, tray, etc.)
│   ├── settings/                  # Settings persistence
│   └── shared/                    # Shared constants and types
```

## Tech Stack

| Layer | Technology |
|-------|-----------|
| Language | C++20 |
| GUI Framework | Qt 6.11 (Widgets, Charts, SystemTray) |
| Build System | CMake 3.20+ |
| Compiler | MinGW 13.1.0 / MSVC 2022 |
| Installer | WiX Toolset v7 |
| Native APIs | Win32, NtQuerySystemInformation, Toolhelp32 |

## License

MIT — see [LICENSE](LICENSE) for details.
