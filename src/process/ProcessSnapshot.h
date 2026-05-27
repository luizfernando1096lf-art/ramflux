// RAMFlux - Process Snapshot Header
// Captura instantânea do estado de processos do sistema

#ifndef PROCESSSNAPSHOT_H
#define PROCESSSNAPSHOT_H

#include "ProcessMonitor.h"
#include <QVector>
#include <QDateTime>
#include <QJsonObject>

class ProcessSnapshot
{
public:
    ProcessSnapshot();
    ~ProcessSnapshot() = default;

    // Criar snapshot dos processos atuais
    static ProcessSnapshot captureSnapshot();

    // Adicionar processo individual ao snapshot
    void addProcess(const ProcessInfo &process);

    // Obter lista de processos do snapshot
    QVector<ProcessInfo> getProcesses() const;

    // Obter timestamp do snapshot
    QDateTime getTimestamp() const;

    // Converter para JSON
    QJsonObject toJson() const;

    // Converter de JSON
    static ProcessSnapshot fromJson(const QJsonObject &json);

    // Verificar se snapshot está válido
    bool isValid() const;

    // Obter número de processos
    int processCount() const;

    // Calcular totais do snapshot
    quint64 getTotalWorkingSet() const;
    quint64 getTotalPrivateUsage() const;
    quint64 getTotalPeakWorkingSet() const;

private:
    QVector<ProcessInfo> m_processes;
    QDateTime m_timestamp;
    bool m_valid;
};

// Classe utilitária para comparar snapshots
class ProcessSnapshotComparison
{
public:
    static bool compareSnapshots(const ProcessSnapshot &snapshot1,
                                 const ProcessSnapshot &snapshot2,
                                 bool ascending = false);

    static bool hasProcessChanges(const ProcessSnapshot &snapshot1,
                                  const ProcessSnapshot &snapshot2);

    static QVector<ProcessInfo> getNewProcesses(const ProcessSnapshot &snapshot1,
                                                 const ProcessSnapshot &snapshot2);

    static QVector<ProcessInfo> getRemovedProcesses(const ProcessSnapshot &snapshot1,
                                                     const ProcessSnapshot &snapshot2);

    static QVector<ProcessInfo> getChangedProcesses(const ProcessSnapshot &snapshot1,
                                                      const ProcessSnapshot &snapshot2);

    static QVector<ProcessInfo> getMemoryChangedProcesses(const ProcessSnapshot &snapshot1,
                                                           const ProcessSnapshot &snapshot2);
};

#endif // PROCESSSNAPSHOT_H