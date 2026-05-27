// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "LeakHunter.h"
#include "core/EventBus.h"
#include "shared/Constants.h"
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <algorithm>
#include "core/Logger.h"
#include <exception>
#include <unordered_set>
namespace RAMFlux::LeakHunter {
using RAMFlux::Core::Logger;
LeakHunter::LeakHunter() = default;
LeakHunter::~LeakHunter() {    shutdown();
}
bool LeakHunter::initialize() {    Logger::instance().info("[LeakHunter] Initializing...");    m_running = true;    m_thread = std::thread(&LeakHunter::monitorLoop, this);
    Logger::instance().info("[LeakHunter] Monitor thread started");
    return true;
}
void LeakHunter::shutdown() {
if(m_running.exchange(false)) {
if(m_thread.joinable()) {            m_thread.join();        }        Logger::instance().info("[LeakHunter] Shutdown complete");    }}
std::string LeakHunter::name() const {
return "LeakHunter";
}
void LeakHunter::monitorLoop() {
while(m_running) {
try {
if(!m_enabled) {                std::this_thread::sleep_for(std::chrono::milliseconds(Constants::LEAK_CHECK_INTERVAL_MS));
    continue;            }            std::unordered_set<uint32_t> seenPids;
HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hSnapshot != INVALID_HANDLE_VALUE) {                PROCESSENTRY32W pe32;                pe32.dwSize = sizeof(pe32);
    if(Process32FirstW(hSnapshot, &pe32)) {
do {                        seenPids.insert(pe32.th32ProcessID);
HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
    if(!hProcess) continue;                        PROCESS_MEMORY_COUNTERS pmc;
    if(GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {                            auto report = analyzeProcess(pe32.th32ProcessID, pe32.szExeFile, pmc.WorkingSetSize);
    if(report.suspicious) {                                {                                    std::lock_guard<std::mutex> lock(m_dataMutex);
    auto it = std::find_if(m_reports.begin(), m_reports.end(),                                        [pid = pe32.th32ProcessID](const LeakReport& r) {
return r.pid == pid; }
);
    if(it != m_reports.end()) {                                        *it = report;                                    } else {                                        m_reports.push_back(report);                                    }                                }                                if (report.growthBytes >= m_thresholdMB * 1024 * 1024) {                                    Core::EventBus::instance().post(Constants::EventType::LeakDetected);
std::string pname(report.processName.begin(), report.processName.end());
    Logger::instance().warn(std::string("[LeakHunter] Potential leak: ")                                        + pname + " PID:"                                        + std::to_string(report.pid)                                        + " growth: " + std::to_string(report.growthBytes / (1024*1024))                                        + " MB");                                }                            }                        }                        CloseHandle(hProcess);                    } while (Process32NextW(hSnapshot, &pe32));                }                CloseHandle(hSnapshot);            }            {                std::lock_guard<std::mutex> lock(m_dataMutex);
    if(m_reports.size() > 1000) {                    m_reports.erase(m_reports.begin(), m_reports.begin() + 900);                }                auto now = std::chrono::steady_clock::now();                m_reports.erase(std::remove_if(m_reports.begin(), m_reports.end(),                    [now](const LeakReport& r) {                        auto age = std::chrono::duration_cast<std::chrono::minutes>(                            now - r.detectedAt).count();
    return age > 60 || r.growthBytes == 0;                    }
), m_reports.end());
    for(auto it = m_history.begin(); it != m_history.end();) {
if(seenPids.find(it->first) == seenPids.end()) {                        it = m_history.erase(it);                    } else {                        ++it;                    }                }            }        } catch (const std::exception& e) {            Logger::instance().error(std::string("[LeakHunter] Error: ") + e.what());        } catch (...) {            Logger::instance().error("[LeakHunter] Unknown error");        }        std::this_thread::sleep_for(std::chrono::milliseconds(Constants::LEAK_CHECK_INTERVAL_MS));    }}
LeakReport LeakHunter::analyzeProcess(uint32_t pid, const std::wstring& name, uint64_t currentWS) {    LeakReport report;    report.pid = pid;    report.processName = name;    report.currentWorkingSet = currentWS;    report.detectedAt = std::chrono::steady_clock::now();
std::lock_guard<std::mutex> lock(m_dataMutex);
    auto& hist = m_history[pid];    hist.samples.push_back(currentWS);    hist.name = name;
    if(hist.samples.size() == 1) {        hist.firstSeen = std::chrono::steady_clock::now();        hist.minWorkingSet = currentWS;    }    hist.maxWorkingSet = std::max(hist.maxWorkingSet, currentWS);    hist.minWorkingSet = std::min(hist.minWorkingSet, currentWS);
    if(hist.samples.size() > Constants::LEAK_HISTORY_SAMPLES) {        hist.samples.erase(hist.samples.begin());    }    report.initialWorkingSet = hist.minWorkingSet;    report.peakWorkingSet = hist.maxWorkingSet;    report.sampleCount = static_cast<int>(hist.samples.size());
    if(hist.samples.size() >= 5) {        uint64_t first = hist.samples.front();
uint64_t last = hist.samples.back();        report.growthBytes = (last > first) ? (last - first) : 0;
    if(first > 0) {            report.growthPercent = (static_cast<double>(report.growthBytes) / first) * 100.0;        }        report.suspicious = (report.growthBytes >= 50 * 1024 * 1024) && report.growthPercent > 20.0;    }
return report;
}
std::vector<LeakReport> LeakHunter::currentReports() const {    std::lock_guard<std::mutex> lock(m_dataMutex);
std::vector<LeakReport> sorted = m_reports;
std::sort(sorted.begin(), sorted.end(),        [](const LeakReport& a, const LeakReport& b) {
return a.growthBytes > b.growthBytes;        }
);
    return sorted;
}
std::vector<LeakReport> LeakHunter::getActiveLeaks(uint64_t minGrowthMB) {    std::lock_guard<std::mutex> lock(m_dataMutex);
std::vector<LeakReport> leaks;
uint64_t minBytes = minGrowthMB * 1024 * 1024;
    for(const auto& [pid, hist] : m_history) {
if(hist.samples.size() >= 5) {            uint64_t first = hist.samples.front();
uint64_t last = hist.samples.back();
    if(last > first && (last - first) >= minBytes) {                LeakReport r;                r.pid = pid;                r.processName = hist.name;                r.initialWorkingSet = first;                r.currentWorkingSet = last;                r.growthBytes = last - first;                r.sampleCount = static_cast<int>(hist.samples.size());                r.suspicious = true;                leaks.push_back(r);            }        }    }    std::sort(leaks.begin(), leaks.end(),        [](const LeakReport& a, const LeakReport& b) {
return a.growthBytes > b.growthBytes;        }
);
    return leaks;
}
void LeakHunter::setThresholdMB(uint64_t mb) {    m_thresholdMB.store(std::max(uint64_t{
10}
, std::min(uint64_t{
10000}
, mb)));
}
void LeakHunter::setEnabled(bool enabled) {    m_enabled.store(enabled);
}
bool LeakHunter::isEnabled() const {
return m_enabled.load();
}} // namespace RAMFlux::LeakHunter


