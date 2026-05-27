# Changelog

All notable changes to RAMFlux are documented here.

The format is based on [Keep a Changelog](https://keepachangelog.com/),
and this project adheres to [Semantic Versioning](https://semver.org/).

## [1.1.2] — 2026-05-27

### Fixed
- **UI: fundo branco na aba Console** — `ConsoleWidget` não tinha background explícito, herdando o tema claro do sistema. Adicionado `setStyleSheet("background-color: #1e1e2e")` no widget
- **UI: linhas brancas alternadas na tabela de processos** — `setAlternatingRowColors(true)` sem regra `::item:alternate` usava a cor padrão do sistema (branca). Adicionado `QTableWidget::item:alternate { background-color: #181825; }`

### Changed
- `ConsoleWidget`: fundo explícito `#1e1e2e` (Catppuccin Base) no widget raiz
- `ProcessListWidget`: estilo `::item:alternate` com `#181825` (Catppuccin Mantle) para uniformizar fundo escuro

---

## [Unreleased]

### Fixed

### Changed

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
