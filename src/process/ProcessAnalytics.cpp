// RAMFlux - Process Analytics Implementation
// Análise avançada do working set e métricas de processos

#include "ProcessAnalytics.h"
#include <algorithm>

ProcessAnalytics::ProcessAnalytics(QObject *parent)
    : QObject(parent)
{
}

WorkingSetAnalysis ProcessAnalytics::analyzeWorkingSet(const QVector<ProcessInfo> &processes,
                                                        const SYSTEM_INFO &sysInfo) const
{
    WorkingSetAnalysis analysis;
    
    analysis.totalWorkingSet = 0;
    analysis.totalPrivateWorkingSet = 0;
    analysis.sharedWorkingSet = 0;
    analysis.processMetrics.clear();
    
    analysis.analysisTimestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    
    double totalMemory = static_cast<double>(sysInfo.dwTotalPageFile) + sysInfo.dwTotalPhys;
    
    for (const auto &proc : processes) {
        ProcessMemoryMetrics metrics;
        metrics.workingSetSize = proc.workingSetSize;
        metrics.privateUsage = proc.privateUsage;
        metrics.peakWorkingSet = proc.peakWorkingSet;
        metrics.pageFileUsage = proc.pageFileUsage;
        metrics.memoryPercentage = (proc.workingSetSize * 100.0) / sysInfo.dwTotalPhys;
        
        analysis.totalWorkingSet += proc.workingSetSize;
        analysis.totalPrivateWorkingSet += proc.privateUsage;
        
        analysis.processMetrics.append(metrics);
    }
    
    if (analysis.sharedWorkingSet > 0) {
        analysis.privateToSharedRatio = analysis.totalPrivateWorkingSet / analysis.sharedWorkingSet;
    } else {
        analysis.privateToSharedRatio = 1.0;
    }
    
    analysis.workingSetEfficiency = analysis.sharedWorkingSet * 100.0 / analysis.totalWorkingSet;
    
    return analysis;
}

double ProcessAnalytics::calculateMemorySharingRatio(const QVector<ProcessInfo> &processes) const
{
    quint64 totalPrivate = 0;
    quint64 totalShared = 0;
    
    for (const auto &proc : processes) {
        totalPrivate += proc.privateUsage;
        // Memória compartilhada é aproximada subtraindo private do working set total
    }
    
    if (totalShared > 0) {
        return static_cast<double>(totalPrivate) / totalShared;
    }
    return 1.0;
}

QVector<ProcessInfo> ProcessAnalytics::detectMemoryLeaks(const QVector<ProcessInfo> &processes,
                                                           quint64 thresholdPercentage) const
{
    QVector<ProcessInfo> leaks;
    
    for (const auto &proc : processes) {
        if (proc.memoryPercentage > thresholdPercentage) {
            leaks.append(proc);
        }
    }
    
    return leaks;
}

QVector<ProcessPerformanceMetrics> ProcessAnalytics::calculateCpuMetrics(const QVector<ProcessInfo> &processes,
                                                                           const SYSTEM_INFO &sysInfo) const
{
    QVector<ProcessPerformanceMetrics> metrics;
    DWORD totalCores = sysInfo.dwNumberOfProcessors;
    double cpuUsageThreshold = 50.0;  // 50% como limiar de alertas
    
    for (const auto &proc : processes) {
        ProcessPerformanceMetrics perf;
        perf.processId = proc.processId;
        perf.processName = proc.processName;
        
        // CPU usage é aproximado baseado no tempo de processamento
        perf.userModeTime = proc.userModeTime;
        perf.kernelModeTime = proc.kernelModeTime;
        perf.totalProcessorTime = proc.userModeTime + proc.kernelModeTime;
        
        perf.cpuUsagePercentage = proc.cpuTimeUser * 100.0 / totalCores;
        
        metrics.append(perf);
    }
    
    return metrics;
}

QVector<ProcessInfo> ProcessAnalytics::findIdleProcesses(const QVector<ProcessInfo> &processes) const
{
    QVector<ProcessInfo> idle;
    
    // Processo é considerado ocioso se tiver menos de 5% de CPU por 1 minuto
    for (const auto &proc : processes) {
        if (proc.cpuUsagePercentage < 5.0) {
            idle.append(proc);
        }
    }
    
    return idle;
}

QVector<ProcessInfo> ProcessAnalytics::findResourceHog(const QVector<ProcessInfo> &processes,
                                                         const QString &resourceType) const
{
    QVector<ProcessInfo> hogs;
    
    for (const auto &proc : processes) {
        if (resourceType == "memory" && proc.workingSetSize > 100 * 1024 * 1024) {
            hogs.append(proc);
        } else if (resourceType == "cpu" && proc.cpuUsagePercentage > 50.0) {
            hogs.append(proc);
        }
    }
    
    return hogs;
}

QMap<QString, double> ProcessAnalytics::calculateMemoryEfficiency(const QVector<ProcessInfo> &processes) const
{
    QMap<QString, double> efficiency;
    
    for (const auto &proc : processes) {
        QString key = proc.processName;
        double currentMemory = static_cast<double>(proc.workingSetSize);
        double peakMemory = static_cast<double>(proc.peakWorkingSet);
        
        if (peakMemory > 0) {
            efficiency[key] = (currentMemory / peakMemory) * 100.0;
        }
    }
    
    return efficiency;
}

QVector<ProcessInfo> ProcessAnalytics::sortProcessesByMemory(const QVector<ProcessInfo> &processes) const
{
    QVector<ProcessInfo> sorted = processes;
    std::sort(sorted.begin(), sorted.end(), [](const ProcessInfo &a, const ProcessInfo &b) {
        return a.workingSetSize > b.workingSetSize;
    });
    
    return sorted;
}

QVector<ProcessInfo> ProcessAnalytics::sortProcessesByCpu(const QVector<ProcessInfo> &processes) const
{
    QVector<ProcessInfo> sorted = processes;
    std::sort(sorted.begin(), sorted.end(), [](const ProcessInfo &a, const ProcessInfo &b) {
        return a.cpuTimeUser + a.cpuTimeKernel > b.cpuTimeUser + b.cpuTimeKernel;
    });
    
    return sorted;
}

QMap<QString, quint64> ProcessAnalytics::getMemoryStatistics(const QVector<ProcessInfo> &processes) const
{
    QMap<QString, quint64> stats;
    
    quint64 totalMemory = 0;
    quint64 totalPrivate = 0;
    quint64 totalPeak = 0;
    quint64 totalPages = 0;
    
    for (const auto &proc : processes) {
        totalMemory += proc.workingSetSize;
        totalPrivate += proc.privateUsage;
        totalPeak += proc.peakWorkingSet;
        totalPages += proc.pageFileUsage;
    }
    
    stats["totalWorkingSet"] = totalMemory;
    stats["totalPrivateUsage"] = totalPrivate;
    stats["totalPeakWorkingSet"] = totalPeak;
    stats["totalPageFileUsage"] = totalPages;
    
    return stats;
}

QMap<QString, QVector<ProcessInfo>> ProcessAnalytics::getMemoryDistribution(const QVector<ProcessInfo> &processes) const
{
    QMap<QString, QVector<ProcessInfo>> distribution;
    
    for (const auto &proc : processes) {
        if (proc.workingSetSize < 10 * 1024 * 1024) {
            distribution["<10MB"].append(proc);
        } else if (proc.workingSetSize < 50 * 1024 * 1024) {
            distribution["10-50MB"].append(proc);
        } else if (proc.workingSetSize < 100 * 1024 * 1024) {
            distribution["50-100MB"].append(proc);
        } else if (proc.workingSetSize < 500 * 1024 * 1024) {
            distribution["100-500MB"].append(proc);
        } else if (proc.workingSetSize < 1000 * 1024 * 1024) {
            distribution["500MB-1GB"].append(proc);
        } else {
            distribution[">1GB"].append(proc);
        }
    }
    
    return distribution;
}

QVector<ProcessInfo> ProcessAnalytics::identifySuspiciousProcesses(const QVector<ProcessInfo> &processes) const
{
    QVector<ProcessInfo> suspicious;
    
    for (const auto &proc : processes) {
        // Processos suspeitos: alto uso de memória + muitos handles
        if (proc.workingSetSize > 50 * 1024 * 1024 && proc.handleCount > 1000) {
            suspicious.append(proc);
        }
        // Processos com crescimento rápido de memória
        if (proc.memoryPercentage > 30.0) {
            suspicious.append(proc);
        }
    }
    
    return suspicious;
}

QString ProcessAnalytics::generateReport(const QVector<ProcessInfo> &processes) const
{
    QString report = "=== Relatório de Análise de Processos ===\n\n";
    
    report += QString("Total de Processos: %1\n").arg(processes.size());
    report += QString("Memória Total em Uso: %1 MB\n")
              .arg(processes.summarize([](const ProcessInfo &p) -> quint64 { return p.workingSetSize; }) / (1024 * 1024));
    
    report += QString("Processos com Potencial de Vazamento: %1\n")
              .arg(detectMemoryLeaks(processes).size());
    
    report += "\n--- Processos Top 10 por Uso de Memória ---\n";
    
    QVector<ProcessInfo> sorted = sortProcessesByMemory(processes);
    for (int i = 0; i < qMin(10, sorted.size()); ++i) {
        const auto &proc = sorted[i];
        report += QString("[%1] %2 - %3 MB\n")
                  .arg(i + 1)
                  .arg(proc.processName)
                  .arg(proc.workingSetSize / (1024 * 1024));
    }
    
    report += "\n";
    
    return report;
}

// Métodos privados
bool ProcessAnalytics::checkMemoryLeak(const ProcessInfo &process, const ProcessInfo &previous) const
{
    if (previous.workingSetSize == 0) {
        return false;
    }
    
    double growth = static_cast<double>(process.workingSetSize - previous.workingSetSize) /
                   previous.workingSetSize * 100.0;
    
    return growth > 20.0;  // Mais de 20% de crescimento indica vazamento
}

double ProcessAnalytics::calculateMemoryGrowthRate(const ProcessInfo &current,
                                                    const ProcessInfo &previous) const
{
    if (previous.workingSetSize == 0 || previous.startTime == 0) {
        return 0.0;
    }
    
    double deltaTime = static_cast<double>(current.startTime - previous.startTime);
    if (deltaTime <= 0) {
        return 0.0;
    }
    
    return static_cast<double>(current.workingSetSize - previous.workingSetSize) / deltaTime;
}