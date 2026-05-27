// RAMFlux - Process Analytics Header
// Análise avançada do working set e métricas de processos

#ifndef PROCESSANALYTICS_H
#define PROCESSANALYTICS_H

#include "ProcessMonitor.h"
#include <QObject>
#include <QVector>
#include <QMap>

struct ProcessMemoryMetrics {
    quint64 workingSetSize = 0;
    quint64 privateUsage = 0;
    quint64 peakWorkingSet = 0;
    quint64 pageFileUsage = 0;
    double memoryPercentage = 0.0;
    double memoryGrowthRate = 0.0;  // bytes/segundo
    quint64 averageMemory = 0;
    quint64 currentMemory = 0;
    quint64 peakMemory = 0;
};

struct ProcessPerformanceMetrics {
    DWORD processId = 0;
    QString processName;
    quint64 userModeTime = 0;
    quint64 kernelModeTime = 0;
    quint64 totalProcessorTime = 0;
    double cpuUsagePercentage = 0.0;
    double cpuGrowthRate = 0.0;
    quint64 contextSwitches = 0;
    quint64 pageFaults = 0;
    quint64 voluntaryContextSwitches = 0;
    quint64 involuntaryContextSwitches = 0;
};

struct WorkingSetAnalysis {
    quint64 totalWorkingSet = 0;
    quint64 totalPrivateWorkingSet = 0;
    quint64 sharedWorkingSet = 0;
    double privateToSharedRatio = 0.0;
    double workingSetEfficiency = 0.0;
    QVector<ProcessMemoryMetrics> processMetrics;
    QVector<ProcessPerformanceMetrics> performanceMetrics;
    QString analysisTimestamp;
};

class ProcessAnalytics : public QObject
{
    Q_OBJECT

public:
    explicit ProcessAnalytics(QObject *parent = nullptr);
    ~ProcessAnalytics() = default;

    // Analisar working set de todos os processos
    WorkingSetAnalysis analyzeWorkingSet(const QVector<ProcessInfo> &processes,
                                         const SYSTEM_INFO &sysInfo) const;

    // Calcular memória compartilhada vs privada
    double calculateMemorySharingRatio(const QVector<ProcessInfo> &processes) const;

    // Identificar processos com vazamento de memória
    QVector<ProcessInfo> detectMemoryLeaks(const QVector<ProcessInfo> &processes,
                                            quint64 thresholdPercentage = 20.0) const;

    // Calcular métricas de uso de CPU
    QVector<ProcessPerformanceMetrics> calculateCpuMetrics(const QVector<ProcessInfo> &processes,
                                                            const SYSTEM_INFO &sysInfo) const;

    // Identificar processos ociosos
    QVector<ProcessInfo> findIdleProcesses(const QVector<ProcessInfo> &processes) const;

    // Identificar processos de uso excessivo
    QVector<ProcessInfo> findResourceHog(const QVector<ProcessInfo> &processes,
                                          const QString &resourceType = "memory") const;

    // Calcular eficiência de memória por tipo de processo
    QMap<QString, double> calculateMemoryEfficiency(const QVector<ProcessInfo> &processes) const;

    // Ordenar processos por uso de memória
    QVector<ProcessInfo> sortProcessesByMemory(const QVector<ProcessInfo> &processes) const;

    // Ordenar processos por uso de CPU
    QVector<ProcessInfo> sortProcessesByCpu(const QVector<ProcessInfo> &processes) const;

    // Obter estatísticas agregadas de memória
    QMap<QString, quint64> getMemoryStatistics(const QVector<ProcessInfo> &processes) const;

    // Obter distribuições de memória por faixa
    QMap<QString, QVector<ProcessInfo>> getMemoryDistribution(const QVector<ProcessInfo> &processes) const;

    // Identificar processos suspeitos
    QVector<ProcessInfo> identifySuspiciousProcesses(const QVector<ProcessInfo> &processes) const;

    // Gerar relatório de análise de processos
    QString generateReport(const QVector<ProcessInfo> &processes) const;

private:
    // Verificar vazamento de memória em um processo específico
    bool checkMemoryLeak(const ProcessInfo &process, const ProcessInfo &previous) const;
    
    // Calcular crescimento de memória
    double calculateMemoryGrowthRate(const ProcessInfo &current,
                                     const ProcessInfo &previous) const;
};

#endif // PROCESSANALYTICS_H