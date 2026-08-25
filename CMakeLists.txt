cmake_minimum_required(VERSION 3.22)
project(RAMFlux VERSION 2.51.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-cast-function-type")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts Network Concurrent)

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
    src/helper/HelperClient.cpp
)

set(OPTIMIZER_SOURCES
    src/optimizer/FluxOptimizer.cpp
)

set(SCHEDULER_SOURCES
    src/scheduler/FluxScheduler.cpp
    src/scheduler/CpuLimiter.cpp
    src/scheduler/ProcessSuspender.cpp
    src/scheduler/NetworkQoS.cpp
    src/scheduler/ResponsivenessSlider.cpp
    src/scheduler/IoMonitor.cpp
    src/scheduler/IoBandwidthThrottler.cpp
)

set(ANALYZER_SOURCES
    src/analyzer/FluxProcessAnalyzer.cpp
)

set(GAMEMODE_SOURCES
    src/gamemode/FluxGameMode.cpp
)

set(MINING_SOURCES
    src/mining/FluxMiningMode.cpp
)

set(PROCESS_SOURCES
    src/process/ProcessCache.cpp
)

set(PROFILES_SOURCES
    src/profiles/ProfileManager.cpp
)

set(LEAKHUNTER_SOURCES
    src/leakhunter/LeakHunter.cpp
)

set(QOS_SOURCES
    src/qos/MemoryQoS.cpp
)

set(DEDUP_SOURCES
    src/dedup/MemoryDedup.cpp
)

set(PLUGIN_SOURCES
    src/plugins/PluginManager.cpp
)

set(HIBERNATE_SOURCES
    src/hibernate/HibernateAssist.cpp
)

set(AI_SOURCES
    src/ai/HeuristicEngine.cpp
    src/ai/WorkloadClassifier.cpp
    src/ai/PressurePredictor.cpp
    src/ai/MLEngine.cpp
    src/ai/IoCostTracker.cpp
    src/ai/StandbyScanner.cpp
    src/ai/PagePrefetcher.cpp
)

set(RULES_SOURCES
    src/rules/ProcessRulesEngine.cpp
)

set(CLASSIFIER_SOURCES
    src/classifier/FluxClassifier.cpp
)

set(BENCHMARK_SOURCES
    src/benchmark/BenchmarkRunner.cpp
)

set(DIAGNOSTICS_SOURCES
    src/diagnostics/DiagnosticsEngine.cpp
)

set(POWER_SOURCES
    src/power/PowerManager.cpp
)

set(IO_SOURCES
    src/io/ConfigIO.cpp
    src/io/UpdateChecker.cpp
)

set(UI_SOURCES
    src/ui/MainWindow.cpp
    src/ui/ThemeManager.cpp
    src/ui/MemoryCard.cpp
    src/ui/HistoryChart.cpp
    src/ui/ProcessListWidget.cpp
    src/ui/SettingsDialog.cpp
    src/ui/CpuAffinityDialog.cpp
    src/ui/SystemTrayManager.cpp
    src/ui/ConsoleWidget.cpp
    src/ui/IoDashboardWidget.cpp
    src/ui/MemoryHeatmapWidget.cpp
    src/ui/ForecastWidget.cpp
    src/ui/HealthDashboardWidget.cpp
    src/ui/PluginBrowserWidget.cpp
    src/ui/BenchmarkWidget.cpp
    src/ui/DedupWidget.cpp
    src/ui/ResponsivenessWidget.cpp
    src/ui/HibernateWidget.cpp
    src/ui/PowerPlanWidget.cpp
    src/ui/StandbyWidget.cpp
    src/ui/ClassifierWidget.cpp
    src/ui/PrefetchWidget.cpp
    src/ui/QosWidget.cpp
    src/ui/SchedulerDashboardWidget.cpp
)

set(RESOURCES
    resources/app.qrc
)

add_executable(RAMFluxHelper WIN32
    src/helper/RAMFluxHelper.cpp
    resources/helper.rc
)
target_include_directories(RAMFluxHelper PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(RAMFluxHelper PRIVATE psapi dbghelp)

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
    ${MINING_SOURCES}
    ${PROCESS_SOURCES}
    ${PROFILES_SOURCES}
    ${LEAKHUNTER_SOURCES}
    ${QOS_SOURCES}
    ${DEDUP_SOURCES}
    ${PLUGIN_SOURCES}
    ${HIBERNATE_SOURCES}
    ${AI_SOURCES}
    ${BENCHMARK_SOURCES}
    ${DIAGNOSTICS_SOURCES}
    ${POWER_SOURCES}
    ${IO_SOURCES}
    ${RULES_SOURCES}
    ${CLASSIFIER_SOURCES}
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
    src/scheduler/CpuLimiter.h
    src/scheduler/ProcessSuspender.h
    src/scheduler/NetworkQoS.h
    src/scheduler/ResponsivenessSlider.h
    src/scheduler/IoMonitor.h
    src/scheduler/IoBandwidthThrottler.h
    src/analyzer/FluxProcessAnalyzer.h
    src/gamemode/FluxGameMode.h
    src/mining/FluxMiningMode.h
    src/profiles/ProfileManager.h
    src/process/ProcessCache.h
    src/leakhunter/LeakHunter.h
    src/hibernate/HibernateAssist.h
    src/diagnostics/DiagnosticsEngine.h
    src/power/PowerManager.h
    src/io/ConfigIO.h
    src/io/UpdateChecker.h
    src/ai/HeuristicEngine.h
    src/ai/WorkloadClassifier.h
    src/ai/PressurePredictor.h
    src/ui/MainWindow.h
    src/ui/MemoryCard.h
    src/ui/HistoryChart.h
    src/ui/ProcessListWidget.h
    src/ui/SettingsDialog.h
    src/ui/CpuAffinityDialog.h
    src/ui/SystemTrayManager.h
    src/ui/ConsoleWidget.h
    src/ui/IoDashboardWidget.h
    src/ui/MemoryHeatmapWidget.h
    src/ui/ForecastWidget.h
    src/ui/HealthDashboardWidget.h
    src/ui/PluginBrowserWidget.h
    src/ui/BenchmarkWidget.h
    src/ui/DedupWidget.h
    src/ui/ResponsivenessWidget.h
    src/ui/HibernateWidget.h
    src/ui/PowerPlanWidget.h
    src/ui/StandbyWidget.h
    src/ui/ClassifierWidget.h
    src/ui/PrefetchWidget.h
    src/ui/QosWidget.h
    src/ui/SchedulerDashboardWidget.h
    src/ui/ThemeManager.h
    src/shared/Constants.h
    src/shared/ProcessUtils.h
    src/helper/HelperClient.h
    src/rules/ProcessRulesEngine.h
    src/rules/RuleTypes.h
    src/classifier/FluxClassifier.h
    ${RESOURCES}
)

target_include_directories(RAMFlux PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(RAMFlux PRIVATE
    Qt6::Widgets
    Qt6::Charts
    Qt6::Network
    Qt6::Concurrent
)

if(MINGW)
    target_link_libraries(RAMFlux PRIVATE psapi dbghelp wtsapi32 dxgi powrprof)
endif()
if(MSVC)
    target_link_libraries(RAMFlux PRIVATE psapi dbghelp wtsapi32 dxgi powrprof)
endif()

set_target_properties(RAMFlux PROPERTIES
    WIN32_EXECUTABLE TRUE
)

target_sources(RAMFlux PRIVATE
    resources/app.rc
)
