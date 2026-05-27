// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <atomic>
#include <thread>
#include <memory>
#include "core/IModule.h"
#include "MemoryCollector.h"
#include "MemorySnapshot.h"
namespace RAMFlux::Telemetry {
class FluxTelemetry : public Core::IModule {
public:
    FluxTelemetry();    ~FluxTelemetry() override;
bool initialize() override;
void shutdown() override;
std::string name() const override;    MemorySnapshot lastSnapshot() const;
void setPollingInterval(int ms);
    private:
    void pollingLoop();
std::unique_ptr<MemoryCollector> m_collector;
std::thread m_thread;    mutable std::mutex m_snapshotMutex;    MemorySnapshot m_lastSnapshot;
std::atomic<bool> m_running{
false};
std::atomic<int> m_pollingIntervalMs{
1000};
};
} // namespace RAMFlux::Telemetry 