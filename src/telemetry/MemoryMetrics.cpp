// RAMFlux - Memory Metrics Implementation
// Implementação de métricas de memória

#include "MemoryMetrics.h"

// Construtor
MemoryMetrics::MemoryMetrics(QObject *parent)
    : QObject(parent)
{
    // Inicializar histórico
}

// Resetar todas as métricas
void MemoryMetrics::reset()
{
    totalMemory_ = 0;
    usedMemory_ = 0;
    freeMemory_ = 0;
    availableMemory_ = 0;
    committedMemory_ = 0;
    peakVirtualMemory_ = 0;
    workingSet_ = 0;
    pageFaults_ = 0;
    cacheHitRate_ = 100;
    processCount_ = 0;
    processes_.clear();
    memoryHistory_.clear();
    processHistory_.clear();
    
    emit memoryChanged();
    emit processChanged();
}

// Obter top processos por memória
QList<ProcessInfo> MemoryMetrics::getTopMemoryProcesses(int limit)
{
    // Ordenar por memória usada (embaixo)
    processes_.sort();
    
    // Limite
    if (static_cast<int>(processes_.count()) > limit) {
        processes_.truncate(limit);
    }
    
    // Criar resultado
    QList<ProcessInfo> result;
    for (const auto &proc : processes_) {
        result.append(proc);
    }
    
    return result;
}

// Obter processo por ID
ProcessInfo MemoryMetrics::getProcessById(quint64 processId)
{
    for (const auto &proc : processes_) {
        if (proc.id == processId) {
            return proc;
        }
    }
    
    return ProcessInfo{};
}