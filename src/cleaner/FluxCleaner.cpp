// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "FluxCleaner.h"
#include "ntapi/FluxNTAPI.h"
#include "core/EventBus.h"
#include "telemetry/MemorySnapshot.h"
#include "core/Logger.h"
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
bool FluxCleaner::cleanStandbyList() {
if(!canClean())
return false;    Core::EventBus::instance().post(Constants::EventType::CleaningStarted);
uint64_t before = NtApi::getStandbyMemorySize();
bool success = NtApi::clearStandbyList();
uint64_t after = NtApi::getStandbyMemorySize();
    if(success) {        uint64_t recovered = (before > after) ? (before - after) : 0;        {            std::lock_guard<std::mutex> lock(m_statsMutex);            m_stats.bytesRecovered += recovered;            m_stats.standbyCleanCount++;            m_stats.lastRecoveredBytes = recovered;            m_stats.lastCleanTime = std::chrono::steady_clock::now();        }        m_lastCleanTimeEpoch.store(std::chrono::duration_cast<std::chrono::milliseconds>(            std::chrono::steady_clock::now().time_since_epoch()).count());
    Logger::instance().info(std::string("[FluxCleaner] Standby cleaned, recovered ")            + std::to_string(recovered / (1024 * 1024)) + " MB");    }    Core::EventBus::instance().post(Constants::EventType::CleaningFinished);
    return success;
}
bool FluxCleaner::cleanModifiedPageList() {
if(!canClean())
return false;    Core::EventBus::instance().post(Constants::EventType::CleaningStarted);
uint64_t before = NtApi::getTotalModifiedMemory();
bool success = NtApi::clearModifiedPageList();
uint64_t after = NtApi::getTotalModifiedMemory();
    if(success) {        uint64_t recovered = (before > after) ? (before - after) : 0;        {            std::lock_guard<std::mutex> lock(m_statsMutex);            m_stats.bytesRecovered += recovered;            m_stats.modifiedCleanCount++;            m_stats.lastRecoveredBytes = recovered;            m_stats.lastCleanTime = std::chrono::steady_clock::now();        }        m_lastCleanTimeEpoch.store(std::chrono::duration_cast<std::chrono::milliseconds>(            std::chrono::steady_clock::now().time_since_epoch()).count());
    Logger::instance().info(std::string("[FluxCleaner] Modified page list cleaned, recovered ")            + std::to_string(recovered / (1024 * 1024)) + " MB");    }    Core::EventBus::instance().post(Constants::EventType::CleaningFinished);
    return success;
}
bool FluxCleaner::trimProcesses() {
if(!canClean())
return false;    Core::EventBus::instance().post(Constants::EventType::CleaningStarted);
bool success = NtApi::trimAllProcesses();
    if(success) {        {            std::lock_guard<std::mutex> lock(m_statsMutex);            m_stats.trimCount++;            m_stats.lastCleanTime = std::chrono::steady_clock::now();        }        m_lastCleanTimeEpoch.store(std::chrono::duration_cast<std::chrono::milliseconds>(            std::chrono::steady_clock::now().time_since_epoch()).count());
    Logger::instance().info("[FluxCleaner] Process working sets trimmed");    }    Core::EventBus::instance().post(Constants::EventType::CleaningFinished);
    return success;
}
bool FluxCleaner::trimProcess(uint32_t pid) {
return NtApi::trimProcessWorkingSet(pid);
}
bool FluxCleaner::cleanWorkingSet() {
if(!canClean())
return false;    Core::EventBus::instance().post(Constants::EventType::CleaningStarted);    trimProcesses();    Core::EventBus::instance().post(Constants::EventType::CleaningFinished);
    return true;
}
bool FluxCleaner::cleanFileCache() {
if(!canClean())
return false;    Core::EventBus::instance().post(Constants::EventType::CleaningStarted);    bool success = NtApi::clearSystemFileCache();    Core::EventBus::instance().post(Constants::EventType::CleaningFinished);
    return success;
}
bool FluxCleaner::defragment() {
if(!canClean())
return false;    Core::EventBus::instance().post(Constants::EventType::CleaningStarted);
    // Step 1: combine identical memory pages across processes (kernel-level defrag)
    bool ok = NtApi::defragmentMemory();
    // Step 2: trim working sets, largest processes first
    trimProcesses();
    {        std::lock_guard<std::mutex> lock(m_statsMutex);        m_stats.deepCleanCount++;        m_stats.lastCleanTime = std::chrono::steady_clock::now();    }    m_lastCleanTimeEpoch.store(std::chrono::duration_cast<std::chrono::milliseconds>(        std::chrono::steady_clock::now().time_since_epoch()).count());
    Logger::instance().info("[FluxCleaner] Memory defragmentation completed");    Core::EventBus::instance().post(Constants::EventType::CleaningFinished);
    return ok;
}
bool FluxCleaner::cleanCombinedList() {
if(!canClean())
return false;    Core::EventBus::instance().post(Constants::EventType::CleaningStarted);    bool success = NtApi::clearCombinedPageList();    Core::EventBus::instance().post(Constants::EventType::CleaningFinished);
    return success;
}
bool FluxCleaner::quickClean() {    if(!canClean())        return false;    Core::EventBus::instance().post(Constants::EventType::CleaningStarted);    bool ok = true;    ok = cleanStandbyList() && ok;    ok = trimProcesses() && ok;    if(ok) {        std::lock_guard<std::mutex> lock(m_statsMutex);        m_stats.thresholdCleanCount++;        m_stats.lastCleanTime = std::chrono::steady_clock::now();    }    Core::EventBus::instance().post(Constants::EventType::CleaningFinished);    return ok;
}
bool FluxCleaner::deepClean(uint64_t areas) {
if(!canClean())
return false;    Core::EventBus::instance().post(Constants::EventType::CleaningStarted);
    uint64_t activeAreas = areas & m_enabledAreas.load();
    if(activeAreas & Area::Standby) cleanStandbyList();
    if(activeAreas & Area::Modified) cleanModifiedPageList();
    if(activeAreas & Area::WorkingSet) trimProcesses();
    if(activeAreas & Area::FileCache) cleanFileCache();
    if(activeAreas & Area::Combined) cleanCombinedList();
    if(activeAreas & Area::Defrag) defragment();
    {        std::lock_guard<std::mutex> lock(m_statsMutex);        m_stats.deepCleanCount++;    }    Core::EventBus::instance().post(Constants::EventType::CleaningFinished);
    return true;
}
bool FluxCleaner::adaptiveClean(const Telemetry::MemorySnapshot& snap) {
if(!canClean())
return false;
    if(snap.pressureScore >= Constants::PRESSURE_CRITICAL_MAX) {        Logger::instance().info("[FluxCleaner] Critical pressure, performing deep clean");
    return deepClean();    }    if (snap.pressureScore >= Constants::PRESSURE_HIGH_MAX) {        Logger::instance().info("[FluxCleaner] High pressure, cleaning standby list");
    return cleanStandbyList();    }    uint64_t standbyMB = snap.standbyMemory / (1024 * 1024);
uint64_t freeMB = snap.freeRam / (1024 * 1024);
uint64_t thresholdMB = m_standbyThresholdMB.load();
uint64_t freeThresholdMB = m_freeMemThresholdMB.load();
    if(standbyMB > thresholdMB && freeMB < freeThresholdMB) {        Logger::instance().info(std::string("[FluxCleaner] Threshold triggered: standby=")            + std::to_string(standbyMB) + "MB > " + std::to_string(thresholdMB) + "MB"            + " and free=" + std::to_string(freeMB) + "MB < " + std::to_string(freeThresholdMB) + "MB");        {            std::lock_guard<std::mutex> lock(m_statsMutex);            m_stats.thresholdCleanCount++;        }        Core::EventBus::instance().post(Constants::EventType::ThresholdCleanTriggered);
    return cleanStandbyList();    }    if (snap.hardFaultsPerSec > static_cast<uint64_t>(Constants::DEFAULT_HARD_FAULT_THRESHOLD)        && snap.standbyMemory > 256ULL * 1024 * 1024) {        Logger::instance().info("[FluxCleaner] High page faults with large standby, cleaning");
    return cleanStandbyList();    }
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


