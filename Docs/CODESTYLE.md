# Code Style & Conventions

## C++ Standard
- C++20

## Naming
- **Namespaces**: `PascalCase` (`RAMFlux::Telemetry`, `RAMFlux::UI`)
- **Classes**: `PascalCase` (`FluxTelemetry`, `ProcessCache`)
- **Methods/Functions**: `camelCase` (`setPollingInterval`, `processCount`)
- **Member variables**: `m_` prefix + `camelCase` (`m_pollingIntervalMs`, `m_callbacks`)
- **Constants/Macros**: `UPPER_SNAKE_CASE` (`APP_VERSION`, `LEAK_CHECK_INTERVAL_MS`)
- **Files**: `PascalCase` matching class name (`FluxTelemetry.h`, `ProcessCache.cpp`)

## Includes
1. Own header first (`MainWindow.cpp` → `MainWindow.h`)
2. Project headers (`"core/EventBus.h"`)
3. Qt headers (`<QWidget>`)
4. STL headers (`<vector>`)
5. Platform headers (`<windows.h>`)

## Thread Safety
- `std::mutex` for shared state; prefer `std::atomic` for simple flags/counters
- No recursive mutex — reorganize locks to avoid reentrancy
- EventBus dispatch thread processes callbacks; UI updates use `QMetaObject::invokeMethod(..., Qt::QueuedConnection)`

## Memory
- No raw `new`/`delete`; use `std::unique_ptr`, `std::shared_ptr`, or QObject parent ownership
- ProcessCache singleton via `static` local in `instance()` method

## Qt Patterns
- Signals/slots with `QOverload<int>::of(...)` for overloaded signals
- UI updates only on main thread
- Stylesheets inline in C++ (QSS strings), not separate files
