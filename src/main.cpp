// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include <QApplication>
#include <QTimer>
#include <QIcon>
#include <QSettings>
#include "shared/Constants.h"
#include "shared/StartupMarker.h"
#include <atomic>
#include <csignal>
#include <chrono>
#include <thread>
#include <windows.h>
#include <psapi.h>
#include <dbghelp.h>
#pragma comment(lib, "psapi.lib")
#include "core/Logger.h"
#include "ntapi/FluxNTAPI.h"
#include "core/FluxCore.h"
#include "telemetry/FluxTelemetry.h"
#include "cleaner/FluxCleaner.h"
#include "optimizer/FluxOptimizer.h"
#include "scheduler/FluxScheduler.h"
#include "analyzer/FluxProcessAnalyzer.h"
#include "gamemode/FluxGameMode.h"
#include "mining/FluxMiningMode.h"
#include "profiles/ProfileManager.h"
#include "leakhunter/LeakHunter.h"
#include "ai/HeuristicEngine.h"
#include "plugins/PluginManager.h"
#include "rules/ProcessRulesEngine.h"
#include "classifier/FluxClassifier.h"
#include "ui/MainWindow.h"
#include "benchmark/BenchmarkRunner.h"
using RAMFlux::Core::Logger;
using RAMFlux::markStartup;
static std::atomic<bool> g_running{
true};
extern "C" void signalHandler(int) {    g_running = false; }
LONG WINAPI crashHandler(EXCEPTION_POINTERS* ex) {
    char tempDir[MAX_PATH] = {0};
    if(!GetTempPathA(MAX_PATH, tempDir)) tempDir[0] = '\0';
    std::string logPath = std::string(tempDir) + "RAMFlux_crash.log";
    std::string dmpPath = std::string(tempDir) + "RAMFlux_crash.dmp";
    FILE* crashLog = nullptr;
    fopen_s(&crashLog, logPath.c_str(), "w");
    if(crashLog) {
        fprintf(crashLog, "RAMFlux CRASH\nException code: 0x%08lX\nException address: %p\n",
            (unsigned long)ex->ExceptionRecord->ExceptionCode, ex->ExceptionRecord->ExceptionAddress);
            void* stack[64];
            WORD frames = CaptureStackBackTrace(0, 64, stack, nullptr);
            fprintf(crashLog, "Stack trace (%d frames):\n", frames);
            for(WORD i = 0; i < frames; ++i)
                fprintf(crashLog, "  [%02d] %p\n", i, stack[i]);
            fprintf(crashLog, "\nLoaded modules:\n");
            HMODULE mods[1024];
            DWORD needed;
            if(EnumProcessModules(GetCurrentProcess(), mods, sizeof(mods), &needed)) {
                DWORD numMods = needed / sizeof(HMODULE);
                if(numMods > 1024) numMods = 1024;
                for(DWORD i = 0; i < numMods; i++) {
                    MODULEINFO mi;
                    if(!GetModuleInformation(GetCurrentProcess(), mods[i], &mi, sizeof(mi))) continue;
                    DWORD_PTR base = (DWORD_PTR)mi.lpBaseOfDll;
                    char name[MAX_PATH];
                    GetModuleBaseNameA(GetCurrentProcess(), mods[i], name, sizeof(name));
                    fprintf(crashLog, "  0x%llX  %s\n", (unsigned long long)base, name);
                }
            }
            fclose(crashLog);
    }
    HANDLE hFile = CreateFileA(dmpPath.c_str(), GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if(hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION mei{};
        mei.ThreadId = GetCurrentThreadId();
        mei.ExceptionPointers = ex;
        mei.ClientPointers = FALSE;
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
            hFile, MiniDumpWithIndirectlyReferencedMemory, &mei, nullptr, nullptr);
        CloseHandle(hFile);
    }
    char msg[4096];
    char stackBuf[2048] = "";
    void* stack[64];
    WORD frames = CaptureStackBackTrace(0, 64, stack, nullptr);
    for(WORD i = 0; i < frames && i < 12; ++i) {
        char tmp[64];
        snprintf(tmp, sizeof(tmp), "%p\n", stack[i]);
        strncat_s(stackBuf, sizeof(stackBuf), tmp, _TRUNCATE);
    }
    snprintf(msg, sizeof(msg),
        "RAMFlux encountered a fatal error.\n\n"
        "Exception code: 0x%08lX\n"
        "Address: %p\n\n"
        "Call stack:\n%s\n"
        "Crash dump: RAMFlux_crash.dmp\n\n"
        "Please report this issue with the dump file.",
        (unsigned long)ex->ExceptionRecord->ExceptionCode, ex->ExceptionRecord->ExceptionAddress, stackBuf);
    MessageBoxA(nullptr, msg, "RAMFlux Error", MB_ICONERROR | MB_OK);
    TerminateProcess(GetCurrentProcess(), 1);
    return EXCEPTION_EXECUTE_HANDLER;
}
int main(int argc, char* argv[]) {    bool headless = false;    bool cleanOnce = false;    bool reportOnce = false;    bool jsonReport = false;    bool optimizeOnce = false;    bool onceFlag = false;    bool showHelp = false;    bool benchmarkMode = false;    int benchBaseline = 60;    int benchPostOpt = 120;    int benchPressure = 30;    bool benchSkipPressure = false;    std::string benchOutput = ".\\benchmark_results";
    for(int i = 1; i < argc; ++i) {        std::string arg(argv[i]);
    if(arg == "--headless" || arg == "--silent" || arg == "-h" || arg == "--daemon") {            headless = true;        } else if(arg == "--clean" || arg == "-c") {            cleanOnce = true;        } else if(arg == "--report" || arg == "-r") {            reportOnce = true;        } else if(arg == "--json" || arg == "-j") {            jsonReport = true;        } else if(arg == "--optimize") {            optimizeOnce = true;            headless = true;        } else if(arg == "--once") {            onceFlag = true;            headless = true;        } else if(arg == "--help" || arg == "--usage") {            showHelp = true;        } else if(arg == "--benchmark" || arg == "-b") {            benchmarkMode = true;            auto readIntArg = [&](int& dst, int lo, int hi) {                if(i + 1 < argc && argv[i+1][0] != '-') {                    try { dst = std::max(lo, std::min(hi, std::stoi(argv[++i]))); }                    catch(...) { /* keep default on malformed argument */ }                }            };            readIntArg(benchBaseline, 5, 600);            readIntArg(benchPostOpt, 5, 600);        } else if(arg == "--bench-no-pressure") {            benchSkipPressure = true;        } else if(arg == "--bench-output" && i + 1 < argc) {            benchOutput = argv[++i];        }    }
    if(showHelp) {
        fprintf(stdout, "RAMFlux %s - Intelligent Memory Orchestrator\n", RAMFlux::Constants::APP_VERSION);
        fprintf(stdout, "Usage: RAMFlux [options]\n");
        fprintf(stdout, "  --headless, -h         Run without GUI\n");
        fprintf(stdout, "  --optimize             One-shot intelligent optimize (headless, uses priority-aware standby)\n");
        fprintf(stdout, "  --clean, -c            One-shot deep clean\n");
        fprintf(stdout, "  --report, -r           Print memory report and exit\n");
        fprintf(stdout, "  --json, -j             JSON output (with --report or --optimize)\n");
        fprintf(stdout, "  --once                 Exit after one operation (with --optimize/--clean)\n");
        fprintf(stdout, "  --benchmark, -b        Run benchmark\n");
        fprintf(stdout, "  --help                 Show this help\n");
        return 0;
    }        SetUnhandledExceptionFilter(crashHandler);
    SetErrorMode(SEM_FAILCRITICALERRORS);    markStartup("Init begin");    QApplication app(argc, argv);        app.setApplicationName("RAMFlux");
    app.setApplicationVersion(RAMFlux::Constants::APP_VERSION);    app.setOrganizationName("RAMFlux");    app.setWindowIcon(QIcon(":/app.ico"));
    Logger::instance().setLogFile("RAMFlux.log");
    if(!headless) {        Logger::instance().info("[RAMFlux] Initializing GUI...");    }    markStartup("Register modules");    auto& core = RAMFlux::Core::FluxCore::instance();    core.moduleManager().registerModule(        std::make_unique<RAMFlux::Telemetry::FluxTelemetry>());    core.moduleManager().registerModule(        std::make_unique<RAMFlux::Cleaner::FluxCleaner>());    core.moduleManager().registerModule(        std::make_unique<RAMFlux::Optimizer::FluxOptimizer>());    core.moduleManager().registerModule(        std::make_unique<RAMFlux::Scheduler::FluxScheduler>());    core.moduleManager().registerModule(        std::make_unique<RAMFlux::Analyzer::FluxProcessAnalyzer>());        core.moduleManager().registerModule(
        std::make_unique<RAMFlux::GameMode::FluxGameMode>());
    core.moduleManager().registerModule(
        std::make_unique<RAMFlux::Mining::FluxMiningMode>());
    core.moduleManager().registerModule(
        std::make_unique<RAMFlux::Profiles::ProfileManager>());    core.moduleManager().registerModule(        std::make_unique<RAMFlux::LeakHunter::LeakHunter>());
    core.moduleManager().registerModule(
        std::make_unique<RAMFlux::AI::HeuristicEngine>());
    core.moduleManager().registerModule(
        std::make_unique<RAMFlux::Rules::ProcessRulesEngine>());
    core.moduleManager().registerModule(
        std::make_unique<RAMFlux::Classifier::FluxClassifier>());
    core.moduleManager().registerModule(
        std::make_unique<RAMFlux::Plugins::PluginManager>());
    core.moduleManager().registerModule(
        std::make_unique<RAMFlux::Power::PowerManager>());
    markStartup("All modules registered");    if(!core.bootstrap()) {
return 1;    }    if(cleanOnce) {        Logger::instance().info("[RAMFlux] Running one-shot clean (--clean)");        auto* cleaner = dynamic_cast<RAMFlux::Cleaner::FluxCleaner*>(            core.moduleManager().getModule("FluxCleaner"));        if(cleaner) {            bool ok = cleaner->deepClean();            Logger::instance().info(std::string("[RAMFlux] Clean ") + (ok ? "completed" : "failed (cooldown)"));        }        core.shutdown();        return 0;    }
    if(optimizeOnce) {
        Logger::instance().info("[RAMFlux] Running one-shot intelligent optimize (--optimize)");
        // Wait for telemetry to populate (MemoryCollector interval ~1s)
        std::this_thread::sleep_for(std::chrono::seconds(3));
        auto* telemetry = dynamic_cast<RAMFlux::Telemetry::FluxTelemetry*>(
            core.moduleManager().getModule("FluxTelemetry"));
        auto* cleaner = dynamic_cast<RAMFlux::Cleaner::FluxCleaner*>(
            core.moduleManager().getModule("FluxCleaner"));
        bool ok = false;
        uint64_t recovered = 0;
        if(telemetry && cleaner) {
            auto snap = telemetry->lastSnapshot();
            uint64_t before = RAMFlux::NtApi::getStandbyMemorySize();
            ok = cleaner->intelligentStandbyClean(snap);
            if(!ok) ok = cleaner->trimColdPages();
            uint64_t after = RAMFlux::NtApi::getStandbyMemorySize();
            recovered = (before > after) ? (before - after) : 0;
            if(jsonReport) {
                fprintf(stdout, "{\"optimized\":%s,\"recoveredMB\":%llu,\"pressure\":%llu,\"standbyMB\":%llu}\n",
                    ok ? "true" : "false",
                    static_cast<unsigned long long>(recovered / (1024*1024)),
                    static_cast<unsigned long long>(snap.pressureScore),
                    static_cast<unsigned long long>(snap.standbyMemory / (1024*1024)));
            } else {
                Logger::instance().info(std::string("[RAMFlux] Optimize ") + (ok ? "completed" : "skipped")
                    + ", recovered " + std::to_string(recovered / (1024*1024)) + " MB");
            }
        }
        core.shutdown();
        return 0;
    }    if(benchmarkMode) {
        Logger::instance().info("[RAMFlux] Running benchmark mode");
        RAMFlux::Benchmark::BenchmarkConfig cfg;
        cfg.baselineDurationSec = benchBaseline;
        cfg.postOptDurationSec = benchPostOpt;
        cfg.pressureDurationSec = benchPressure;
        cfg.skipPressure = benchSkipPressure;
        cfg.outputDir = benchOutput;
        RAMFlux::Benchmark::BenchmarkRunner runner(cfg);
        if(runner.run()) {
            runner.generateReport();
            auto& r = runner.report();
            Logger::instance().info(std::string("[RAMFlux] Benchmark complete — Efficiency Score: ") + std::to_string(r.efficiencyScore) + "/100");
            if(!jsonReport) {
                Logger::instance().info("=== RAMFlux Benchmark Summary ===");
                Logger::instance().info(std::string("Efficiency Score:     ") + std::to_string(r.efficiencyScore) + " / 100");
                Logger::instance().info(std::string("Free RAM Improvement: ") + std::to_string(r.freeMemImprovementPct) + "%");
                Logger::instance().info(std::string("Standby Reduction:    ") + std::to_string(r.standbyReductionPct) + "%");
                Logger::instance().info(std::string("Hard Fault Change:    ") + std::to_string(r.hardFaultChangePct) + "%");
                Logger::instance().info(std::string("Pressure Reduction:   ") + std::to_string(r.pressureReductionPct) + "%");
                Logger::instance().info(std::string("Total Recovered:      ") + std::to_string(r.totalBytesRecovered / (1024 * 1024)) + " MB");
                Logger::instance().info("===============================");
            }
        } else {
            Logger::instance().error("[RAMFlux] Benchmark failed");
        }
        core.shutdown();
        return 0;
    }    if(reportOnce) {        using namespace RAMFlux::NtApi;        MEMORYSTATUSEX ms{}; ms.dwLength = sizeof(ms); GlobalMemoryStatusEx(&ms);        PERFORMANCE_INFORMATION pi{}; pi.cb = sizeof(pi); GetPerformanceInfo(&pi, sizeof(pi));        auto pm = getPhysicalMemoryBreakdown();        FILE* out = stdout;        if(jsonReport) {            fprintf(out, "{\"totalGB\":%.2f,\"usedGB\":%.2f,\"freeGB\":%.2f,\"loadPct\":%llu,",                    ms.ullTotalPhys / 1.0e9, (ms.ullTotalPhys - ms.ullAvailPhys) / 1.0e9, ms.ullAvailPhys / 1.0e9,                    static_cast<unsigned long long>(ms.dwMemoryLoad));            fprintf(out, "\"standbyGB\":%.2f,\"compressedGB\":%.2f,\"modifiedGB\":%.2f,",                    getStandbyMemorySize() / 1.0e9, getCompressedMemorySize() / 1.0e9, getTotalModifiedMemory() / 1.0e9);            fprintf(out, "\"pages\":{\"total\":%llu,\"active\":%llu,\"standby\":%llu,\"modified\":%llu,\"free\":%llu,\"zero\":%llu,\"bad\":%llu}}\n",                    static_cast<unsigned long long>(pm.totalPages), static_cast<unsigned long long>(pm.activePages),                    static_cast<unsigned long long>(pm.standbyPages), static_cast<unsigned long long>(pm.modifiedPages),                    static_cast<unsigned long long>(pm.freePages), static_cast<unsigned long long>(pm.zeroPages),                    static_cast<unsigned long long>(pm.badPages));        } else {            fprintf(out, "=== RAMFlux Memory Report ===\n");            fprintf(out, "Total:  %.2f GB\n", ms.ullTotalPhys / 1.0e9);            fprintf(out, "Used:   %.2f GB  (%llu%%)\n", (ms.ullTotalPhys - ms.ullAvailPhys) / 1.0e9, static_cast<unsigned long long>(ms.dwMemoryLoad));            fprintf(out, "Free:   %.2f GB\n", ms.ullAvailPhys / 1.0e9);            fprintf(out, "Standby:   %.2f GB\n", getStandbyMemorySize() / 1.0e9);            fprintf(out, "Modified:  %.2f GB\n", getTotalModifiedMemory() / 1.0e9);            fprintf(out, "Compressed:%.2f GB\n", getCompressedMemorySize() / 1.0e9);            fprintf(out, "--- Physical Pages ---\n");            fprintf(out, "Active:  %llu\n", static_cast<unsigned long long>(pm.activePages));            fprintf(out, "Standby: %llu\n", static_cast<unsigned long long>(pm.standbyPages));            fprintf(out, "Modified:%llu\n", static_cast<unsigned long long>(pm.modifiedPages));            fprintf(out, "Free:    %llu\n", static_cast<unsigned long long>(pm.freePages));            fprintf(out, "Zero:    %llu\n", static_cast<unsigned long long>(pm.zeroPages));            fprintf(out, "Bad:     %llu\n", static_cast<unsigned long long>(pm.badPages));        }        fflush(out);        core.shutdown();        return 0;    }    if (headless) {        Logger::instance().info("[RAMFlux] Running in headless mode (--headless)");        std::signal(SIGINT, signalHandler);        std::signal(SIGTERM, signalHandler);        QTimer pollTimer;        pollTimer.setInterval(500);        QObject::connect(&pollTimer, &QTimer::timeout, [&]() {            if(!g_running.load()) QApplication::quit();        });        pollTimer.start();        app.exec();    } else {        markStartup("Creating MainWindow");        RAMFlux::UI::MainWindow mainWindow;        markStartup("MainWindow created");        QSettings s(QSettings::IniFormat, QSettings::UserScope, "RAMFlux", "RAMFlux");        bool startMinimized = s.value("startMinimized", false).toBool();        if(!startMinimized) { mainWindow.show(); }        markStartup("app.exec");        app.exec();    }    core.shutdown();    Logger::instance().info("[RAMFlux] Exiting");    return 0;}


