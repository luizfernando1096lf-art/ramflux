// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <unordered_set>
#include "core/IModule.h"
#include "shared/Constants.h"
namespace RAMFlux::LeakHunter {
struct LeakReport {    uint32_t pid{
0};
std::wstring processName;
uint64_t initialWorkingSet{
0};
uint64_t currentWorkingSet{
0};
uint64_t peakWorkingSet{
0};
uint64_t growthBytes{
0};
double growthPercent{
0.0};
int sampleCount{
0};
bool suspicious{
false};
std::chrono::steady_clock::time_point detectedAt;
};
struct ProcessMemoryHistory {    std::vector<uint64_t> samples;
std::chrono::steady_clock::time_point firstSeen;
uint64_t maxWorkingSet{
0};
uint64_t minWorkingSet{
UINT64_MAX};
std::wstring name;
};
class LeakHunter : public Core::IModule {
public:
    LeakHunter();    ~LeakHunter() override;
bool initialize() override;
void shutdown() override;
std::string name() const override;
std::vector<LeakReport> currentReports() const;
std::vector<LeakReport> getActiveLeaks(uint64_t minGrowthMB = 100);
void setThresholdMB(uint64_t mb);
void setEnabled(bool enabled);
bool isEnabled() const;
    private:
    void monitorLoop();    LeakReport analyzeProcess(uint32_t pid, const std::wstring& name, uint64_t currentWS);
std::thread m_thread;
std::atomic<bool> m_running{
false};
std::atomic<bool> m_enabled{
true};
std::atomic<uint64_t> m_thresholdMB{
100};
mutable std::mutex m_dataMutex;
std::map<uint32_t, ProcessMemoryHistory> m_history;
std::vector<LeakReport> m_reports;
};
} // namespace RAMFlux::LeakHunter


