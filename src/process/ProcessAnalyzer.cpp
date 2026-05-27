/**
 * @file ProcessAnalyzer.cpp
 * @brief Implementação de ProcessAnalyzer para análise de processos
 */

#include "ProcessAnalyzer.h"
#include <algorithm>

namespace RAMFlux
{
namespace Process
{

// ============================================================================
// ProcessEnumerator Implementação
// ============================================================================

std::vector<HANDLE> ProcessEnumerator::enumProcesses() const
{
    std::vector<HANDLE> handles;
    PROCESSENTRY32 pe32{};
    pe32.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return handles;
    }

    if (Process32First(hSnapshot, &pe32))
    {
        do
        {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe32.th32ProcessID);
            if (hProcess != nullptr)
            {
                handles.push_back(hProcess);
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return handles;
}

std::vector<ProcessInfo> ProcessEnumerator::enumProcessesWithInfo() const
{
    std::vector<ProcessInfo> infos;
    PROCESSENTRY32 pe32{};
    pe32.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return infos;
    }

    if (Process32First(hSnapshot, &pe32))
    {
        do
        {
            ProcessInfo info;
            info.hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe32.th32ProcessID);
            info.processId = pe32.th32ProcessID;
            info.processName = pe32.szExeFile;
            info.workingSetSize = 0;
            info.privateSize = 0;
            info.exitCode = 0;
            info.exists = true;

            if (info.hProcess != nullptr)
            {
                getProcessInfo(info.hProcess, info.processId, info);
                infos.push_back(info);
            }
            else
            {
                info.exists = false;
                infos.push_back(info);
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return infos;
}

std::vector<ProcessInfo> ProcessEnumerator::filterByName(const std::wstring& name) const
{
    std::vector<ProcessInfo> infos;
    auto allProcesses = enumProcessesWithInfo();

    for (auto& info : allProcesses)
    {
        if (info.exists && info.processName.find(name) != std::wstring::npos)
        {
            infos.push_back(info);
        }
    }

    return infos;
}

bool ProcessEnumerator::processExists(DWORD processId) const
{
    PROCESSENTRY32 pe32{};
    pe32.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    bool exists = false;

    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        if (Process32First(hSnapshot, &pe32))
        {
            do
            {
                if (pe32.th32ProcessID == processId)
                {
                    exists = true;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }

        CloseHandle(hSnapshot);
    }

    return exists;
}

void ProcessEnumerator::getProcessInfo(HANDLE hProcess, DWORD pid, ProcessInfo& info) const
{
    HANDLE hProcessHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcessHandle != nullptr)
    {
        info.hProcess = hProcessHandle;
        info.workingSetSize = 0;
        info.privateSize = 0;

        SIZE_T workingSet = 0;
        SIZE_T privateSize = 0;

        if (GetProcessWorkingSet(hProcessHandle, workingSet) &&
            getProcessPrivateSize(hProcessHandle, privateSize))
        {
            info.workingSetSize = workingSet;
            info.privateSize = privateSize;
        }

        CloseHandle(hProcessHandle);
    }
    else
    {
        info.hProcess = nullptr;
        info.exists = false;
    }
}

SIZE_T ProcessEnumerator::getProcessWorkingSet(HANDLE hProcess, SIZE_T& workingSet) const
{
    HANDLE hProcessHandle = hProcess;
    MEMORY_BASIC_INFORMATION mbi{};

    DWORD pageSize = static_cast<DWORD>(GetSystemInfo().dwPageSize);
    DWORD pageSizeKB = pageSize / 1024;

    SIZE_T totalWorkingSet = 0;

    // Para otimização, usaremos GetProcessMemoryInformation que é mais eficiente
    PROCESS_MEMORY_COUNTERS_EX pmc{};
    bool success = GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc));

    if (success)
    {
        workingSet = pmc.WorkingSetSize;
    }

    return workingSet;
}

SIZE_T ProcessEnumerator::getProcessPrivateSize(HANDLE hProcess, SIZE_T& privateSize) const
{
    // A função GetProcessMemoryInfo não fornece PrivateSize
    // Precisamos usar VirtualQuery para contar memória privada
    privateSize = 0;

    HANDLE hProcessHandle = hProcess;
    void* baseAddr = nullptr;
    SIZE_T bytesNeeded;
    BOOL result = VirtualQueryEx(hProcessHandle, baseAddr, &bytesNeeded);
    privateSize = 0;

    return privateSize;
}

// ============================================================================
// ProcessAnalyzer Implementação
// ============================================================================

std::map<DWORD, ProcessInfo> ProcessAnalyzer::analyzeAllProcesses() const
{
    std::map<DWORD, ProcessInfo> allProcesses;
    auto enumerator = ProcessEnumerator{};
    auto infos = enumerator.enumProcessesWithInfo();

    for (auto& info : infos)
    {
        allProcesses[info.processId] = info;
    }

    return allProcesses;
}

std::map<DWORD, ProcessInfo> ProcessAnalyzer::analyzeProcesses(const std::vector<DWORD>& pids) const
{
    std::map<DWORD, ProcessInfo> results;

    for (DWORD pid : pids)
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
        if (hProcess != nullptr)
        {
            ProcessInfo info;
            info.hProcess = hProcess;
            info.processId = pid;
            info.exitCode = 0;
            info.exists = true;

            // Obter nome do processo
            std::wstring name;
            DWORD bufferSize = GetModuleFileNameExW(hProcess, nullptr, nullptr, 0);
            if (bufferSize > 0)
            {
                std::vector<wchar_t> buffer(bufferSize + 1);
                DWORD bytesRead = GetModuleFileNameExW(hProcess, nullptr, buffer.data(), bufferSize);
                if (bytesRead > 0)
                {
                    name = std::wstring(buffer.data(), bytesRead);
                }
            }
            info.processName = name;

            // Obter informações de memória
            PROCESS_MEMORY_COUNTERS_EX pmc{};
            pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS_EX);
            if (GetProcessMemoryInfo(hProcess, reinterpret_cast<PMEMORY_COUNTERS_EX>(&pmc), sizeof(pmc)))
            {
                info.workingSetSize = pmc.WorkingSetSize;
                info.privateSize = pmc.PrivateUsage;
            }
            info.exitCode = 0;

            results[pid] = info;
        }
        else
        {
            ProcessInfo info;
            info.processId = pid;
            info.exists = false;
            results[pid] = info;
        }

        CloseHandle(hProcess);
    }

    return results;
}

ProcessInfo ProcessAnalyzer::findHighestMemoryProcess() const
{
    auto enumerator = ProcessEnumerator{};
    auto infos = enumerator.enumProcessesWithInfo();

    if (infos.empty())
    {
        ProcessInfo info{};
        info.exists = false;
        return info;
    }

    auto maxIt = std::max_element(infos.begin(), infos.end(),
        [](const ProcessInfo& a, const ProcessInfo& b)
    {
        return a.workingSetSize < b.workingSetSize;
    });

    return *maxIt;
}

std::vector<ProcessInfo> ProcessAnalyzer::findTopProcessesByMemory(int top) const
{
    auto enumerator = ProcessEnumerator{};
    auto infos = enumerator.enumProcessesWithInfo();

    if (infos.empty())
    {
        return {};
    }

    // Ordenar por workingSetSize em ordem decrescente
    std::vector<ProcessInfo> sorted = infos;
    std::sort(sorted.begin(), sorted.end(),
        [](const ProcessInfo& a, const ProcessInfo& b)
    {
        return a.workingSetSize > b.workingSetSize;
    });

    if (static_cast<int>(sorted.size()) > top)
    {
        sorted.resize(top);
    }

    return sorted;
}

ProcessAnalyzer::AggregateMetrics ProcessAnalyzer::getAggregateMetrics() const
{
    auto enumerator = ProcessEnumerator{};
    auto infos = enumerator.enumProcessesWithInfo();

    AggregateMetrics metrics{};
    metrics.processCount = 0;
    metrics.threadCount = 0;
    metrics.totalProcessMemory = 0;
    metrics.totalPrivateMemory = 0;

    for (const auto& info : infos)
    {
        if (info.exists)
        {
            metrics.totalProcessMemory += info.workingSetSize;
            metrics.totalPrivateMemory += info.privateSize;
            metrics.processCount++;
        }
    }

    // Contar threads (simplificado - apenas número de threads do sistema)
    SYSTEM_INFO sysInfo{};
    GetSystemInfo(&sysInfo);
    metrics.threadCount = sysInfo.dwNumberOfProcessors * 10; // Estimativa básica

    return metrics;
}

bool ProcessAnalyzer::isValidProcess(HANDLE hProcess) const
{
    return hProcess != nullptr && hProcess != INVALID_HANDLE_VALUE;
}

bool ProcessAnalyzer::getProcessMemoryInfo(HANDLE hProcess, PROCESS_MEMORY_COUNTERS_EX& memInfo) const
{
    if (!hProcess)
    {
        return false;
    }

    SIZE_T size = static_cast<SIZE_T>(GetProcessMemoryInfoSize(hProcess));
    if (size == 0 || size > sizeof(memInfo))
    {
        size = sizeof(PROCESS_MEMORY_COUNTERS_EX);
    }

    return GetProcessMemoryInfo(hProcess, reinterpret_cast<PMEMORY_COUNTERS_EX>(&memInfo), size);
}

// ============================================================================
// Funções de Auxílio
// ============================================================================

SIZE_T GetProcessMemoryInfoSize(HANDLE hProcess)
{
    return sizeof(PROCESS_MEMORY_COUNTERS_EX);
}

// ============================================================================
} // namespace Process
} // namespace RAMFlux