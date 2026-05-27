// RAMFlux - Process Monitor Header
// Monitoramento de processos do sistema Windows

#ifndef PROCESSMONITOR_H
#define PROCESSMONITOR_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QDateTime>
#include <windows.h>

struct ProcessInfo {
    DWORD processId = 0;
    QString processName;
    QString executablePath;
    quint64 workingSetSize = 0;          // Memória em uso (working set)
    quint64 privateUsage = 0;             // Memória privada
    quint64 peakWorkingSet = 0;          // Working set máximo
    quint64 pageFaultCount = 0;
    quint64 handleCount = 0;
    quint64 threadCount = 0;
    quint64 cpuTimeUser = 0;              // Tempo de CPU usuário (ms)
    quint64 cpuTimeKernel = 0;            // Tempo de CPU kernel (ms)
    bool isSystemProcess = false;
    quint64 startTime = 0;                // Tempo de início (segundos)
    QString commandLine;                  // Linha de comando
    DWORD exitCode = STILL_ACTIVE;
};

class ProcessMonitor : public QObject
{
    Q_OBJECT

public:
    explicit ProcessMonitor(QObject *parent = nullptr);
    ~ProcessMonitor() = default;

    // Obter lista de todos os processos
    QVector<ProcessInfo> getAllProcesses() const;

    // Obter processo específico por ID
    ProcessInfo getProcessByPid(DWORD pid) const;

    // Obter processo específico por nome
    QVector<ProcessInfo> getProcessesByName(const QString &name) const;

    // Obter processos que estão consumindo muita memória (threshold)
    QVector<ProcessInfo> getHighMemoryProcesses(quint64 thresholdBytes = 100 * 1024 * 1024) const;

    // Obter processos que consumem muita CPU
    QVector<ProcessInfo> getHighCpuProcesses() const;

    // Obter número total de processos
    int getTotalProcessCount() const { return processInfos_.size(); }

    // Obter total de memória em uso por processos
    quint64 getTotalProcessMemory() const;

    // Obter snapshot dos processos
    void captureSnapshot();

    // Obter lista de nomes de processos
    QVector<QString> getProcessNames() const;

    // Verificar se um processo está em execução
    bool isProcessRunning(DWORD pid) const;

    // Obter processos por grupo/parent-child relationships
    QVector<ProcessInfo> getChildrenOfProcess(DWORD parentPid) const;
    QVector<ProcessInfo> getParentsOfProcess(DWORD childPid) const;

signals:
    void processListChanged();
    void highMemoryProcessDetected(const ProcessInfo &process);
    void processStarted(const ProcessInfo &process);
    void processStopped(const ProcessInfo &process);

private:
    QVector<ProcessInfo> processInfos_;
    QTimer *processTimer_;
    quint64 lastMemoryCheck_ = 0;
    const int MEMORY_CHECK_INTERVAL_MS = 1000;  // 1s para atualizar memória
};

#endif // PROCESSMONITOR_H