// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include "core/IModule.h"
namespace RAMFlux::Analyzer {
struct ProcessInfo {    uint32_t pid{
0};
std::wstring name;
uint64_t workingSet{
0};
uint64_t peakWorkingSet{
0};
uint64_t privateUsage{
0};
uint32_t handleCount{
0};
uint32_t threadCount{
0};
uint64_t pageFaults{
0};
double cpuPercent{
0.0};
uint64_t pageFileUsage{
0};
};
class FluxProcessAnalyzer : public Core::IModule {
public:
    FluxProcessAnalyzer() = default;    ~FluxProcessAnalyzer() override = default;
bool initialize() override;
void shutdown() override;
std::string name() const override;
std::vector<ProcessInfo> getTopProcesses(int count = 10);
std::vector<ProcessInfo> detectMemoryLeaks(uint64_t thresholdMB = 500);
bool trimProcess(uint32_t pid);    ProcessInfo getProcessInfo(uint32_t pid);
    private:
    uint64_t calculateCpuUsage(uint32_t pid);
bool m_initialized{
false};
struct CpuSample {        uint64_t kernel{
0};
uint64_t user{
0};
uint64_t time{
0};    };
std::map<uint32_t, CpuSample> m_cpuSamples;
};
} // namespace RAMFlux::Analyzer


