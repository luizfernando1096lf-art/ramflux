// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include "MemorySnapshot.h"
namespace RAMFlux::Telemetry {
class MemoryCollector {
public:
    MemoryCollector();    MemorySnapshot collect();
    private:
    uint64_t m_prevIdleTime{
0};
uint64_t m_prevKernelTime{
0};
uint64_t m_prevUserTime{
0};
std::chrono::steady_clock::time_point m_lastCpuTime;
};
} // namespace RAMFlux::Telemetry 