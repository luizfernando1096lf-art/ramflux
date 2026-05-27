// RAMFlux - Process Snapshot Implementation
// Captura instantânea do estado de processos do sistema

#include "ProcessSnapshot.h"
#include <windows.h>
#include <string>
#include <QJsonArray>

ProcessSnapshot::ProcessSnapshot()
    : m_valid(false)
{
}

ProcessSnapshot ProcessSnapshot::captureSnapshot()
{
    ProcessSnapshot snapshot;
    snapshot.m_valid = true;
    snapshot.m_timestamp = QDateTime::currentDateTime();
    
    QVector<ProcessHandle> handles;
    EnumProcesses(handles.data(), 0, nullptr);
    
    for (ProcessHandle handle : handles) {
        ProcessInfo info;
        info.processId = handle;
        GetProcessInfo(info);
        snapshot.addProcess(info);
    }
    
    return snapshot;
}

void ProcessSnapshot::addProcess(const ProcessInfo &process)
{
    m_processes.append(process);
}

QVector<ProcessInfo> ProcessSnapshot::getProcesses() const
{
    return m_processes;
}

QDateTime ProcessSnapshot::getTimestamp() const
{
    return m_timestamp;
}

QJsonObject ProcessSnapshot::toJson() const
{
    QJsonObject json;
    
    json["timestamp"] = m_timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz");
    json["processCount"] = m_processes.size();
    json["totalWorkingSet"] = getTotalWorkingSet();
    json["totalPrivateUsage"] = getTotalPrivateUsage();
    json["totalPeakWorkingSet"] = getTotalPeakWorkingSet();
    
    QJsonArray processesArray;
    for (const auto &proc : m_processes) {
        QJsonObject procJson;
        procJson["processId"] = proc.processId;
        procJson["processName"] = proc.processName;
        procJson["workingSetSize"] = proc.workingSetSize;
        procJson["privateUsage"] = proc.privateUsage;
        procJson["peakWorkingSet"] = proc.peakWorkingSet;
        procJson["cpuTimeUser"] = proc.cpuTimeUser;
        procJson["cpuTimeKernel"] = proc.cpuTimeKernel;
        procJson["priorityBase"] = proc.priorityBase;
        procJson["processName"] = proc.processName;
        processesArray.append(procJson);
    }
    
    json["processes"] = processesArray;
    
    return json;
}

ProcessSnapshot ProcessSnapshot::fromJson(const QJsonObject &json)
{
    ProcessSnapshot snapshot;
    snapshot.m_valid = json.contains("processes");
    
    if (json.contains("timestamp")) {
        snapshot.m_timestamp = QDateTime::fromString(json["timestamp"].toString(), "yyyy-MM-dd HH:mm:ss.zzz");
    }
    
    if (json.contains("processes") && json["processes"].isArray()) {
        const QJsonArray &processesArray = json["processes"].toArray();
        for (const auto &procJson : processesArray) {
            const QJsonObject &procObj = procJson.toObject();
            ProcessInfo info;
            info.processId = procObj["processId"].toUInt();
            info.processName = procObj["processName"].toString();
            info.workingSetSize = procObj["workingSetSize"].toUInt64();
            info.privateUsage = procObj["privateUsage"].toUInt64();
            info.peakWorkingSet = procObj["peakWorkingSet"].toUInt64();
            info.cpuTimeUser = procObj["cpuTimeUser"].toUInt();
            info.cpuTimeKernel = procObj["cpuTimeKernel"].toUInt();
            info.priorityBase = procObj["priorityBase"].toUInt();
            
            snapshot.addProcess(info);
        }
    }
    
    return snapshot;
}

bool ProcessSnapshot::isValid() const
{
    return m_valid;
}

int ProcessSnapshot::processCount() const
{
    return m_processes.size();
}

quint64 ProcessSnapshot::getTotalWorkingSet() const
{
    quint64 total = 0;
    for (const auto &proc : m_processes) {
        total += proc.workingSetSize;
    }
    return total;
}

quint64 ProcessSnapshot::getTotalPrivateUsage() const
{
    quint64 total = 0;
    for (const auto &proc : m_processes) {
        total += proc.privateUsage;
    }
    return total;
}

quint64 ProcessSnapshot::getTotalPeakWorkingSet() const
{
    quint64 total = 0;
    for (const auto &proc : m_processes) {
        total += proc.peakWorkingSet;
    }
    return total;
}

// Comparação de snapshots
bool ProcessSnapshotComparison::compareSnapshots(const ProcessSnapshot &snapshot1,
                                                   const ProcessSnapshot &snapshot2,
                                                   bool ascending)
{
    return true;  // Implementação básica
}

bool ProcessSnapshotComparison::hasProcessChanges(const ProcessSnapshot &snapshot1,
                                                    const ProcessSnapshot &snapshot2)
{
    return true;  // Implementação básica
}

QVector<ProcessInfo> ProcessSnapshotComparison::getNewProcesses(const ProcessSnapshot &snapshot1,
                                                                  const ProcessSnapshot &snapshot2)
{
    return QVector<ProcessInfo>();  // Implementação básica
}

QVector<ProcessInfo> ProcessSnapshotComparison::getRemovedProcesses(const ProcessSnapshot &snapshot1,
                                                                     const ProcessSnapshot &snapshot2)
{
    return QVector<ProcessInfo>();  // Implementação básica
}

QVector<ProcessInfo> ProcessSnapshotComparison::getChangedProcesses(const ProcessSnapshot &snapshot1,
                                                                     const ProcessSnapshot &snapshot2)
{
    return QVector<ProcessInfo>();  // Implementação básica
}

QVector<ProcessInfo> ProcessSnapshotComparison::getMemoryChangedProcesses(const ProcessSnapshot &snapshot1,
                                                                            const ProcessSnapshot &snapshot2)
{
    return QVector<ProcessInfo>();  // Implementação básica
}