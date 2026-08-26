// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "MemoryDedup.h"
#include "ntapi/FluxNTAPI.h"
#include "process/ProcessCache.h"
#include "shared/ProcessUtils.h"
#include "telemetry/MemorySnapshot.h"
#include "core/Logger.h"
#include <windows.h>
#include <algorithm>
#include <unordered_set>
#include <cstring>
namespace RAMFlux::Dedup {
using Core::Logger;
MemoryDedup::MemoryDedup() {
    m_lastScan = std::chrono::steady_clock::now();
}
uint64_t MemoryDedup::fnv1a64(const uint8_t* data, size_t len) {
    uint64_t hash = 0xCBF29CE484222325ULL;
    for(size_t i = 0; i < len; ++i) {
        hash ^= static_cast<uint64_t>(data[i]);
        hash *= 0x100000001B3ULL;
    }
    return hash;
}
bool MemoryDedup::checkZeroPage(const uint8_t* data) {
    const uint64_t* qwords = reinterpret_cast<const uint64_t*>(data);
    for(size_t i = 0; i < PAGE_SIZE / sizeof(uint64_t); ++i) {
        if(qwords[i] != 0) return false;
    }
    return true;
}
DedupReport MemoryDedup::scan(const Telemetry::MemorySnapshot& snap) {
    DedupReport r;
    if(!m_enabled) return r;
    auto now = std::chrono::steady_clock::now();
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - m_lastScan).count();
        if(elapsed < SCAN_INTERVAL_SEC) return m_lastReport;
        m_lastScan = now;
    }
    auto entries = Process::ProcessCache::instance().processes();
    std::vector<const Process::ProcessEntry*> candidates;
    for(const auto& e : entries) {
        if(e.pid <= 4 || e.pid == GetCurrentProcessId()) continue;
        if(RAMFlux::ProcessUtils::isKnownCriticalProcess(e.name)) continue;
        candidates.push_back(&e);
    }
    std::sort(candidates.begin(), candidates.end(),
        [](const Process::ProcessEntry* a, const Process::ProcessEntry* b) {
            return a->workingSet > b->workingSet;
        });
    size_t limit = std::min(candidates.size(), MAX_PROCESSES);
    candidates.resize(limit);
    struct ScannedPage {
        uint32_t pid;
        uint64_t hash;
        uint64_t va;
        bool isZero;
    };
    std::vector<ScannedPage> allPages;
    allPages.reserve(limit * MAX_PAGES_PER_PROCESS);
    int totalHashed = 0;
    for(const auto* entry : candidates) {
        NtApi::ScopedPrivilege _priv(SE_DEBUG_NAME);
        HANDLE hProcess = OpenProcess(PROCESS_VM_READ |
            PROCESS_QUERY_INFORMATION, FALSE, entry->pid);
        if(!hProcess) continue;
        DedupProcessStats stats;
        stats.pid = entry->pid;
        stats.name = entry->name;
        uint8_t* addr = nullptr;
        MEMORY_BASIC_INFORMATION mbi;
        int regionCount = 0;
        uint64_t zeroCount = 0;
        uint64_t pageCount = 0;
        while(regionCount < static_cast<int>(MAX_REGIONS) &&
            pageCount < MAX_PAGES_PER_PROCESS &&
            VirtualQueryEx(hProcess, addr, &mbi, sizeof(mbi)) == sizeof(mbi)) {
            regionCount++;
            if(mbi.State == MEM_COMMIT &&
                mbi.Type == MEM_PRIVATE &&
                (mbi.Protect & (PAGE_READWRITE | PAGE_EXECUTE_READWRITE))) {
                uint64_t regionStart =
                    reinterpret_cast<uint64_t>(mbi.BaseAddress);
                uint64_t regionEnd = regionStart + mbi.RegionSize;
                size_t pagesInRegion = static_cast<size_t>(
                    (regionEnd - regionStart) / PAGE_SIZE);
                size_t remaining = MAX_PAGES_PER_PROCESS - pageCount;
                size_t desired = std::min(remaining,
                    std::max(size_t{1}, pagesInRegion / 3));
                size_t step = pagesInRegion / std::max(desired, size_t{1});
                if(step < 1) step = 1;
                for(size_t i = 0; i < pagesInRegion &&
                    pageCount < MAX_PAGES_PER_PROCESS; i += step) {
                    uint64_t pageVa = regionStart + i * PAGE_SIZE;
                    uint8_t buf[PAGE_SIZE];
                    SIZE_T bytesRead = 0;
                    if(!ReadProcessMemory(hProcess,
                        reinterpret_cast<LPCVOID>(pageVa),
                        buf, PAGE_SIZE, &bytesRead) || bytesRead != PAGE_SIZE)
                        continue;
                    pageCount++;
                    totalHashed++;
                    bool isZero = checkZeroPage(buf);
                    uint64_t hash = isZero ? 0 : fnv1a64(buf, PAGE_SIZE);
                    if(isZero) zeroCount++;
                    allPages.push_back({entry->pid, hash, pageVa, isZero});
                }
            }
            addr = reinterpret_cast<uint8_t*>(mbi.BaseAddress) + mbi.RegionSize;
        }
        CloseHandle(hProcess);
        stats.totalPagesScanned = pageCount;
        stats.zeroPages = zeroCount;
        stats.zeroRatio = (pageCount > 0)
            ? static_cast<double>(zeroCount) / pageCount : 0.0;
        r.processStats.push_back(stats);
    }
    r.pagesHashed = totalHashed;
    std::unordered_map<uint64_t, DedupCandidateGroup> groupMap;
    for(const auto& page : allPages) {
        if(page.isZero) continue;
        auto& g = groupMap[page.hash];
        g.hash = page.hash;
        g.pages.push_back({page.pid, page.va});
    }
    for(auto& [hash, group] : groupMap) {
        if(group.pages.size() >= 2) {
            std::unordered_set<uint32_t> uniquePids;
            for(const auto& p : group.pages)
                uniquePids.insert(p.pid);
            uint64_t savings = static_cast<uint64_t>(
                (group.pages.size() - uniquePids.size()) * PAGE_SIZE);
            group.estimatedSavingsBytes = savings;
            r.totalEstimatedSavingsBytes += savings;
            r.candidates.push_back(std::move(group));
            r.totalCandidates++;
        }
    }
    for(auto& stat : r.processStats) {
        uint64_t crossSavings = 0;
        for(const auto& cand : r.candidates) {
            bool hasThisPid = false;
            for(const auto& p : cand.pages) {
                if(p.pid == stat.pid) { hasThisPid = true; break; }
            }
            if(hasThisPid) {
                stat.duplicatePages++;
                crossSavings += PAGE_SIZE;
            }
        }
        stat.estimatedSavingsBytes = stat.zeroPages * PAGE_SIZE +
            crossSavings;
    }
    r.scanned = true;
    r.lastScan = now;
    Logger::instance().info(std::string("[MemoryDedup] Scanned ")
        + std::to_string(candidates.size()) + " processes, "
        + std::to_string(totalHashed) + " pages, "
        + std::to_string(r.totalCandidates) + " duplicate groups, "
        + "potential savings "
        + std::to_string(r.totalEstimatedSavingsBytes / (1024 * 1024))
        + "MB");
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_lastReport = r;
    }
    return r;
}
bool MemoryDedup::tryOfferDuplicates(const DedupReport& report, const Telemetry::MemorySnapshot& snap) {
    if(!m_offerEnabled.load() || !m_enabled.load()) return false;
    if(snap.pressureScore < 75) return false;
    if(report.totalEstimatedSavingsBytes < 5ULL * 1024 * 1024) return false;
    // Prototype: report savings and, for high pressure, trim cold pages as proxy for dedup
    // True cross-process COW would require kernel driver (KSM-like). This prototype logs
    // and offers zero-page reclaim as safe user-mode approximation.
    int groups = static_cast<int>(report.candidates.size());
    Logger::instance().info(std::string("[MemoryDedup] Dedup opportunity: ")
        + std::to_string(groups) + " groups, "
        + std::to_string(report.totalEstimatedSavingsBytes / (1024*1024)) + " MB potential, pressure "
        + std::to_string(snap.pressureScore) + " — use intelligentStandbyClean for reclaim");
    return true;
}
DedupReport MemoryDedup::lastReport() const {
    std::lock_guard<std::mutex> lock(m_mtx);
    return m_lastReport;
}
void MemoryDedup::reset() {
    std::lock_guard<std::mutex> lock(m_mtx);
    m_lastReport = {};
}
void MemoryDedup::setEnabled(bool enabled) { m_enabled = enabled; }
bool MemoryDedup::isEnabled() const { return m_enabled.load(); }
} // namespace RAMFlux::Dedup
