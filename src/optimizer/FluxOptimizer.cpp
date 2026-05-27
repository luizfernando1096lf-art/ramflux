// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "FluxOptimizer.h"
#include "core/Logger.h"
#include <algorithm>
namespace RAMFlux::Optimizer {
using RAMFlux::Core::Logger;
FluxOptimizer::FluxOptimizer() = default;
bool FluxOptimizer::initialize() {    Logger::instance().info("[FluxOptimizer] Initializing...");
    return true;
}
void FluxOptimizer::shutdown() {    Logger::instance().info("[FluxOptimizer] Shutdown complete");
}
std::string FluxOptimizer::name() const {
return "FluxOptimizer";
}
double FluxOptimizer::calculatePressureScore(const Telemetry::MemorySnapshot& snap) {    double pressure = 0.0;
double memLoadFactor = static_cast<double>(snap.memoryLoad) * 0.30;    pressure += memLoadFactor;
double commitRatio = (snap.commitLimit > 0)        ? (static_cast<double>(snap.committedMemory) / snap.commitLimit) * 100.0        : 0.0;    pressure += commitRatio * 0.15;
double standbyGB = snap.standbyMemory / static_cast<double>(1024ULL * 1024 * 1024);
double standbyFactor = std::min(1.0, standbyGB / 8.0);    pressure += standbyFactor * 100.0 * 0.10;
double cacheRatio = (snap.totalRam > 0)        ? (static_cast<double>(snap.cachedMemory) / snap.totalRam) * 100.0        : 0.0;    pressure += cacheRatio * 0.05;
double faultFactor = std::min(1.0, static_cast<double>(snap.hardFaultsPerSec) / 500.0);    pressure += faultFactor * 100.0 * 0.10;
double pageFileRatio = (snap.totalPageFile > 0)        ? (static_cast<double>(snap.usedPageFile) / snap.totalPageFile) * 100.0        : 0.0;    pressure += pageFileRatio * 0.10;
double compressedGB = snap.compressedRamGB();
double compressionFactor = std::min(1.0, compressedGB / 4.0);    pressure += compressionFactor * 100.0 * 0.08;
double faultTrendFactor = std::clamp(snap.pageFaultTrend * 10.0, -5.0, 15.0);    pressure += faultTrendFactor;
double freeMemGB = snap.freeRamGB();
    if(freeMemGB < 1.0) {        pressure += 10.0;    } else
if(freeMemGB < 2.0) {        pressure += 5.0;    }    if (snap.standbyMemory > 2ULL * 1024 * 1024 * 1024 && freeMemGB < 2.0) {        pressure += 8.0;    }
return std::clamp(pressure, 0.0, 100.0);
}
Constants::PressureLevel FluxOptimizer::evaluatePressure(Telemetry::MemorySnapshot& snap) {    double score = calculatePressureScore(snap);    snap.pressureScore = static_cast<uint64_t>(score);    Constants::PressureLevel level;
    if(score <= Constants::PRESSURE_IDLE_MAX) level = Constants::PressureLevel::Idle;    else
if(score <= Constants::PRESSURE_NORMAL_MAX) level = Constants::PressureLevel::Normal;    else
if(score <= Constants::PRESSURE_HIGH_MAX) level = Constants::PressureLevel::High;    else level = Constants::PressureLevel::Critical;    m_currentPressure = level;
    return level;
}
bool FluxOptimizer::shouldOptimize(Telemetry::MemorySnapshot& snap) {    auto level = evaluatePressure(snap);
    return level >= Constants::PressureLevel::High;
}
Constants::PressureLevel FluxOptimizer::currentPressure() const {
return m_currentPressure;
}} // namespace RAMFlux::Optimizer


