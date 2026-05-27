// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "FluxScheduler.h"
#include "core/EventBus.h"
#include "core/FluxCore.h"
#include "cleaner/FluxCleaner.h"
#include "optimizer/FluxOptimizer.h"
#include "telemetry/FluxTelemetry.h"
#include "ntapi/FluxNTAPI.h"
#include "core/Logger.h"
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <chrono>
#include <thread>
#include <algorithm>
#include <exception>
namespace RAMFlux::Scheduler {
using RAMFlux::Core::Logger;
FluxScheduler::FluxScheduler() {    m_lastScheduleClean = std::chrono::steady_clock::now();
}
FluxScheduler::~FluxScheduler() {    shutdown();
}
bool FluxScheduler::initialize() {    Logger::instance().info("[FluxScheduler] Initializing...");    m_running = true;    m_thread = std::thread(&FluxScheduler::schedulerLoop, this);
    Logger::instance().info("[FluxScheduler] Scheduler thread started");
    return true;
}
void FluxScheduler::shutdown() {
if(m_running.exchange(false)) {
if(m_thread.joinable()) {            m_thread.join();        }        Logger::instance().info("[FluxScheduler] Shutdown complete");    }}
std::string FluxScheduler::name() const {
return "FluxScheduler";
}
void FluxScheduler::schedulerLoop() {
while(m_running) {
try {
if(m_automationEnabled) {                auto& core = Core::FluxCore::instance();
    auto* telemetry = dynamic_cast<Telemetry::FluxTelemetry*>(                    core.moduleManager().getModule("FluxTelemetry"));
    auto* optimizer = dynamic_cast<Optimizer::FluxOptimizer*>(                    core.moduleManager().getModule("FluxOptimizer"));
    auto* cleaner = dynamic_cast<Cleaner::FluxCleaner*>(                    core.moduleManager().getModule("FluxCleaner"));
    if(telemetry && optimizer && cleaner) {                    auto snap = telemetry->lastSnapshot();
    if(optimizer->shouldOptimize(snap) && cleaner->canClean()) {
if(snap.pressureScore >= Constants::PRESSURE_CRITICAL_MAX) {                            cleaner->deepClean();
    Logger::instance().info("[FluxScheduler] Deep clean triggered");                        } else {                            bool cleaned = cleaner->adaptiveClean(snap);
    if(cleaned) {                                Logger::instance().info("[FluxScheduler] Adaptive clean triggered");                            }                        }                    }                }            }            if(m_scheduleEnabled) {                auto now = std::chrono::steady_clock::now();                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastScheduleClean).count();                if(elapsed >= m_scheduleIntervalMs.load()) {                    m_lastScheduleClean = now;                    auto* cleaner = dynamic_cast<Cleaner::FluxCleaner*>(                        Core::FluxCore::instance().moduleManager().getModule("FluxCleaner"));                    if(cleaner && cleaner->canClean()) {                        cleaner->deepClean();                        Logger::instance().info("[FluxScheduler] Scheduled clean triggered");                    }                }            }            if(m_proBalanceEnabled) {                applyProBalance();            }        } catch (const std::exception& e) {            Logger::instance().error(std::string("[FluxScheduler] Error: ") + e.what());        } catch (...) {            Logger::instance().error("[FluxScheduler] Unknown error");        }        std::this_thread::sleep_for(std::chrono::milliseconds(m_intervalMs.load()));    }}
void FluxScheduler::setAutomationEnabled(bool enabled) {    m_automationEnabled.store(enabled);
    Logger::instance().info(std::string("[FluxScheduler] Automation ")        + (enabled ? "enabled" : "disabled"));
}
bool FluxScheduler::isAutomationEnabled() const {
return m_automationEnabled.load();
}
void FluxScheduler::setIntervalMs(int ms) {    m_intervalMs.store(std::max(1000, ms));
}
void FluxScheduler::setProBalanceEnabled(bool enabled) {    m_proBalanceEnabled.store(enabled);    if(!enabled) {        for(auto& [pid, originalPc] : m_restoredPriorities) {            NtApi::setProcessPriority(pid, originalPc);        }        m_restoredPriorities.clear();    }    Logger::instance().info(std::string("[FluxScheduler] ProBalance ")        + (enabled ? "enabled" : "disabled"));
}
bool FluxScheduler::isProBalanceEnabled() const {
return m_proBalanceEnabled.load();
}
void FluxScheduler::setProBalanceMemoryMB(uint64_t mb) {    m_proBalanceMemMB.store(mb);
}
void FluxScheduler::setScheduleIntervalMs(int ms) {    m_scheduleIntervalMs.store(std::max(60000, ms));
}
int FluxScheduler::scheduleIntervalMs() const {    return m_scheduleIntervalMs.load();
}
void FluxScheduler::setScheduleEnabled(bool enabled) {    m_scheduleEnabled.store(enabled);    if(enabled) {        m_lastScheduleClean = std::chrono::steady_clock::now();    }
}
bool FluxScheduler::isScheduleEnabled() const {    return m_scheduleEnabled.load();
}
void FluxScheduler::applyProBalance() {    uint64_t thresholdBytes = m_proBalanceMemMB.load() * 1024ULL * 1024ULL;    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);    if(hSnapshot == INVALID_HANDLE_VALUE) return;    std::unordered_map<uint32_t, uint64_t> procMem;    PROCESSENTRY32W pe32;    pe32.dwSize = sizeof(pe32);    if(Process32FirstW(hSnapshot, &pe32)) {        do {            uint64_t ws = NtApi::getWorkingSetSize(pe32.th32ProcessID);            if(ws > thresholdBytes) {                procMem[pe32.th32ProcessID] = ws;            }        } while (Process32NextW(hSnapshot, &pe32));    }    CloseHandle(hSnapshot);    for(auto& [pid, mem] : procMem) {                uint32_t currentPc = NtApi::getProcessPriority(pid);
        // PRIORITY_CLASS constants are bitfield-style, not monotonic.
        // Only lower NORMAL/ABOVE_NORMAL/HIGH to BELOW_NORMAL.
        if(currentPc == NORMAL_PRIORITY_CLASS || currentPc == ABOVE_NORMAL_PRIORITY_CLASS || currentPc == HIGH_PRIORITY_CLASS) {            if(m_restoredPriorities.find(pid) == m_restoredPriorities.end()) {                m_restoredPriorities[pid] = currentPc;            }            NtApi::setProcessPriority(pid, BELOW_NORMAL_PRIORITY_CLASS);            Logger::instance().info(std::string("[FluxScheduler] ProBalance: lowered PID ") + std::to_string(pid) + " with " + std::to_string(mem / (1024*1024)) + " MB");        }    }    std::vector<uint32_t> toRestore;    for(auto& [pid, originalPc] : m_restoredPriorities) {        if(procMem.find(pid) == procMem.end()) {            NtApi::setProcessPriority(pid, originalPc);            toRestore.push_back(pid);        }    }    for(uint32_t pid : toRestore) {        m_restoredPriorities.erase(pid);    }
}
} // namespace RAMFlux::Scheduler
