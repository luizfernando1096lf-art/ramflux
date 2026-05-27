// RAMFlux - Process Monitor Implementation
// Monitoramento de processos do sistema Windows

#include "ProcessMonitor.h"
#include <psapi.h>
#include <tlhelp32.h>

ProcessMonitor::ProcessMonitor(QObject *parent)
    : QObject(parent),
      processTimer_(new QTimer(this))
{
    processTimer_->setInterval(MEMORY_CHECK_INTERVAL_MS);
    connect(processTimer_, &QTimer::timeout, this, &ProcessMonitor::captureSnapshot);
    captureSnapshot();
    processTimer_->start();
}

void ProcessMonitor::captureSnapshot()
{
    processInfos_.clear();
    
    PROCESSENTRY32 processEntry = {0};
    processEntry.dwSize = sizeof(PROCESSENTRY32);
    
    HANDLE hSnapshot = CreateToolhelpExeSnapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return;
    }
    
    if (Process32First(hSnapshot, &processEntry) == TRUE) {
        do {
            ProcessInfo info;
            info.processId = processEntry.th32ProcessID;
            info.processName = QString::fromUtf16(processEntry.szExeName, 32);
            info.isSystemProcess = (info.processName.startsWith(QLatin1String("system")) || 
                                   info.processName.startsWith(QLatin1String("smss")) ||
                                   info.processName.startsWith(QLatin1String("csrss")) ||
                                   info.processName.startsWith(QLatin1String("wininit")) ||
                                   info.processName.startsWith(QLatin1String("services")) ||
                                   info.processName.startsWith(QLatin1String("lsass")));
            
            // Verificar informações detalhadas do processo
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, info.processId);
            if (hProcess != NULL) {
                PROCESS_MEMORY_COUNTERS_EX pcm = {0};
                if (GetProcessMemoryInfo(hProcess, (PPROCESS_MEMORY_COUNTERS)&pcm, sizeof(pcm)) != 0) {
                    info.workingSetSize = pcm.WorkingSetSize;
                    info.privateUsage = pcm.PrivateUsage;
                    info.peakWorkingSet = pcm.PeakWorkingSetSize;
                    info.pageFaultCount = pcm.PagefileUsage;
                }
                if (pcm.HandleCount > 0) {
                    info.handleCount = pcm.HandleCount;
                }
                CloseHandle(hProcess);
            }
            
            // Obter informações adicionais usando QueryFullProcessImageName
            wchar_t imagePath[MAX_PATH] = {0};
            if (GetModuleFileNameExA(hProcess, NULL, imagePath, MAX_PATH) > 0) {
                info.executablePath = QString::fromLocal8Bit(imagePath);
            }
            
            // Obter linha de comando
            wchar_t commandLine[MAX_PATH * 2] = {0};
            if (GetCommandLineA(hProcess, commandLine, sizeof(commandLine) / sizeof(wchar_t)) > 0) {
                info.commandLine = QString::fromWCharArray(commandLine);
            }
            
            // Obter tempo de início
            HANDLE hStart = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, info.processId);
            if (hStart != NULL) {
                FILETIME ftCreation, ftExit, ftKernel, ftUser;
                if (GetProcessTimes(hStart, &ftCreation, &ftExit, &ftKernel, &ftUser)) {
                    ULARGE_INTEGER creationTime;
                    creationTime.LowPart = ftCreation.dwLowDateTime;
                    creationTime.HighPart = ftCreation.dwHighDateTime;
                    info.startTime = creationTime.QuadPart / 10000000;  // ticks -> segundos
                }
                CloseHandle(hStart);
            }
            
            // Verificar se é processo do sistema
            info.exitCode = GetExitCodeProcess(hProcess);
            CloseHandle(hProcess);
            
            processInfos_.append(info);
        } while (Process32Next(hSnapshot, &processEntry) == TRUE);
    }
    
    CloseHandle(hSnapshot);
    
    emit processListChanged();
    
    // Detectar processos de alta memória
    for (const auto &proc : processInfos_) {
        if (proc.workingSetSize > 100 * 1024 * 1024) {  // 100MB
            emit highMemoryProcessDetected(proc);
        }
    }
}

QVector<ProcessInfo> ProcessMonitor::getAllProcesses() const
{
    return processInfos_;
}

ProcessInfo ProcessMonitor::getProcessByPid(DWORD pid) const
{
    for (const auto &proc : processInfos_) {
        if (proc.processId == pid) {
            return proc;
        }
    }
    return {};
}

QVector<ProcessInfo> ProcessMonitor::getProcessesByName(const QString &name) const
{
    QVector<ProcessInfo> result;
    for (const auto &proc : processInfos_) {
        if (proc.processName.contains(name, Qt::CaseInsensitive)) {
            result.append(proc);
        }
    }
    return result;
}

QVector<ProcessInfo> ProcessMonitor::getHighMemoryProcesses(quint64 thresholdBytes) const
{
    QVector<ProcessInfo> result;
    for (const auto &proc : processInfos_) {
        if (proc.workingSetSize > thresholdBytes) {
            result.append(proc);
        }
    }
    return result;
}

QVector<ProcessInfo> ProcessMonitor::getHighCpuProcesses() const
{
    // Implementação futura - atualmente não disponível através do Windows API de forma direta
    // Seria necessário usar perf API ou instrumentation
    return {};
}

quint64 ProcessMonitor::getTotalProcessMemory() const
{
    quint64 total = 0;
    for (const auto &proc : processInfos_) {
        total += proc.workingSetSize;
    }
    return total;
}

void ProcessMonitor::captureSnapshot()
{
    // Captura snapshot atualizado dos processos
    QVector<ProcessInfo> currentProcesses;
    
    PROCESSENTRY32 processEntry = {0};
    processEntry.dwSize = sizeof(PROCESSENTRY32);
    
    HANDLE hSnapshot = CreateToolhelpExeSnapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        if (Process32First(hSnapshot, &processEntry) == TRUE) {
            do {
                ProcessInfo info;
                info.processId = processEntry.th32ProcessID;
                info.processName = QString::fromUtf16(processEntry.szExeName, 32);
                
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, info.processId);
                if (hProcess != NULL) {
                    PROCESS_MEMORY_COUNTERS_EX pcm = {0};
                    if (GetProcessMemoryInfo(hProcess, (PPROCESS_MEMORY_COUNTERS)&pcm, sizeof(pcm)) != 0) {
                        info.workingSetSize = pcm.WorkingSetSize;
                        info.privateUsage = pcm.PrivateUsage;
                        info.peakWorkingSet = pcm.PeakWorkingSetSize;
                        info.pageFaultCount = pcm.PagefileUsage;
                        info.handleCount = pcm.HandleCount;
                    }
                    if (GetModuleFileNameExA(hProcess, NULL, (LPTCH)processEntry.szExePath, MAX_PATH) > 0) {
                        info.executablePath = QString::fromLocal8Bit(processEntry.szExePath);
                    }
                    CloseHandle(hProcess);
                }
                
                currentProcesses.append(info);
            } while (Process32Next(hSnapshot, &processEntry) == TRUE);
        }
        CloseHandle(hSnapshot);
    }
    
    // Atualiza lista de processos
    for (const auto &proc : currentProcesses) {
        bool found = false;
        for (auto it = processInfos_.begin(); it != processInfos_.end(); ++it) {
            if (it->processId == proc.processId) {
                *it = proc;
                found = true;
                break;
            }
        }
        if (!found) {
            processInfos_.append(proc);
        }
    }
    
    // Remove processos que pararam
    QVector<DWORD> removedPids;
    for (int i = processInfos_.size() - 1; i >= 0; --i) {
        bool found = false;
        for (const auto &current : currentProcesses) {
            if (current.processId == processInfos_[i].processId) {
                found = true;
                break;
            }
        }
        if (!found) {
            removedPids.append(processInfos_[i].processId);
        }
    }
    
    // Emite sinal para processos parados
    for (const auto &pid : removedPids) {
        for (const auto &proc : processInfos_) {
            if (proc.processId == pid) {
                emit processStopped(proc);
                break;
            }
        }
    }
    
    processInfos_ = currentProcesses;
    emit processListChanged();
    
    // Detectar novos processos de alta memória
    for (const auto &proc : processInfos_) {
        if (proc.workingSetSize > 100 * 1024 * 1024) {
            emit highMemoryProcessDetected(proc);
        }
    }
}

QVector<QString> ProcessMonitor::getProcessNames() const
{
    QVector<QString> names;
    for (const auto &proc : processInfos_) {
        names.append(proc.processName);
    }
    return names;
}

bool ProcessMonitor::isProcessRunning(DWORD pid) const
{
    for (const auto &proc : processInfos_) {
        if (proc.processId == pid) {
            return proc.exitCode == STILL_ACTIVE;
        }
    }
    return false;
}

QVector<ProcessInfo> ProcessMonitor::getChildrenOfProcess(DWORD parentPid) const
{
    QVector<ProcessInfo> children;
    for (const auto &proc : processInfos_) {
        // Verificar se é filho do processo pai (usando handles ou outros métodos)
        // Esta implementação é simplificada
        children.append(proc);
    }
    return children;
}

QVector<ProcessInfo> ProcessMonitor::getParentsOfProcess(DWORD childPid) const
{
    QVector<ProcessInfo> parents;
    // Implementação futura - seria necessário usar QueryInformationProcess com ProcessIdToSessionId
    return parents;
}

// Função auxiliar para obter handle de processo
HANDLE ProcessMonitor::getProcessHandle(DWORD pid)
{
    return OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
}

// Função auxiliar para fechar handle de processo
void ProcessMonitor::closeProcessHandle(HANDLE hProcess)
{
    if (hProcess != NULL) {
        CloseHandle(hProcess);
    }
}