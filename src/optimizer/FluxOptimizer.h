// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <memory>
#include "core/IModule.h"
#include "telemetry/MemorySnapshot.h"
#include "shared/Constants.h"
namespace RAMFlux::Optimizer {
class FluxOptimizer : public Core::IModule {
public:
    FluxOptimizer();    ~FluxOptimizer() override = default;
bool initialize() override;
void shutdown() override;
std::string name() const override;    Constants::PressureLevel evaluatePressure(Telemetry::MemorySnapshot& snap);
bool shouldOptimize(Telemetry::MemorySnapshot& snap);    Constants::PressureLevel currentPressure() const;
double calculatePressureScore(const Telemetry::MemorySnapshot& snap);
    private:
    Constants::PressureLevel m_currentPressure{
Constants::PressureLevel::Idle};
};
} // namespace RAMFlux::Optimizer


