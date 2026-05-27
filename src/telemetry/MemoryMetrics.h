// RAMFlux - Memory Metrics Header
// Métricas detalhadas de uso de memória

#ifndef MEMORYMETRICS_H
#define MEMORYMETRICS_H

#include <QObject>
#include <QVector>
#include <QVariantMap>

class MemoryMetrics : public QObject
{
    Q_OBJECT

public:
    explicit MemoryMetrics(QObject *parent = nullptr);
    ~MemoryMetrics() = default;

    // Métricas básicas
    quint64 getTotalMemory() const { return totalMemory_; }
    quint64 getUsedMemory() const { return usedMemory_; }
    quint64 getFreeMemory() const { return freeMemory_; }
    quint64 getAvailableMemory() const { return availableMemory_; }
    quint64 getCommittedMemory() const { return committedMemory_; }
    quint64 getPeakVirtualMemory() const { return peakVirtualMemory_; }
    quint64 getWorkingSet() const { return workingSet_; }
    quint64 getPageFaults() const { return pageFaults_; }
    quint64 getCacheHitRate() const { return cacheHitRate_; }

    // Métricas de processos
    int getProcessCount() const { return processCount_; }
    QList<ProcessInfo> getTopMemoryProcesses(int limit = 10) const;
    ProcessInfo getProcessById(quint64 processId) const;

    // Histórico
    QVector<QVariantMap> getMemoryHistory(int limit = 100) const;
    QVector<QVariantMap> getProcessHistory(int limit = 100) const;

    // Reset
    void reset();

signals:
    void memoryChanged();
    void processChanged();

private:
    quint64 totalMemory_ = 0;
    quint64 usedMemory_ = 0;
    quint64 freeMemory_ = 0;
    quint64 availableMemory_ = 0;
    quint64 committedMemory_ = 0;
    quint64 peakVirtualMemory_ = 0;
    quint64 workingSet_ = 0;
    quint64 pageFaults_ = 0;
    quint64 cacheHitRate_ = 100;

    int processCount_ = 0;
    QVector<ProcessInfo> processes_;
    QVector<QVariantMap> memoryHistory_;
    QVector<QVariantMap> processHistory_;

    struct ProcessInfo
    {
        quint64 id;
        QString name;
        quint64 memoryMB;
        quint64 workingSetMB;
        quint64 privateMemoryMB;
        quint64 virtualMemoryMB;
        quint32 threadCount;
        bool isApp;
        QString commandLine;
    };
};

#endif // MEMORYMETRICS_H