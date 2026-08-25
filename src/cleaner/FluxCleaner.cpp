// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "FluxCleaner.h"
#include "ntapi/FluxNTAPI.h"
#include "core/EventBus.h"
#include "telemetry/MemorySnapshot.h"
#include "core/Logger.h"
#include "helper/HelperClient.h"
#include "process/ProcessCache.h"
#include <windows.h>
#include <algorithm>
#include <cstdint>
namespace RAMFlux::Cleaner {
using RAMFlux::Core::Logger;
FluxCleaner::FluxCleaner() {    m_lastCleanTimeEpoch.store(std::chrono::duration_cast<std::chrono::milliseconds>(        std::chrono::steady_clock::now().time_since_epoch()).count());
}
bool FluxCleaner::initialize() {    Logger::instance().info("[FluxCleaner] Initializing...");    m_initialized = true;
    return true;
}
void FluxCleaner::shutdown() {    m_initialized = false;
    Logger::instance().info("[FluxCleaner] Shutdown complete");
}
std::string FluxCleaner::name() const {
return "FluxCleaner";
}
bool FluxCleaner::cleanStandbyList(bool force) {
if(!force && !canClean())
return false;
    double diskQueue = NtApi::getDiskQueueLength();
    uint64_t standbySize = NtApi::getStandbyMemorySize();
    if(diskQueue > Constants::IO_DISK_QUEUE_SKIP_THRESHOLD) {
        Logger::instance().info(std::string("[FluxCleaner] Skip standby: diskQ=")
            + std::to_string(diskQueue) + ", standby=" + std::to_string(standbySize / (1024*1024)) + "MB");
        return false;
    }
    if(standbySize < Constants::IO_STANDBY_MIN_CLEAN_BYTES) {
        Logger::instance().info(std::string("[FluxCleaner] Skip standby: ")
            + std::to_string(standbySize / (1024*1024)) + "MB < min "
            + std::to_string(Constants::IO_STANDBY_MIN_CLEAN_BYTES / (1024*1024)) + "MB");
        return false;
    }
Core::EventBus::instance().post(Constants::EventType::CleaningStarted);
uint64_t before = standbySize;
bool success = false;
success = Helper::sendCommand(Helper::Command::Standby);
if(!success) success = NtApi::clearStandbyList();
uint64_t after = NtApi::getStandbyMemorySize();
    if(success) {        uint64_t recovered = (before > after) ? (before - after) : 0;        {            std::lock_guard<std::mutex> lock(m_statsMutex);            m_stats.bytesRecovered += recovered;            m_stats.standbyCleanCount++;            m_stats.lastRecoveredBytes = recovered;            m_stats.lastCleanTime = std::chrono::steady_clock::now();        }        if(!force) m_lastCleanTimeEpoch.store(std::chrono::duration_cast<std::chrono::milliseconds>(            std::chrono::steady_clock::now().time_since_epoch()).count());
        Logger::instance().info(std::string("[FluxCleaner] Standby cleaned (diskQ=") + std::to_string(diskQueue)
            + "), recovered " + std::to_string(recovered / (1024 * 1024)) + " MB");
    }    Core::EventBus::instance().post(Constants::EventType::CleaningFinished);
    return success;
}
bool FluxCleaner::gentleStandbyClean() {
    if(!canClean()) return false;
    double diskQueue = NtApi::getDiskQueueLength();
    uint64_t standbySize = NtApi::getStandbyMemorySize();
    if(diskQueue > Constants::IO_DISK_QUEUE_SKIP_THRESHOLD) {
        Logger::instance().info(std::string("[FluxCleaner] Gentle skip: diskQ=")
            + std::to_string(diskQueue));
        return false;
    }
    if(standbySize < Constants::IO_STANDBY_MIN_CLEAN_BYTES) return false;
    Core::EventBus::instance().post(Constants::EventType::CleaningStarted);
    uint64_t before = standbySize;
    bool ok = NtApi::gentleStandbyClean();
    if(ok) {
        uint64_t after = NtApi::getStandbyMemorySize();
        uint64_t recovered = (before > after) ? (before - after) : 0;
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats.bytesRecovered += recovered;
        m_stats.standbyCleanCount++;
        m_stats.lastRecoveredBytes = recovered;
        m_stats.lastCleanTime = std::chrono::steady_clock::now();
        m_lastCleanTimeEpoch.store(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
        Logger::instance().info(std::string("[FluxCleaner] Gentle clean: ")
            + std::to_string(recovered / (1024*1024)) + "MB recovered in phases");
    }
    Core::EventBus::instance().post(Constants::EventType::CleaningFinished);
    return ok;
}
bool FluxCleaner::selectiveClean() {
    if(!canClean()) return false;
    double diskQueue = NtApi::getDiskQueueLength();
    if(diskQueue > Constants::IO_DISK_QUEUE_SKIP_THRESHOLD) {
        Logger::instance().info(std::string("[FluxCleaner] Skip selective: diskQ=")
            + std::to_string(diskQueue));
        return false;
    }
    Core::EventBus::instance().post(Constants::EventType::CleaningStarted);
    uint64_t standbyBefore = NtApi::getStandbyMemorySize();
    bool success = NtApi::selectiveStandbyClean(Constants::SL_STANDBY_PRIORITY_BELOW_NORMAL);
    if(success) {
        uint64_t standbyAfter = NtApi::getStandbyMemorySize();
        uint64_t recovered = (standbyBefore > standbyAfter) ? (standbyBefore - standbyAfter) : 0;
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats.standbyCleanCount++;
        m_stats.bytesRecovered += recovered;
        m_stats.lastRecoveredBytes = recovered;
        m_stats.lastCleanTime = std::chrono::steady_clock::now();
        m_lastCleanTimeEpoch.store(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
        Logger::instance().info("[FluxCleaner] Selective standby clean completed (diskQ="
            + std::to_string(diskQueue) + ")");
    }
    Core::EventBus::instance().post(Constants::EventType::CleaningFinished);
    return success;
}
bool FluxCleaner::intelligentStandbyClean(const Telemetry::MemorySnapshot& snap) {
    if(!canClean()) return false;
    double diskQueue = NtApi::getDiskQueueLength();
    if(diskQueue > Constants::IO_DISK_QUEUE_SKIP_THRESHOLD) {
        Logger::instance().info(std::string("[FluxCleaner] Intelligent standby skip: diskQ=")
            + std::to_string(diskQueue));
        return false;
    }
    uint32_t maxPrio;
    const char* prioName;
    if(snap.pressureScore < Constants::PRESSURE_NORMAL_MAX) {
        maxPrio = Constants::SL_STANDBY_PRIORITY_LOWEST;
        prioName = "LOWEST (1)";
    } else if(snap.pressureScore < Constants::PRESSURE_HIGH_MAX) {
        maxPrio = Constants::SL_STANDBY_PRIORITY_BELOW_NORMAL;
        prioName = "BELOW_NORMAL (2)";
    } else if(snap.pressureScore < 90) {
        maxPrio = Constants::SL_STANDBY_PRIORITY_NORMAL;
        prioName = "NORMAL (5)";
    } else {
        Logger::instance().info("[FluxCleaner] Intelligent: pressure critical, full standby clear");
        return cleanStandbyList(true);
    }
    Core::EventBus::instance().post(Constants::EventType::CleaningStarted);
    uint64_t before = NtApi::getStandbyMemorySize();
    bool success = NtApi::selectiveStandbyClean(maxPrio);
    uint64_t after = NtApi::getStandbyMemorySize();
    if(success) {
        uint64_t recovered = (before > after) ? (before - after) : 0;
        {
            std::lock_guard<std::mutex> lock(m_statsMutex);
            m_stats.standbyCleanCount++;
            m_stats.bytesRecovered += recovered;
            m_stats.lastRecoveredBytes = recovered;
            m_stats.lastCleanTime = std::chrono::steady_clock::now();
        }
        m_lastCleanTimeEpoch.store(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
        Logger::instance().info(std::string("[FluxCleaner] Intelligent standby (") + prioName
            + ", pressure=" + std::to_string(snap.pressureScore) + "): "
            + std::to_string(recovered / (1024*1024)) + " MB");
    }
    Core::EventBus::instance().post(Constants::EventType::CleaningFinished);
    return success;
}
bool FluxCleaner::cleanModifiedPageList(bool force) {
if(!force && !canClean())
return false;
    double diskQueue = NtApi::getDiskQueueLength();
    uint64_t modifiedSize = NtApi::getTotalModifiedMemory();
    if(diskQueue > Constants::IO_DISK_QUEUE_SKIP_THRESHOLD) {
        Logger::instance().info(std::string("[FluxCleaner] Skip modified: diskQ=")
            + std::to_string(diskQueue));
        return false;
    }
    if(modifiedSize < Constants::IO_MODIFIED_MIN_CLEAN_BYTES) {
        Logger::instance().info(std::string("[FluxCleaner] Skip modified: ")
            + std::to_string(modifiedSize / (1024*1024)) + "MB < min "
            + std::to_string(Constants::IO_MODIFIED_MIN_CLEAN_BYTES / (1024*1024)) + "MB");
        return false;
    }
Core::EventBus::instance().post(Constants::EventType::CleaningStarted);
uint64_t before = modifiedSize;
bool success = false;
if(Helper::isHelperRunning()) success = Helper::sendCommand(Helper::Command::Modified);
if(!success) success = NtApi::clearModifiedPageList();
uint64_t after = NtApi::getTotalModifiedMemory();
    if(success) {        uint64_t recovered = (before > after) ? (before - after) : 0;        {            std::lock_guard<std::mutex> lock(m_statsMutex);            m_stats.bytesRecovered += recovered;            m_stats.modifiedCleanCount++;            m_stats.lastRecoveredBytes = recovered;            m_stats.lastCleanTime = std::chrono::steady_clock::now();        }        if(!force) m_lastCleanTimeEpoch.store(std::chrono::duration_cast<std::chrono::milliseconds>(            std::chrono::steady_clock::now().time_since_epoch()).count());
        Logger::instance().info(std::string("[FluxCleaner] Modified cleaned (diskQ=") + std::to_string(diskQueue)
            + "), recovered " + std::to_string(recovered / (1024 * 1024)) + " MB");
    }    Core::EventBus::instance().post(Constants::EventType::CleaningFinished);
    return success;
}
bool FluxCleaner::trimProcesses(bool force) {
if(!force && !canClean())
return false;    Core::EventBus::instance().post(Constants::EventType::CleaningStarted);
bool success = false;
if(Helper::isHelperRunning()) success = Helper::sendCommand(Helper::Command::TrimAll);
if(!success) {
    auto now = std::chrono::steady_clock::now();
    auto entries = Process::ProcessCache::instance().processes();
    bool anyTrimmed = false;
    uint32_t selfPid = GetCurrentProcessId();
    for(const auto& e : entries) {
        if(e.pid == selfPid) continue;
        auto elapsed = std::chrono::duration<double>(now - e.lastWsChangeTime).count();
        if(elapsed < Constants::WS_AGE_IDLE_SECONDS) continue;
        if(NtApi::trimProcessWorkingSet(e.pid)) anyTrimmed = true;
    }
    success = anyTrimmed;
}
    if(success) {        {            std::lock_guard<std::mutex> lock(m_statsMutex);            m_stats.trimCount++;            m_stats.lastCleanTime = std::chrono::steady_clock::now();        }        if(!force) m_lastCleanTimeEpoch.store(std::chrono::duration_cast<std::chrono::milliseconds>(            std::chrono::steady_clock::now().time_since_epoch()).count());
    Logger::instance().info("[FluxCleaner] Process working sets trimmed (age-aware)");    }    Core::EventBus::instance().post(Constants::EventType::CleaningFinished);
    return success;
}
bool FluxCleaner::trimProcessesIdleOnly() {
    auto now = std::chrono::steady_clock::now();
    auto entries = Process::ProcessCache::instance().processes();
    bool anyTrimmed = false;
    uint32_t selfPid = GetCurrentProcessId();
    for(const auto& e : entries) {
        if(e.pid == selfPid) continue;
        auto elapsed = std::chrono::duration<double>(now - e.lastWsChangeTime).count();
        if(elapsed < Constants::WS_AGE_IDLE_SECONDS) continue;
        if(NtApi::trimProcessWorkingSet(e.pid)) anyTrimmed = true;
    }
    return anyTrimmed;
}
bool FluxCleaner::trimColdPages() {
    auto entries = Process::ProcessCache::instance().processes();
    bool anyTrimmed = false;
    uint64_t totalColdBefore = 0;
    uint32_t selfPid = GetCurrentProcessId();
    for(const auto& e : entries) {
        if(e.pid == selfPid || e.pid <= 4) continue;
        if(e.workingSet < 100ULL * 1024 * 1024) continue;
        if(e.totalSnapshotPages == 0 || e.coldPageBytes == 0) continue;
        double coldRatio = static_cast<double>(e.coldPageBytes) / e.workingSet;
        if(coldRatio < 0.3) continue;
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<double>(now - e.lastWsChangeTime).count();
        if(elapsed < Constants::WS_AGE_IDLE_SECONDS) continue;
        if(NtApi::trimProcessWorkingSet(e.pid)) {
            anyTrimmed = true;
            totalColdBefore += e.coldPageBytes;
        }
    }
    if(anyTrimmed) {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats.bytesRecovered += totalColdBefore;
        m_stats.trimCount++;
        m_stats.lastCleanTime = std::chrono::steady_clock::now();
        m_lastCleanTimeEpoch.store(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
        Logger::instance().info(std::string("[FluxCleaner] Cold page trim: ") + std::to_string(totalColdBefore / (1024*1024)) + " MB recovered");
    }
    return anyTrimmed;
}
bool FluxCleaner::trimProcess(uint32_t pid) {
bool ok = false;
if(Helper::isHelperRunning()) ok = Helper::trimProcess(pid);
if(!ok) ok = NtApi::trimProcessWorkingSet(pid);
return ok;
}
bool FluxCleaner::cleanWorkingSet() {
if(!canClean())
return false;    return trimProcesses();
}
bool FluxCleaner::cleanFileCache(bool force) {
if(!force && !canClean())
return false;
    double diskQueue = NtApi::getDiskQueueLength();
    if(diskQueue > Constants::IO_DISK_QUEUE_SKIP_THRESHOLD / 2) {
        Logger::instance().info(std::string("[FluxCleaner] Skip file cache: diskQ=")
            + std::to_string(diskQueue));
        return false;
    }
Core::EventBus::instance().post(Constants::EventType::CleaningStarted);
bool success = false;
if(Helper::isHelperRunning()) success = Helper::sendCommand(Helper::Command::FileCache);
if(!success) success = NtApi::clearSystemFileCache();
    if(success) {
        Logger::instance().warn(std::string("[FluxCleaner] File cache flushed"));
    }    Core::EventBus::instance().post(Constants::EventType::CleaningFinished);
return success;
}
bool FluxCleaner::defragment(bool force) {
if(!force && !canClean())
return false;    if(m_batteryAware.load()) {        auto ps = NtApi::getPowerStatus();        if(!ps.onAC) {            Logger::instance().info("[FluxCleaner] Skipping defrag: on battery");            return false;        }    }
bool ok = false;
if(Helper::isHelperRunning()) ok = Helper::sendCommand(Helper::Command::Defrag);
if(!ok) ok = NtApi::clearCombinedPageList();
trimProcesses();
    {        std::lock_guard<std::mutex> lock(m_statsMutex);        m_stats.deepCleanCount++;        m_stats.lastCleanTime = std::chrono::steady_clock::now();    }    m_lastCleanTimeEpoch.store(std::chrono::duration_cast<std::chrono::milliseconds>(        std::chrono::steady_clock::now().time_since_epoch()).count());
    Logger::instance().info("[FluxCleaner] Memory defragmentation completed");
    return ok;
}
bool FluxCleaner::cleanCombinedList(bool force) {
if(!force && !canClean())
return false;    Core::EventBus::instance().post(Constants::EventType::CleaningStarted);
bool success = false;
if(Helper::isHelperRunning()) success = Helper::sendCommand(Helper::Command::Combined);
if(!success) success = NtApi::clearCombinedPageList();    Core::EventBus::instance().post(Constants::EventType::CleaningFinished);
return success;
}
bool FluxCleaner::quickClean() {    if(!canClean())        return false;    bool ok = true;    double dq = NtApi::getDiskQueueLength();    if(dq <= Constants::IO_DISK_QUEUE_SKIP_THRESHOLD) {        ok = cleanStandbyList(true) && ok;    } else {        Logger::instance().info(std::string("[FluxCleaner] quickClean: skip standby (diskQ=")            + std::to_string(dq) + "), trim only");    }    ok = trimProcesses(true) && ok;    m_lastCleanTimeEpoch.store(std::chrono::duration_cast<std::chrono::milliseconds>(        std::chrono::steady_clock::now().time_since_epoch()).count());    if(ok) {        std::lock_guard<std::mutex> lock(m_statsMutex);        m_stats.thresholdCleanCount++;        m_stats.lastCleanTime = std::chrono::steady_clock::now();    }    return ok;
}
bool FluxCleaner::deepClean(uint64_t areas) {
if(!canClean())
return false;
    double dq = NtApi::getDiskQueueLength();
    if(dq > Constants::IO_DISK_QUEUE_SKIP_THRESHOLD) {
        Logger::instance().info(std::string("[FluxCleaner] deepClean: skip I/O areas (diskQ=")
            + std::to_string(dq) + "), cold trim only");
        trimColdPages();
        return true;
    }
    if(m_batteryAware.load()) {        auto ps = NtApi::getPowerStatus();        if(!ps.onAC) {            areas &= ~(Area::Defrag | Area::FileCache);            Logger::instance().info("[FluxCleaner] Battery: skipping defrag & file cache in deep clean");        }    }
    trimColdPages();
    uint64_t activeAreas = areas & m_enabledAreas.load();
    if(activeAreas & Area::Standby) cleanStandbyList(true);
    if(activeAreas & Area::Modified) cleanModifiedPageList(true);
    if((activeAreas & Area::WorkingSet) && !(activeAreas & Area::Defrag)) trimProcesses(true);
    if(activeAreas & Area::FileCache) cleanFileCache(true);
    if(activeAreas & Area::Combined) cleanCombinedList(true);
    if(activeAreas & Area::Defrag) defragment(true);
    m_lastCleanTimeEpoch.store(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
    {        std::lock_guard<std::mutex> lock(m_statsMutex);        m_stats.deepCleanCount++;    }
    return true;
}
bool FluxCleaner::prepareForGame() {
    if(!canClean()) return false;
    Logger::instance().info("[FluxCleaner] Preparing memory for game...");
    cleanStandbyList(true);
    cleanModifiedPageList(true);
    m_lastCleanTimeEpoch.store(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
    auto entries = Process::ProcessCache::instance().processes();
    auto now = std::chrono::steady_clock::now();
    uint32_t selfPid = GetCurrentProcessId();
    int trimmed = 0;
    uint64_t recovered = 0;
    for(const auto& e : entries) {
        if(e.pid == selfPid || e.pid <= 4) continue;
        if(e.workingSet < Constants::GM_PREGAME_PROCESS_TRIM_THRESHOLD) continue;
        auto age = std::chrono::duration<double>(now - e.lastWsChangeTime).count();
        if(age < Constants::GM_PREGAME_IDLE_SECONDS) continue;
        uint64_t wsBefore = e.workingSet;
        if(NtApi::trimProcessWorkingSet(e.pid)) {
            trimmed++;
            recovered += wsBefore;
        }
    }
    {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats.bytesRecovered += recovered;
        m_stats.trimCount += trimmed;
        m_stats.lastCleanTime = now;
    }
    Logger::instance().info(std::string("[FluxCleaner] Pre-game prep: ")
        + std::to_string(trimmed) + " processes trimmed ("
        + std::to_string(recovered / (1024*1024)) + "MB)");
    return true;
}
bool FluxCleaner::adaptiveClean(const Telemetry::MemorySnapshot& snap) {
if(!canClean())
return false;
    trimColdPages();
    if(snap.pressureScore >= Constants::PRESSURE_CRITICAL_MAX) {        Logger::instance().info("[FluxCleaner] Pressure CRITICAL, performing deep clean");
    return deepClean();    }
    if(snap.pressureScore >= Constants::PRESSURE_HIGH_MAX) {        Logger::instance().info("[FluxCleaner] Pressure >= High, quick cleaning");
    return quickClean();    }
    uint64_t freeMB = snap.freeRam / (1024 * 1024);
uint64_t freeThresholdMB = m_freeMemThresholdMB.load();
    if(freeMB < freeThresholdMB) {        Logger::instance().info(std::string("[FluxCleaner] Free memory low: ")            + std::to_string(freeMB) + "MB < " + std::to_string(freeThresholdMB) + "MB, quick cleaning");        {            std::lock_guard<std::mutex> lock(m_statsMutex);            m_stats.thresholdCleanCount++;        }        Core::EventBus::instance().post(Constants::EventType::ThresholdCleanTriggered);
    return quickClean();    }
    if (snap.hardFaultsPerSec > static_cast<uint64_t>(Constants::DEFAULT_HARD_FAULT_THRESHOLD)        && snap.standbyMemory > 256ULL * 1024 * 1024) {        Logger::instance().info("[FluxCleaner] High page faults with large standby, intelligent cleaning");
    return intelligentStandbyClean(snap);    }
return false;
}
bool FluxCleaner::canClean() const {    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(        std::chrono::steady_clock::now().time_since_epoch()).count();
    auto elapsed = now - m_lastCleanTimeEpoch.load();
    return elapsed >= m_cooldownMs.load();
}
CleanerStats FluxCleaner::stats() const {    std::lock_guard<std::mutex> lock(m_statsMutex);
    return m_stats;
}
void FluxCleaner::setCooldownMs(int ms) {    m_cooldownMs.store(std::max(1000, ms));
}
int FluxCleaner::cooldownMs() const {
return m_cooldownMs.load();
}
void FluxCleaner::setStandbyThresholdMB(uint64_t mb) {    m_standbyThresholdMB.store(mb);
}
uint64_t FluxCleaner::standbyThresholdMB() const {
return m_standbyThresholdMB.load();
}
void FluxCleaner::setFreeMemThresholdMB(uint64_t mb) {    m_freeMemThresholdMB.store(mb);
}
uint64_t FluxCleaner::freeMemThresholdMB() const {
return m_freeMemThresholdMB.load();
}
void FluxCleaner::setEnabledAreas(uint64_t flags) {    m_enabledAreas.store(flags);
}
uint64_t FluxCleaner::enabledAreas() const {
return m_enabledAreas.load();
}
void FluxCleaner::resetStats() {    std::lock_guard<std::mutex> lock(m_statsMutex);    m_stats = CleanerStats{}; }
} // namespace RAMFlux::Cleaner


