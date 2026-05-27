// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <atomic>
#include <memory>
#include <thread>
#include <unordered_map>
#include <cstdint>
#include "core/IModule.h"
#include "shared/Constants.h"
namespace RAMFlux::Scheduler {
class FluxScheduler : public Core::IModule {
public:
    FluxScheduler();    ~FluxScheduler() override;
bool initialize() override;
void shutdown() override;
std::string name() const override;
void setAutomationEnabled(bool enabled);
bool isAutomationEnabled() const;
    void setIntervalMs(int ms);
    void setProBalanceEnabled(bool enabled);
    bool isProBalanceEnabled() const;
    void setProBalanceMemoryMB(uint64_t mb);
    void setScheduleIntervalMs(int ms);
    int scheduleIntervalMs() const;
    void setScheduleEnabled(bool enabled);
    bool isScheduleEnabled() const;
    private:
    void schedulerLoop();
    void applyProBalance();
std::thread m_thread;
std::atomic<bool> m_running{
false};
std::atomic<bool> m_automationEnabled{
false};
std::atomic<int> m_intervalMs{
5000};
std::atomic<bool> m_proBalanceEnabled{false};
std::atomic<uint64_t> m_proBalanceMemMB{512};
std::atomic<bool> m_scheduleEnabled{false};
std::atomic<int> m_scheduleIntervalMs{3600000};
std::chrono::steady_clock::time_point m_lastScheduleClean;
std::unordered_map<uint32_t, uint32_t> m_restoredPriorities;
};
} // namespace RAMFlux::Scheduler


