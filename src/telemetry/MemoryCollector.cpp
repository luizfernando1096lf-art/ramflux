// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "MemoryCollector.h"
#include "ntapi/FluxNTAPI.h"
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <algorithm>
namespace RAMFlux::Telemetry {
MemoryCollector::MemoryCollector() {    m_lastCpuTime = std::chrono::steady_clock::now();
}
MemorySnapshot MemoryCollector::collect() {    MemorySnapshot snap;    snap.timestamp = std::chrono::steady_clock::now();    MEMORYSTATUSEX memStatus;    memStatus.dwLength = sizeof(memStatus);
    if(GlobalMemoryStatusEx(&memStatus)) {        snap.totalRam = memStatus.ullTotalPhys;        snap.usedRam = memStatus.ullTotalPhys - memStatus.ullAvailPhys;        snap.freeRam = memStatus.ullAvailPhys;        snap.memoryLoad = memStatus.dwMemoryLoad;        snap.totalVirtual = memStatus.ullTotalVirtual;        snap.usedVirtual = memStatus.ullTotalVirtual - memStatus.ullAvailVirtual;        snap.totalPageFile = memStatus.ullTotalPageFile;        snap.usedPageFile = memStatus.ullTotalPageFile - memStatus.ullAvailPageFile;    }    PERFORMANCE_INFORMATION perfInfo{};
perfInfo.cb = sizeof(perfInfo);
    if(GetPerformanceInfo(&perfInfo, sizeof(perfInfo))) {        snap.commitLimit = static_cast<uint64_t>(perfInfo.CommitLimit) * perfInfo.PageSize;        snap.committedMemory = static_cast<uint64_t>(perfInfo.CommitTotal) * perfInfo.PageSize;        snap.cachedMemory = static_cast<uint64_t>(perfInfo.SystemCache) * perfInfo.PageSize;        snap.kernelMemory = static_cast<uint64_t>(perfInfo.KernelTotal) * perfInfo.PageSize;        snap.kernelPaged = static_cast<uint64_t>(perfInfo.KernelPaged) * perfInfo.PageSize;        snap.kernelNonpaged = static_cast<uint64_t>(perfInfo.KernelNonpaged) * perfInfo.PageSize;        snap.processCount = perfInfo.ProcessCount;    }    snap.standbyMemory = NtApi::getStandbyMemorySize();    snap.modifiedMemory = NtApi::getTotalModifiedMemory();    snap.compressedMemory = NtApi::getCompressedMemorySize();    snap.compressionTotalData = NtApi::getCompressionTotalData();
    if(snap.compressionTotalData > snap.compressedMemory && snap.compressedMemory > 0) {        snap.compressionRatio = static_cast<double>(snap.compressionTotalData) / snap.compressedMemory;    }    snap.pressureScore = snap.memoryLoad;    FILETIME idleTime, kernelTime, userTime;
    if(GetSystemTimes(&idleTime, &kernelTime, &userTime)) {        auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(            now - m_lastCpuTime).count();
uint64_t idle = (static_cast<uint64_t>(idleTime.dwHighDateTime) << 32) | idleTime.dwLowDateTime;
uint64_t kernel = (static_cast<uint64_t>(kernelTime.dwHighDateTime) << 32) | kernelTime.dwLowDateTime;
uint64_t user = (static_cast<uint64_t>(userTime.dwHighDateTime) << 32) | userTime.dwLowDateTime;
    if(elapsed > 0 && m_prevKernelTime > 0) {            uint64_t totalDiff = (kernel + user) - (m_prevKernelTime + m_prevUserTime);
uint64_t idleDiff = idle - m_prevIdleTime;            snap.cpuUsage = (totalDiff > 0)                ? (1.0 - static_cast<double>(idleDiff) / totalDiff) * 100.0                : 0.0;        }        m_prevIdleTime = idle;        m_prevKernelTime = kernel;        m_prevUserTime = user;        m_lastCpuTime = now;    }    uint64_t currentPageFileUsage = snap.usedPageFile;
static uint64_t lastPageFileSample = 0;
static auto lastPFTime = std::chrono::steady_clock::now();
    auto pfNow = std::chrono::steady_clock::now();
    auto pfElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(pfNow - lastPFTime).count();
    if(pfElapsed > 0 && lastPageFileSample > 0) {        int64_t delta = static_cast<int64_t>(currentPageFileUsage) - static_cast<int64_t>(lastPageFileSample);
    if(delta > 0) {            snap.hardFaultsPerSec = static_cast<uint64_t>(delta * 1000 / pfElapsed / 4096);        }
static uint64_t prevFaultRate = 0;
    if(prevFaultRate > 0 && snap.hardFaultsPerSec > 0) {            snap.pageFaultTrend = (static_cast<double>(snap.hardFaultsPerSec) - prevFaultRate) / prevFaultRate;        }        prevFaultRate = snap.hardFaultsPerSec;    }    lastPageFileSample = currentPageFileUsage;    lastPFTime = pfNow;
HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hSnapshot != INVALID_HANDLE_VALUE) {        PROCESSENTRY32W pe32;        pe32.dwSize = sizeof(pe32);
std::vector<ProcessMemoryBreakdown> processes;
    if(Process32FirstW(hSnapshot, &pe32)) {
do {                ProcessMemoryBreakdown pi;                pi.pid = pe32.th32ProcessID;                pi.name = pe32.szExeFile;                pi.threadCount = pe32.cntThreads;
HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
    if(hProcess) {                    PROCESS_MEMORY_COUNTERS_EX pmc;
                        if(GetProcessMemoryInfo(hProcess, reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&pmc), sizeof(pmc))) {                        pi.workingSet = pmc.WorkingSetSize;                        pi.peakWorkingSet = pmc.PeakWorkingSetSize;                        pi.privateUsage = pmc.PrivateUsage;                        pi.pageFileUsage = pmc.PagefileUsage;                        pi.pageFaults = pmc.PageFaultCount;                    }                    CloseHandle(hProcess);                }                processes.push_back(pi);            } while (Process32NextW(hSnapshot, &pe32));        }        CloseHandle(hSnapshot);
std::sort(processes.begin(), processes.end(),            [](const ProcessMemoryBreakdown& a, const ProcessMemoryBreakdown& b) {
return a.workingSet > b.workingSet;            }
);
    if(processes.size() > 20) processes.resize(20);        snap.topProcesses = std::move(processes);    }
    return snap;
}} // namespace RAMFlux::Telemetry 