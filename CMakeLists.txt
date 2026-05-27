cmake_minimum_required(VERSION 3.22)
project(RAMFlux VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-cast-function-type")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)

set(CORE_SOURCES
    src/core/EventBus.cpp
    src/core/FluxCore.cpp
    src/core/Logger.cpp
    src/core/ModuleManager.cpp
)

set(TELEMETRY_SOURCES
    src/telemetry/FluxTelemetry.cpp
    src/telemetry/MemoryCollector.cpp
)

set(NTAPI_SOURCES
    src/ntapi/FluxNTAPI.cpp
)

set(CLEANER_SOURCES
    src/cleaner/FluxCleaner.cpp
)

set(OPTIMIZER_SOURCES
    src/optimizer/FluxOptimizer.cpp
)

set(SCHEDULER_SOURCES
    src/scheduler/FluxScheduler.cpp
)

set(ANALYZER_SOURCES
    src/analyzer/FluxProcessAnalyzer.cpp
)

set(GAMEMODE_SOURCES
    src/gamemode/FluxGameMode.cpp
)

set(PROFILES_SOURCES
    src/profiles/ProfileManager.cpp
)

set(LEAKHUNTER_SOURCES
    src/leakhunter/LeakHunter.cpp
)

set(UI_SOURCES
    src/ui/MainWindow.cpp
    src/ui/MemoryCard.cpp
    src/ui/HistoryChart.cpp
    src/ui/ProcessListWidget.cpp
    src/ui/SettingsDialog.cpp
    src/ui/SystemTrayManager.cpp
    src/ui/ConsoleWidget.cpp
)

set(RESOURCES
    resources/app.qrc
)

add_executable(RAMFlux
    src/main.cpp
    ${CORE_SOURCES}
    ${TELEMETRY_SOURCES}
    ${NTAPI_SOURCES}
    ${CLEANER_SOURCES}
    ${OPTIMIZER_SOURCES}
    ${SCHEDULER_SOURCES}
    ${ANALYZER_SOURCES}
    ${GAMEMODE_SOURCES}
    ${PROFILES_SOURCES}
    ${LEAKHUNTER_SOURCES}
    ${UI_SOURCES}
    src/core/EventBus.h
    src/core/FluxCore.h
    src/core/IModule.h
    src/core/Logger.h
    src/core/ModuleManager.h
    src/telemetry/FluxTelemetry.h
    src/telemetry/MemoryCollector.h
    src/telemetry/MemorySnapshot.h
    src/ntapi/FluxNTAPI.h
    src/cleaner/FluxCleaner.h
    src/optimizer/FluxOptimizer.h
    src/scheduler/FluxScheduler.h
    src/analyzer/FluxProcessAnalyzer.h
    src/gamemode/FluxGameMode.h
    src/profiles/ProfileManager.h
    src/leakhunter/LeakHunter.h
    src/ui/MainWindow.h
    src/ui/MemoryCard.h
    src/ui/HistoryChart.h
    src/ui/ProcessListWidget.h
    src/ui/SettingsDialog.h
    src/ui/SystemTrayManager.h
    src/ui/ConsoleWidget.h
    src/shared/Constants.h
    ${RESOURCES}
)

target_include_directories(RAMFlux PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(RAMFlux PRIVATE
    Qt6::Widgets
    Qt6::Charts
)

if(MINGW)
    target_link_libraries(RAMFlux PRIVATE psapi dbghelp)
endif()

set_target_properties(RAMFlux PROPERTIES
    WIN32_EXECUTABLE TRUE
)

target_sources(RAMFlux PRIVATE
    resources/app.rc
)
