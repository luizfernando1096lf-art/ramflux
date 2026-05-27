// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "FluxTelemetry.h"
#include "core/EventBus.h"
#include "shared/Constants.h"
#include <chrono>
#include <thread>
#include "core/Logger.h"
#include <exception>
namespace RAMFlux::Telemetry {
using RAMFlux::Core::Logger;
FluxTelemetry::FluxTelemetry()    : m_collector(std::make_unique<MemoryCollector>()) {}
FluxTelemetry::~FluxTelemetry() {    shutdown();
}
bool FluxTelemetry::initialize() {    Logger::instance().info("[FluxTelemetry] Initializing...");    m_running = true;    m_thread = std::thread(&FluxTelemetry::pollingLoop, this);
    Logger::instance().info("[FluxTelemetry] Telemetry  thread started");
    return true;
}
void FluxTelemetry::shutdown() {
if(m_running.exchange(false)) {
if(m_thread.joinable()) {            m_thread.join();        }        Logger::instance().info("[FluxTelemetry] Shutdown complete");    }}
std::string FluxTelemetry::name() const {
return "FluxTelemetry";
}
void FluxTelemetry::pollingLoop() {
while(m_running) {
try {            auto snapshot = m_collector->collect();            {                std::lock_guard<std::mutex> lock(m_snapshotMutex);                m_lastSnapshot = snapshot;            }            Core::EventBus::instance().post(Constants::EventType::MemoryUpdated);
    if(snapshot.pressureScore >= Constants::PRESSURE_CRITICAL_MAX) {                Core::EventBus::instance().post(Constants::EventType::PressureCritical);            } else if(snapshot.pressureScore >= Constants::PRESSURE_HIGH_MAX) {                Core::EventBus::instance().post(Constants::EventType::PressureHigh);            }        } catch (const std::exception& e) {            Logger::instance().error(std::string("[FluxTelemetry] Error: ") + e.what());        } catch (...) {            Logger::instance().error("[FluxTelemetry] Unknown error");        }        std::this_thread::sleep_for(std::chrono::milliseconds(m_pollingIntervalMs.load()));    }}
MemorySnapshot FluxTelemetry::lastSnapshot() const {    std::lock_guard<std::mutex> lock(m_snapshotMutex);
    return m_lastSnapshot;
}
void FluxTelemetry::setPollingInterval(int ms) {    m_pollingIntervalMs.store(std::max(100, ms));
}} // namespace RAMFlux::Telemetry 