// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <cstddef>
namespace RAMFlux::NtApi {
bool clearStandbyList();
bool clearModifiedPageList();
bool clearWorkingSet();
bool clearCombinedPageList();
bool defragmentMemory();
bool setProcessPriority(uint32_t pid, uint32_t priorityClass);
uint32_t getProcessPriority(uint32_t pid);
bool setProcessIoPriority(uint32_t pid, uint32_t priority);
bool clearSystemFileCache();
uint64_t getStandbyMemorySize();
uint64_t getCompressedMemorySize();
uint64_t getCompressionTotalData();
double getCompressionRatio();
uint64_t getWorkingSetSize(uint32_t processId);
bool trimProcessWorkingSet(uint32_t processId);
bool trimAllProcesses();
std::vector<uint32_t> getProcessesWithLargeWS(uint64_t thresholdBytes);
uint64_t getProcessStandbyMemory(uint32_t processId);
uint64_t getProcessPageTableUsage(uint32_t processId);
    struct FileCacheInfo {    std::wstring fileName;
std::wstring filePath;
uint64_t activeSize{
0};
uint64_t standbySize{
0};
uint64_t modifiedSize{
0};
uint64_t totalSize{
0};
size_t processCount{
0};
};
std::vector<FileCacheInfo> getTopFileCache(int count = 10);
struct ProcessIoStats {    uint64_t readOps{0};
uint64_t writeOps{0};    uint64_t otherOps{0};
uint64_t readBytes{0};    uint64_t writeBytes{0};
uint64_t otherBytes{0}; };
uint32_t getProcessHandleCount(uint32_t pid);
ProcessIoStats getProcessIoStats(uint32_t pid);
uint64_t getProcessCreationTime(uint32_t pid);
uint64_t getTotalModifiedMemory();
struct PhysicalMemoryBreakdown {    uint64_t activePages{0};
uint64_t standbyPages{0};
uint64_t modifiedPages{0};
uint64_t modifiedNoWritePages{0};
uint64_t transitionPages{0};
uint64_t zeroPages{0};
uint64_t freePages{0};
uint64_t badPages{0};
uint64_t totalPages{0};
uint64_t pageSize{4096}; };
PhysicalMemoryBreakdown getPhysicalMemoryBreakdown();
bool isFullScreenAppActive();
} // namespace RAMFlux::NtApi


