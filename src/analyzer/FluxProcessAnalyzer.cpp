// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "FluxProcessAnalyzer.h"
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <algorithm>
#include "core/Logger.h"
#include <chrono>
namespace RAMFlux::Analyzer {
using RAMFlux::Core::Logger;
bool FluxProcessAnalyzer::initialize() {    Logger::instance().info("[FluxProcessAnalyzer] Initializing...");    m_initialized = true;
    return true;
}
void FluxProcessAnalyzer::shutdown() {    m_initialized = false;    m_cpuSamples.clear();
    Logger::instance().info("[FluxProcessAnalyzer] Shutdown complete");
}
std::string FluxProcessAnalyzer::name() const {
return "FluxProcessAnalyzer";
}
uint64_t FluxProcessAnalyzer::calculateCpuUsage(uint32_t pid) {    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if(!hProcess)
return 0;    FILETIME createTime, exitTime, kernelTime, userTime;
    if(!GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime)) {        CloseHandle(hProcess);
    return 0;    }    CloseHandle(hProcess);
uint64_t kernel = (static_cast<uint64_t>(kernelTime.dwHighDateTime) << 32) | kernelTime.dwLowDateTime;
uint64_t user = (static_cast<uint64_t>(userTime.dwHighDateTime) << 32) | userTime.dwLowDateTime;
    auto now = std::chrono::steady_clock::now();
uint64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(        now.time_since_epoch()).count();
    auto it = m_cpuSamples.find(pid);
    if(it == m_cpuSamples.end()) {        m_cpuSamples[pid] = {
kernel, user, nowMs};
return 0;    }    uint64_t kernelDelta = kernel - it->second.kernel;
uint64_t userDelta = user - it->second.user;
uint64_t timeDelta = nowMs - it->second.time;    it->second = {
kernel, user, nowMs};
if(timeDelta == 0)
return 0;
    return (kernelDelta + userDelta) * 100 / (timeDelta * 10000);
}
ProcessInfo FluxProcessAnalyzer::getProcessInfo(uint32_t pid) {    ProcessInfo info;    info.pid = pid;
HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if(hProcess) {        PROCESS_MEMORY_COUNTERS_EX pmc;
    if(GetProcessMemoryInfo(hProcess, reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&pmc), sizeof(pmc))) {            info.workingSet = pmc.WorkingSetSize;            info.peakWorkingSet = pmc.PeakWorkingSetSize;            info.pageFaults = pmc.PageFaultCount;            info.privateUsage = pmc.PrivateUsage;            info.pageFileUsage = pmc.PagefileUsage;        }        info.handleCount = 0;        info.threadCount = 0;        CloseHandle(hProcess);    }    info.cpuPercent = static_cast<double>(calculateCpuUsage(pid));
    return info;
}
std::vector<ProcessInfo> FluxProcessAnalyzer::getTopProcesses(int count) {    std::vector<ProcessInfo> processes;
HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hSnapshot == INVALID_HANDLE_VALUE)
return processes;    PROCESSENTRY32W pe32;    pe32.dwSize = sizeof(pe32);
    if(Process32FirstW(hSnapshot, &pe32)) {
do {            ProcessInfo info;            info.pid = pe32.th32ProcessID;            info.name = pe32.szExeFile;            info.threadCount = pe32.cntThreads;
HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
    if(hProcess) {                PROCESS_MEMORY_COUNTERS_EX pmc;
    if(GetProcessMemoryInfo(hProcess, reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&pmc), sizeof(pmc))) {                    info.workingSet = pmc.WorkingSetSize;                    info.peakWorkingSet = pmc.PeakWorkingSetSize;                    info.pageFaults = pmc.PageFaultCount;                    info.privateUsage = pmc.PrivateUsage;                    info.pageFileUsage = pmc.PagefileUsage;                }                info.handleCount = 0;                CloseHandle(hProcess);            }            info.cpuPercent = static_cast<double>(calculateCpuUsage(pe32.th32ProcessID));            processes.push_back(info);        } while (Process32NextW(hSnapshot, &pe32));    }    CloseHandle(hSnapshot);
std::sort(processes.begin(), processes.end(),        [](const ProcessInfo& a, const ProcessInfo& b) {
return a.workingSet > b.workingSet;        }
);
    if(static_cast<int>(processes.size()) > count) {        processes.resize(count);    }
return processes;
}
std::vector<ProcessInfo> FluxProcessAnalyzer::detectMemoryLeaks(uint64_t thresholdMB) {    auto processes = getTopProcesses(50);
std::vector<ProcessInfo> leaks;
uint64_t thresholdBytes = thresholdMB * 1024 * 1024;
    for(const auto& p : processes) {
if(p.workingSet > thresholdBytes) {            leaks.push_back(p);        }    }
return leaks;
}
bool FluxProcessAnalyzer::trimProcess(uint32_t pid) {    HANDLE hProcess = OpenProcess(PROCESS_SET_QUOTA | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if(!hProcess)
return false;
BOOL result = SetProcessWorkingSetSize(hProcess, (SIZE_T)-1, (SIZE_T)-1);    CloseHandle(hProcess);
    return result != FALSE;
}} // namespace RAMFlux::Analyzer


