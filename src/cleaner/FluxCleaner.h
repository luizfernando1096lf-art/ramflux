// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <atomic>
#include <chrono>
#include <mutex>
#include <cstdint>
#include "core/IModule.h"
#include "shared/Constants.h"
namespace RAMFlux::Telemetry {
struct MemorySnapshot; }
namespace RAMFlux::Cleaner {
struct CleanerStats {    uint64_t bytesRecovered{
0};
int standbyCleanCount{
0};
int trimCount{
0};
int deepCleanCount{
0};
int modifiedCleanCount{
0};
int thresholdCleanCount{
0};
std::chrono::steady_clock::time_point lastCleanTime;
uint64_t lastRecoveredBytes{
0};
};
class FluxCleaner : public Core::IModule {
public:
    FluxCleaner();    ~FluxCleaner() override = default;
bool initialize() override;
void shutdown() override;
std::string name() const override;
    enum Area : uint64_t {        Standby      = 1ULL << 0,        Modified     = 1ULL << 1,        WorkingSet   = 1ULL << 2,        FileCache    = 1ULL << 3,        Combined     = 1ULL << 4,        Defrag       = 1ULL << 5,    };
    bool cleanStandbyList();
    bool cleanModifiedPageList();
    bool trimProcesses();
    bool trimProcess(uint32_t pid);
    bool cleanWorkingSet();
    bool cleanFileCache();
    bool cleanCombinedList();
    bool defragment();
    bool quickClean();
    bool deepClean(uint64_t areas = Standby | Modified | WorkingSet | FileCache | Defrag);
bool adaptiveClean(const Telemetry::MemorySnapshot& snap);
bool canClean() const;    CleanerStats stats() const;
void setCooldownMs(int ms);
int cooldownMs() const;
    void setStandbyThresholdMB(uint64_t mb);
    uint64_t standbyThresholdMB() const;
    void setFreeMemThresholdMB(uint64_t mb);
    uint64_t freeMemThresholdMB() const;
    void setEnabledAreas(uint64_t flags);
    uint64_t enabledAreas() const;
    void resetStats();
    private:
    std::atomic<int> m_cooldownMs{
30000};
std::atomic<int64_t> m_lastCleanTimeEpoch{
0};
mutable std::mutex m_statsMutex;    CleanerStats m_stats;
std::atomic<bool> m_initialized{
false};
    std::atomic<uint64_t> m_standbyThresholdMB{
    Constants::DEFAULT_STANDBY_THRESHOLD_MB};
    std::atomic<uint64_t> m_freeMemThresholdMB{
    Constants::DEFAULT_FREE_MEM_THRESHOLD_MB};
    std::atomic<uint64_t> m_enabledAreas{
    Area::Standby | Area::Modified | Area::WorkingSet};
};
} // namespace RAMFlux::Cleaner


