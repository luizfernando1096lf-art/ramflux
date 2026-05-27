// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include <QApplication>
#include <QIcon>
#include <QSettings>
#include <atomic>
#include <csignal>
#include <ctime>
#include <chrono>
#include <thread>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <windows.h>
#include <psapi.h>
#include <dbghelp.h>
#include "core/Logger.h"
#include "ntapi/FluxNTAPI.h"
#include "core/FluxCore.h"
#include "telemetry/FluxTelemetry.h"
#include "cleaner/FluxCleaner.h"
#include "optimizer/FluxOptimizer.h"
#include "scheduler/FluxScheduler.h"
#include "analyzer/FluxProcessAnalyzer.h"
#include "gamemode/FluxGameMode.h"
#include "profiles/ProfileManager.h"
#include "leakhunter/LeakHunter.h"
#include "ui/MainWindow.h"
using RAMFlux::Core::Logger;
static std::atomic<bool> g_running{
true};
extern "C" void signalHandler(int) {    g_running = false;
    Logger::instance().info("\n[RAMFlux] Shutdown signal received");
}
static std::string getCrashDumpPath() {    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
std::tm tm;    localtime_s(&tm, &t);
std::ostringstream oss;    oss << "RAMFlux_crash_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".dmp";
    return oss.str();
}
LONG WINAPI crashHandler(EXCEPTION_POINTERS* ex) {    std::ostringstream oss;    oss << "[RAMFlux] CRASH: Exception code 0x" << std::hex << ex->ExceptionRecord->ExceptionCode;
    Logger::instance().error(oss.str());
std::string dumpPath = getCrashDumpPath();
HANDLE hFile = CreateFileA(dumpPath.c_str(), GENERIC_WRITE, 0, nullptr,        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if(hFile != INVALID_HANDLE_VALUE) {        MINIDUMP_EXCEPTION_INFORMATION mei{};
mei.ThreadId = GetCurrentThreadId();        mei.ExceptionPointers = ex;        mei.ClientPointers = FALSE;
    if(MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),            hFile, MiniDumpNormal, &mei, nullptr, nullptr)) {            Logger::instance().error("[RAMFlux] Crash dump saved to " + dumpPath);        }        CloseHandle(hFile);    }    Logger::instance().error("[RAMFlux] Crash handler finished. Exiting.");
    return EXCEPTION_EXECUTE_HANDLER;
}
int main(int argc, char* argv[]) {    bool headless = false;    bool cleanOnce = false;    bool reportOnce = false;    bool jsonReport = false;
    for(int i = 1; i < argc; ++i) {        std::string arg(argv[i]);
    if(arg == "--headless" || arg == "--silent" || arg == "-h" || arg == "--daemon") {            headless = true;        } else if(arg == "--clean" || arg == "-c") {            cleanOnce = true;        } else if(arg == "--report" || arg == "-r") {            reportOnce = true;        } else if(arg == "--json" || arg == "-j") {            jsonReport = true;        }    }    SetUnhandledExceptionFilter(crashHandler);    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);    QApplication app(argc, argv);    app.setApplicationName("RAMFlux");    app.setApplicationVersion("2.0.0");    app.setOrganizationName("RAMFlux");    app.setWindowIcon(QIcon(":/app.ico"));
    Logger::instance().setLogFile("RAMFlux.log");
    if(!headless) {        Logger::instance().info("[RAMFlux] Initializing GUI...");    }    auto& core = RAMFlux::Core::FluxCore::instance();    core.moduleManager().registerModule(        std::make_unique<RAMFlux::Telemetry::FluxTelemetry>());    core.moduleManager().registerModule(        std::make_unique<RAMFlux::Cleaner::FluxCleaner>());    core.moduleManager().registerModule(        std::make_unique<RAMFlux::Optimizer::FluxOptimizer>());    core.moduleManager().registerModule(        std::make_unique<RAMFlux::Scheduler::FluxScheduler>());    core.moduleManager().registerModule(        std::make_unique<RAMFlux::Analyzer::FluxProcessAnalyzer>());    core.moduleManager().registerModule(        std::make_unique<RAMFlux::GameMode::FluxGameMode>());    core.moduleManager().registerModule(        std::make_unique<RAMFlux::Profiles::ProfileManager>());    core.moduleManager().registerModule(        std::make_unique<RAMFlux::LeakHunter::LeakHunter>());
    if(!core.bootstrap()) {
return 1;    }    if(cleanOnce) {        Logger::instance().info("[RAMFlux] Running one-shot clean (--clean)");        auto* cleaner = dynamic_cast<RAMFlux::Cleaner::FluxCleaner*>(            core.moduleManager().getModule("FluxCleaner"));        if(cleaner) {            bool ok = cleaner->deepClean();            Logger::instance().info(std::string("[RAMFlux] Clean ") + (ok ? "completed" : "failed (cooldown)"));        }        core.shutdown();        return 0;    }    if(reportOnce) {        using namespace RAMFlux::NtApi;        MEMORYSTATUSEX ms{}; ms.dwLength = sizeof(ms); GlobalMemoryStatusEx(&ms);        PERFORMANCE_INFORMATION pi{}; pi.cb = sizeof(pi); GetPerformanceInfo(&pi, sizeof(pi));        auto pm = getPhysicalMemoryBreakdown();        FILE* out = stdout;        if(jsonReport) {            fprintf(out, "{\"totalGB\":%.2f,\"usedGB\":%.2f,\"freeGB\":%.2f,\"loadPct\":%llu,",                    ms.ullTotalPhys / 1.0e9, (ms.ullTotalPhys - ms.ullAvailPhys) / 1.0e9, ms.ullAvailPhys / 1.0e9,                    static_cast<unsigned long long>(ms.dwMemoryLoad));            fprintf(out, "\"standbyGB\":%.2f,\"compressedGB\":%.2f,\"modifiedGB\":%.2f,",                    getStandbyMemorySize() / 1.0e9, getCompressedMemorySize() / 1.0e9, getTotalModifiedMemory() / 1.0e9);            fprintf(out, "\"pages\":{\"total\":%llu,\"active\":%llu,\"standby\":%llu,\"modified\":%llu,\"free\":%llu,\"zero\":%llu,\"bad\":%llu}}\n",                    static_cast<unsigned long long>(pm.totalPages), static_cast<unsigned long long>(pm.activePages),                    static_cast<unsigned long long>(pm.standbyPages), static_cast<unsigned long long>(pm.modifiedPages),                    static_cast<unsigned long long>(pm.freePages), static_cast<unsigned long long>(pm.zeroPages),                    static_cast<unsigned long long>(pm.badPages));        } else {            fprintf(out, "=== RAMFlux Memory Report ===\n");            fprintf(out, "Total:  %.2f GB\n", ms.ullTotalPhys / 1.0e9);            fprintf(out, "Used:   %.2f GB  (%llu%%)\n", (ms.ullTotalPhys - ms.ullAvailPhys) / 1.0e9, static_cast<unsigned long long>(ms.dwMemoryLoad));            fprintf(out, "Free:   %.2f GB\n", ms.ullAvailPhys / 1.0e9);            fprintf(out, "Standby:   %.2f GB\n", getStandbyMemorySize() / 1.0e9);            fprintf(out, "Modified:  %.2f GB\n", getTotalModifiedMemory() / 1.0e9);            fprintf(out, "Compressed:%.2f GB\n", getCompressedMemorySize() / 1.0e9);            fprintf(out, "--- Physical Pages ---\n");            fprintf(out, "Active:  %llu\n", static_cast<unsigned long long>(pm.activePages));            fprintf(out, "Standby: %llu\n", static_cast<unsigned long long>(pm.standbyPages));            fprintf(out, "Modified:%llu\n", static_cast<unsigned long long>(pm.modifiedPages));            fprintf(out, "Free:    %llu\n", static_cast<unsigned long long>(pm.freePages));            fprintf(out, "Zero:    %llu\n", static_cast<unsigned long long>(pm.zeroPages));            fprintf(out, "Bad:     %llu\n", static_cast<unsigned long long>(pm.badPages));        }        fflush(out);        core.shutdown();        return 0;    }    if (headless) {        Logger::instance().info("[RAMFlux] Running in headless mode (--headless)");
std::signal(SIGINT, signalHandler);
std::signal(SIGTERM, signalHandler);
    while(g_running) {            std::this_thread::sleep_for(std::chrono::milliseconds(500));            QApplication::processEvents();        }    } else {        RAMFlux::UI::MainWindow mainWindow;
    QSettings s("RAMFlux", "RAMFlux");
bool startMinimized = s.value("startMinimized", false).toBool();
    if(!startMinimized) {            mainWindow.show();        }        app.exec();    }    core.shutdown();
    Logger::instance().info("[RAMFlux] Exiting");
    return 0;
}


