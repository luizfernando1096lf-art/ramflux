// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "FluxNTAPI.h"
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
namespace RAMFlux::NtApi {
static bool enableLockMemoryPrivilege() {    HANDLE hToken;
    if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
return false;    LUID luid;
    if(!LookupPrivilegeValueW(nullptr, SE_LOCK_MEMORY_NAME, &luid)) {        CloseHandle(hToken);
    return false;    }    TOKEN_PRIVILEGES tp;    tp.PrivilegeCount = 1;    tp.Privileges[0].Luid = luid;    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
BOOL ok = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), nullptr, nullptr);
DWORD err = GetLastError();    CloseHandle(hToken);
    return ok && err == ERROR_SUCCESS;
}
bool clearStandbyList() {
if(!enableLockMemoryPrivilege())
return false;    HMODULE hNtDll = GetModuleHandleW(L"ntdll.dll");
    if(!hNtDll)
return false;
    auto NtSetSystemInformation = (LONG(NTAPI*)(DWORD, PVOID, ULONG))        GetProcAddress(hNtDll, "NtSetSystemInformation");
    if(!NtSetSystemInformation)
return false;
    struct { DWORD ListCommand; } cmd;    cmd.ListCommand = 0x03;
    return NtSetSystemInformation(0x50, &cmd, sizeof(cmd)) == 0;
}
bool clearModifiedPageList() {
if(!enableLockMemoryPrivilege())
return false;    HMODULE hNtDll = GetModuleHandleW(L"ntdll.dll");
    if(!hNtDll)
return false;
    auto NtSetSystemInformation = (LONG(NTAPI*)(DWORD, PVOID, ULONG))        GetProcAddress(hNtDll, "NtSetSystemInformation");
    if(!NtSetSystemInformation)
return false;
    struct { DWORD ListCommand; } cmd;    cmd.ListCommand = 0x04;
LONG status = NtSetSystemInformation(0x50, &cmd, sizeof(cmd));
    return status == 0;
}
bool clearWorkingSet() {
if(!enableLockMemoryPrivilege())
return false;    HMODULE hNtDll = GetModuleHandleW(L"ntdll.dll");
    if(!hNtDll)
return false;
    auto NtSetSystemInformation = (LONG(NTAPI*)(DWORD, PVOID, ULONG))        GetProcAddress(hNtDll, "NtSetSystemInformation");
    if(!NtSetSystemInformation)
return false;
    struct { DWORD ListCommand; } cmd;    cmd.ListCommand = 0x01;
    LONG status = NtSetSystemInformation(0x50, &cmd, sizeof(cmd));
    return status == 0;
}
bool clearCombinedPageList() {
if(!enableLockMemoryPrivilege())
return false;    HMODULE hNtDll = GetModuleHandleW(L"ntdll.dll");
    if(!hNtDll)
return false;
    auto NtSetSystemInformation = (LONG(NTAPI*)(DWORD, PVOID, ULONG))        GetProcAddress(hNtDll, "NtSetSystemInformation");
    if(!NtSetSystemInformation)
return false;
    struct { DWORD ListCommand; } cmd;    cmd.ListCommand = 0x05;
    LONG status = NtSetSystemInformation(0x50, &cmd, sizeof(cmd));
    return status == 0;
}
bool defragmentMemory() {
if(!enableLockMemoryPrivilege())
return false;    HMODULE hNtDll = GetModuleHandleW(L"ntdll.dll");
    if(!hNtDll)
return false;
    auto NtSetSystemInformation = (LONG(NTAPI*)(DWORD, PVOID, ULONG))        GetProcAddress(hNtDll, "NtSetSystemInformation");
    if(!NtSetSystemInformation)
return false;
    // Command 0x05: combine identical memory pages across processes
    struct { DWORD ListCommand; } cmd;    cmd.ListCommand = 0x05;
    LONG status = NtSetSystemInformation(0x50, &cmd, sizeof(cmd));
    if(status != 0) return false;
    return true;
}
bool setProcessPriority(uint32_t pid, uint32_t priorityClass) {    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
    if(!hProcess) return false;
    BOOL ok = SetPriorityClass(hProcess, priorityClass);    CloseHandle(hProcess);
    return ok != FALSE;
}
uint32_t getProcessPriority(uint32_t pid) {    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if(!hProcess) return 0;
    uint32_t pc = GetPriorityClass(hProcess);    CloseHandle(hProcess);
    return pc;
}
bool setProcessIoPriority(uint32_t pid, uint32_t priority) {    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
    if(!hProcess) return false;
    BOOL ok = SetProcessInformation(hProcess, (PROCESS_INFORMATION_CLASS)0x11, &priority, sizeof(priority));    CloseHandle(hProcess);
    return ok != FALSE;
}
bool clearSystemFileCache() {
if(!enableLockMemoryPrivilege())
return false;
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hToken;
    if(!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken))
return false;
    LUID luid;
    if(!LookupPrivilegeValueW(nullptr, SE_INCREASE_QUOTA_NAME, &luid)) {        CloseHandle(hToken);
return false;    }    TOKEN_PRIVILEGES tp;    tp.PrivilegeCount = 1;    tp.Privileges[0].Luid = luid;    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;    AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), nullptr, nullptr);    CloseHandle(hToken);
    HMODULE hNtDll = GetModuleHandleW(L"ntdll.dll");
    if(!hNtDll)
return false;
    auto NtSetSystemInformation = (LONG(NTAPI*)(DWORD, PVOID, ULONG))        GetProcAddress(hNtDll, "NtSetSystemInformation");
    if(!NtSetSystemInformation)
return false;
    struct { ULONG MinimumWorkingSet; ULONG MaximumWorkingSet; } info;    info.MinimumWorkingSet = (ULONG)-1;    info.MaximumWorkingSet = (ULONG)-1;
    LONG status = NtSetSystemInformation(0x53, &info, sizeof(info));
    return status == 0;
}
static bool queryMemoryListFields(uint64_t* fields, int maxFields) {    HMODULE hNtDll = GetModuleHandleW(L"ntdll.dll");
    if(!hNtDll)
return false;
    auto NtQuerySystemInformation = (LONG(NTAPI*)(DWORD, PVOID, ULONG, PULONG))        GetProcAddress(hNtDll, "NtQuerySystemInformation");
    if(!NtQuerySystemInformation)
return false;
    // Windows 11 requires a larger buffer (SYSTEM_MEMORY_LIST_INFORMATION grew past 11 fields)
    std::vector<uint8_t> buffer(512);
    ULONG returnLen = 0;
    if(NtQuerySystemInformation(0x50, buffer.data(), static_cast<ULONG>(buffer.size()), &returnLen) != 0)
return false;
    int count = std::min(maxFields, static_cast<int>(returnLen / sizeof(uint64_t)));
    for(int i = 0; i < count; i++) {        fields[i] = reinterpret_cast<uint64_t*>(buffer.data())[i];    }
return true;
}
uint64_t getStandbyMemorySize() {
    uint64_t fields[16]{};
    if(!queryMemoryListFields(fields, 16))
return 0;
    SYSTEM_INFO sysInfo;    GetSystemInfo(&sysInfo);
    return fields[10] * sysInfo.dwPageSize; // StandbyPages at index 10
}
uint64_t getCompressedMemorySize() {    auto getProcWS = [](const wchar_t* name) -> uint64_t {        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hSnapshot == INVALID_HANDLE_VALUE)
return 0;
uint64_t ws = 0;        PROCESSENTRY32W pe32;        pe32.dwSize = sizeof(pe32);
    if(Process32FirstW(hSnapshot, &pe32)) {
do {
if(_wcsicmp(pe32.szExeFile, name) == 0) {                    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
    if(hProcess) {                        PROCESS_MEMORY_COUNTERS pmc;
    if(GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {                            ws = pmc.WorkingSetSize;                        }                        CloseHandle(hProcess);                    }                    break;                }            } while (Process32NextW(hSnapshot, &pe32));        }        CloseHandle(hSnapshot);
    return ws;    };
return getProcWS(L"Memory Compression.exe") + getProcWS(L"MemCompression.exe");
}
uint64_t getCompressionTotalData() {    auto getProcPrivate = [](const wchar_t* name) -> uint64_t {        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hSnapshot == INVALID_HANDLE_VALUE)
return 0;
uint64_t priv = 0;        PROCESSENTRY32W pe32;        pe32.dwSize = sizeof(pe32);
    if(Process32FirstW(hSnapshot, &pe32)) {
do {
if(_wcsicmp(pe32.szExeFile, name) == 0) {                    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
    if(hProcess) {                        PROCESS_MEMORY_COUNTERS_EX pmc;
    if(GetProcessMemoryInfo(hProcess, reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&pmc), sizeof(pmc))) {                            priv = pmc.PrivateUsage;                        }                        CloseHandle(hProcess);                    }                    break;                }            } while (Process32NextW(hSnapshot, &pe32));        }        CloseHandle(hSnapshot);
    return priv;    };
return getProcPrivate(L"Memory Compression.exe") + getProcPrivate(L"MemCompression.exe");
}
double getCompressionRatio() {    uint64_t compressed = getCompressedMemorySize();
uint64_t totalData = getCompressionTotalData();
    if(compressed == 0 || totalData == 0)
return 0.0;
    return static_cast<double>(totalData) / compressed;
}
uint64_t getTotalModifiedMemory() {
    uint64_t fields[16]{};
    if(!queryMemoryListFields(fields, 16))
return 0;
    SYSTEM_INFO sysInfo;    GetSystemInfo(&sysInfo);
    return (fields[6] + fields[7]) * sysInfo.dwPageSize; // ModifiedPages=6, ModifiedNoWritePages=7
}
uint64_t getWorkingSetSize(uint32_t processId) {    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if(!hProcess)
return 0;    PROCESS_MEMORY_COUNTERS pmc;
uint64_t ws = 0;
    if(GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {        ws = pmc.WorkingSetSize;    }    CloseHandle(hProcess);
    return ws;
}
bool trimProcessWorkingSet(uint32_t processId) {    HANDLE hProcess = OpenProcess(PROCESS_SET_QUOTA | PROCESS_QUERY_INFORMATION, FALSE, processId);
    if(!hProcess)
return false;
BOOL result = SetProcessWorkingSetSize(hProcess, (SIZE_T)-1, (SIZE_T)-1);    CloseHandle(hProcess);
    return result != FALSE;
}
bool trimAllProcesses() {    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hSnapshot == INVALID_HANDLE_VALUE)
return false;    PROCESSENTRY32W pe32;    pe32.dwSize = sizeof(pe32);
    if(Process32FirstW(hSnapshot, &pe32)) {
do {
if(pe32.th32ProcessID != GetCurrentProcessId()) {                trimProcessWorkingSet(pe32.th32ProcessID);            }        } while (Process32NextW(hSnapshot, &pe32));    }    CloseHandle(hSnapshot);
    return true;
}
std::vector<uint32_t> getProcessesWithLargeWS(uint64_t thresholdBytes) {    std::vector<uint32_t> result;
HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hSnapshot == INVALID_HANDLE_VALUE)
return result;    PROCESSENTRY32W pe32;    pe32.dwSize = sizeof(pe32);
    if(Process32FirstW(hSnapshot, &pe32)) {
do {            uint64_t ws = getWorkingSetSize(pe32.th32ProcessID);
    if(ws > thresholdBytes) {                result.push_back(pe32.th32ProcessID);            }        } while (Process32NextW(hSnapshot, &pe32));    }    CloseHandle(hSnapshot);
    return result;
}
// Per-process standby memory is not available via Windows API.
// This function is kept for compatibility but always returns 0.
uint64_t getProcessStandbyMemory(uint32_t processId) {
    (void)processId;
    return 0;
}
uint64_t getProcessPageTableUsage(uint32_t processId) {    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
    if(!hProcess)
return 0;    PROCESS_MEMORY_COUNTERS_EX pmc;
uint64_t ptUsage = 0;
    if(GetProcessMemoryInfo(hProcess, reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&pmc), sizeof(pmc))) {        ptUsage = pmc.PagefileUsage > pmc.PrivateUsage ? pmc.PagefileUsage - pmc.PrivateUsage : 0;    }    CloseHandle(hProcess);
    return ptUsage;
}
std::vector<FileCacheInfo> getTopFileCache(int count) {    std::vector<FileCacheInfo> results;    HMODULE hNtDll = GetModuleHandleW(L"ntdll.dll");
    if(!hNtDll)
return results;
    auto NtQuerySystemInformation = (LONG(NTAPI*)(DWORD, PVOID, ULONG, PULONG))        GetProcAddress(hNtDll, "NtQuerySystemInformation");
    if(!NtQuerySystemInformation)
return results;
    // Get total system file cache
    struct { uint64_t CurrentSize, PeakSize, PageFaultCount, MinimumWorkingSet, MaximumWorkingSet, CurrentSizeIncludingTransitionInPages, PeakSizeIncludingTransitionInPages; } cacheInfo{};
    ULONG returnLen = 0;
    LONG status = NtQuerySystemInformation(0x53, &cacheInfo, sizeof(cacheInfo), &returnLen);
    if(status == 0) {        FileCacheInfo total;        total.fileName = L"System File Cache (Total)";        total.totalSize = cacheInfo.CurrentSize;        total.activeSize = cacheInfo.CurrentSize;        total.processCount = 1;        results.push_back(total);    }
    // Per-process module enumeration
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hSnapshot != INVALID_HANDLE_VALUE) {        PROCESSENTRY32W pe32;        pe32.dwSize = sizeof(pe32);
        if(Process32FirstW(hSnapshot, &pe32)) {            std::unordered_map<std::wstring, FileCacheInfo> fileMap;
            do {                if(pe32.th32ProcessID == 0 || pe32.th32ProcessID == 4 || pe32.th32ProcessID == GetCurrentProcessId()) continue;
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
                if(!hProcess) continue;
                DWORD needed = 0;
                if(!EnumProcessModules(hProcess, nullptr, 0, &needed)) { CloseHandle(hProcess); continue; }
                std::vector<HMODULE> mods(needed / sizeof(HMODULE));
                if(EnumProcessModules(hProcess, mods.data(), needed, &needed)) {
                    for(auto hMod : mods) {                        WCHAR modPath[MAX_PATH];
                        if(!GetModuleFileNameExW(hProcess, hMod, modPath, MAX_PATH)) continue;
                        MODULEINFO mi{};
                        if(!GetModuleInformation(hProcess, hMod, &mi, sizeof(mi))) continue;
                        std::wstring path(modPath);
                        auto it = fileMap.find(path);
                        if(it == fileMap.end()) {                            FileCacheInfo fi;                            size_t pos = path.rfind(L'\\');
                        fi.fileName = (pos != std::wstring::npos) ? path.substr(pos + 1) : path;
                        fi.filePath = path;                            fi.totalSize = mi.SizeOfImage;
                        fi.activeSize = mi.SizeOfImage;                            fi.processCount = 1;
                        fileMap[path] = fi;                        } else {                            it->second.totalSize += mi.SizeOfImage;
                        it->second.activeSize += mi.SizeOfImage;                            it->second.processCount++;                        }                    }                }                CloseHandle(hProcess);            } while (Process32NextW(hSnapshot, &pe32));
            for(auto& p : fileMap) { results.push_back(std::move(p.second)); }        }        CloseHandle(hSnapshot);    }
    // Sort by totalSize descending
    std::sort(results.begin(), results.end(), [](const FileCacheInfo& a, const FileCacheInfo& b) { return a.totalSize > b.totalSize; });
    if(static_cast<int>(results.size()) > count) { results.resize(count); }
return results;
}
PhysicalMemoryBreakdown getPhysicalMemoryBreakdown() {    PhysicalMemoryBreakdown b;    SYSTEM_INFO si;    GetSystemInfo(&si);    b.pageSize = si.dwPageSize;
    uint64_t fields[20]{};
    if(queryMemoryListFields(fields, 20)) {        b.totalPages = fields[0];        b.activePages = fields[9];        b.standbyPages = fields[10];        b.modifiedPages = fields[6];        b.modifiedNoWritePages = fields[7];        b.zeroPages = fields[4];        b.freePages = fields[5];        b.badPages = fields[8];
        uint64_t accounted = b.activePages + b.standbyPages + b.modifiedPages            + b.modifiedNoWritePages + b.zeroPages + b.freePages + b.badPages;
        if(b.totalPages > accounted) {            b.transitionPages = b.totalPages - accounted;        }    }
    return b; }
uint32_t getProcessHandleCount(uint32_t pid) {    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if(!hProcess) return 0;
    DWORD count = 0;
    if(!GetProcessHandleCount(hProcess, &count)) count = 0;    CloseHandle(hProcess);
    return count; }
ProcessIoStats getProcessIoStats(uint32_t pid) {    ProcessIoStats stats;    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if(!hProcess) return stats;    IO_COUNTERS io{};
    if(GetProcessIoCounters(hProcess, &io)) {        stats.readOps = io.ReadOperationCount;        stats.writeOps = io.WriteOperationCount;        stats.otherOps = io.OtherOperationCount;        stats.readBytes = io.ReadTransferCount;        stats.writeBytes = io.WriteTransferCount;        stats.otherBytes = io.OtherTransferCount;    }    CloseHandle(hProcess);
    return stats; }
uint64_t getProcessCreationTime(uint32_t pid) {    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if(!hProcess) return 0;    FILETIME create{}, exit{}, kernel{}, user{};
    if(!GetProcessTimes(hProcess, &create, &exit, &kernel, &user)) { CloseHandle(hProcess); return 0; }    CloseHandle(hProcess);
    return (static_cast<uint64_t>(create.dwHighDateTime) << 32) | create.dwLowDateTime; }
bool isFullScreenAppActive() {    HWND fg = GetForegroundWindow();
    if(!fg) return false;
    wchar_t cls[64];    if(!GetClassNameW(fg, cls, 64)) return false;
    if(wcscmp(cls, L"Progman") == 0 || wcscmp(cls, L"WorkerW") == 0) return false;
    RECT wRect;    if(!GetWindowRect(fg, &wRect)) return false;
    HMONITOR mon = MonitorFromWindow(fg, MONITOR_DEFAULTTONEAREST);
    if(!mon) return false;
    MONITORINFO mi;    mi.cbSize = sizeof(mi);
    if(!GetMonitorInfoW(mon, &mi)) return false;
    int w = wRect.right - wRect.left;
    int h = wRect.bottom - wRect.top;
    int mw = mi.rcMonitor.right - mi.rcMonitor.left;
    int mh = mi.rcMonitor.bottom - mi.rcMonitor.top;
    return (w >= mw && h >= mh); }
} // namespace RAMFlux::NtApi


