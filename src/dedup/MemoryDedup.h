// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
namespace RAMFlux::Telemetry { struct MemorySnapshot; }
namespace RAMFlux::Dedup {
struct DedupProcessStats {
    uint32_t pid{0};
    std::wstring name;
    uint64_t totalPagesScanned{0};
    uint64_t zeroPages{0};
    uint64_t duplicatePages{0};
    uint64_t estimatedSavingsBytes{0};
    double zeroRatio{0.0};
};
struct DedupCandidateGroup {
    struct PageInfo { uint32_t pid; uint64_t va; };
    uint64_t hash{0};
    bool isZero{false};
    std::vector<PageInfo> pages;
    uint64_t estimatedSavingsBytes{0};
};
struct DedupReport {
    bool scanned{false};
    std::vector<DedupProcessStats> processStats;
    std::vector<DedupCandidateGroup> candidates;
    uint64_t totalEstimatedSavingsBytes{0};
    int totalCandidates{0};
    int pagesHashed{0};
    std::chrono::steady_clock::time_point lastScan;
};
class MemoryDedup {
public:
    MemoryDedup();
    DedupReport scan(const Telemetry::MemorySnapshot& snap);
    bool tryOfferDuplicates(const DedupReport& report, const Telemetry::MemorySnapshot& snap);
    DedupReport lastReport() const;
    void reset();
    void setEnabled(bool enabled);
    bool isEnabled() const;
    void setOfferEnabled(bool v) { m_offerEnabled = v; }
    bool isOfferEnabled() const { return m_offerEnabled.load(); }
    static constexpr int SCAN_INTERVAL_SEC{120};
    static constexpr size_t MAX_PROCESSES{10};
    static constexpr size_t MAX_PAGES_PER_PROCESS{512};
    static constexpr size_t MAX_REGIONS{256};
    static constexpr size_t PAGE_SIZE{4096};
private:
    struct PageHash {
        uint64_t hash;
        uint64_t va;
        bool isZero;
    };
    static uint64_t fnv1a64(const uint8_t* data, size_t len);
    static bool checkZeroPage(const uint8_t* data);
    mutable std::mutex m_mtx;
    std::atomic<bool> m_enabled{true};
    std::atomic<bool> m_offerEnabled{false};
    std::chrono::steady_clock::time_point m_lastScan;
    DedupReport m_lastReport;
};
} // namespace RAMFlux::Dedup
